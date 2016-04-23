// main.cpp: initialisation & main loop

#include "engine.h"
#include "FileIO.h"

#include <mutex>
#include <string>
#include <sstream>
#include <SDL_events.h>
#include <SDL_image.h>

extern void setcursorpos(int x, int y, bool checkCursor);
extern float cursorx, cursory;

#ifdef __WINRT__
/* Windows includes for ApplicationView */
#include <agile.h>
using namespace Windows::UI::ViewManagement;
#endif

// VARS
bool g_lite = true; // do not render particles, stain and do not show progress
extern char *assets_folder;
FVAR(DPIScale, 0.0, 0.0, 10.0);
FVAR(DPIScaleUsed, 0.0, 0.0, 10.0);
VAR(for_production, 0, 1, 1);

#define NOTEXTURE_PATH "packages/textures/notexture.png"

VAR(winx, 0, SDL_WINDOWPOS_UNDEFINED, 60000);
VAR(winy, 0, SDL_WINDOWPOS_UNDEFINED, 60000);

VARP(doubleclicktime, 10, 200, 1000);

extern void cleargamma();
const char *appLogPath() { return "app.log"; }
const char *savedconfig() { return "config.cfg"; }
const char *restoreconfig() { return CONFIG_PATH "/restore.cfg"; }
const char *defaultconfig() { return CONFIG_PATH "/defaults.cfg"; }

bool _doubleClick = false;
int _clickMs = -100;
int sposx = -1, sposy = -1;

extern int nosound;

VAR(desktopw, 1, 0, 0);
VAR(desktoph, 1, 0, 0);
int screenw = 0, screenh = 0;
SDL_Window *screen = NULL;
SDL_GLContext glcontext = NULL;

int curtime = 0, lastmillis = 1, elapsedtime = 0, totalmillis = 1;

void MainDetachCurrentThread();

void cleanup()
{
    recorder::stop();
    //cleanupserver();
    SDL_ShowCursor(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    if(screen) SDL_SetWindowGrab(screen, SDL_FALSE);
    cleargamma();
    freeocta(worldroot);

    extern void clear_command(); clear_command();
    extern void clear_console(); clear_console();
    extern void clear_mdls();  clear_mdls();
    extern void clear_sound();   clear_sound();
    closelogfile();
    #ifdef __APPLE__
        if(screen) SDL_SetWindowFullscreen(screen, 0);
    #endif
    SDL_Quit();
}

extern void writeinitcfg();

void TerminateHandler()
{
    logoutf("TerminateHandler called!");
}

void DestroyGLContext()
{
    if(glcontext)
    {
        SDL_GL_DeleteContext(glcontext);
        glcontext = NULL;
    }
    if(screen)
    {
        SDL_DestroyWindow(screen);
        screen = NULL;
    }
}

void quit() // normal exit
{
	logoutf("Quiting ...");
    std::set_terminate(TerminateHandler);
    MainDetachCurrentThread();
    if (screen) SDL_GetWindowPosition(screen, &winx, &winy); // Remembrt position of the window
    DestroyGLContext();
    writeinitcfg();
    //writeservercfg();
    //abortconnect();
    //disconnect();
    //localdisconnect();
    writecfg();
    cleanup();
    std::exit(EXIT_SUCCESS);
}

void fatal(const char *s, ...)    // failure exit
{
    static int errors = 0;
    errors++;

    if(errors <= 2) // print up to one extra recursive error
    {
        defvformatstring(msg,s,s);
        logoutf("FATAL ERROR: %s", msg);

        //if(errors <= 1) // avoid recursion
        //    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDLGL2 fatal error", msg, NULL);
    }
}

dynent *player_g = NULL;

int initing = NOT_INITING;

bool initwarning(const char *desc, int level, int type)
{
    if(initing < level)
    {
        addchange(desc, type);
        return true;
    }
    return false;
}

#define SCR_MINW 100
#define SCR_MINH 100
#define SCR_MAXW 10000
#define SCR_MAXH 10000
#define SCR_DEFAULTW 640
#define SCR_DEFAULTH 480
VARFN(screenw, scr_w, SCR_MINW, -1, SCR_MAXW, initwarning("screen resolution"));
VARFN(screenh, scr_h, SCR_MINH, -1, SCR_MAXH, initwarning("screen resolution"));

void writeinitcfg()
{
    stream *f = openutf8file(CONFIG_PATH "/init.cfg", "w");
    if(!f) return;
    extern int fullscreen;
    f->printf("fullscreen %d\n", fullscreen);
    f->printf("screenw %d\n", scr_w);
    f->printf("screenh %d\n", scr_h);
    f->printf("winx %d\n", winx);
    f->printf("winy %d\n", winy);
    extern int sound, soundchans, soundfreq, soundbufferlen;
    f->printf("sound %d\n", sound);
    f->printf("soundchans %d\n", soundchans);
    f->printf("soundfreq %d\n", soundfreq);
    f->printf("soundbufferlen %d\n", soundbufferlen);
    delete f;
}

COMMAND(quit, "");

static void getbackgroundres(int &w, int &h)
{
    float wk = 1, hk = 1;
    if(w < 1024) wk = 1024.0f/w;
    if(h < 768) hk = 768.0f/h;
    wk = hk = max(wk, hk);
    w = int(ceil(w*wk));
    h = int(ceil(h*hk));
}

string backgroundcaption = "";
Texture *backgroundmapshot = NULL;
string backgroundmapname = "";
char *backgroundmapinfo = NULL;

void bgquad(float x, float y, float w, float h, float tx = 0, float ty = 0, float tw = 1, float th = 1)
{
    gle::begin(GL_TRIANGLE_STRIP);
    gle::attribf(x,   y);   gle::attribf(tx,      ty);
    gle::attribf(x+w, y);   gle::attribf(tx + tw, ty);
    gle::attribf(x,   y+h); gle::attribf(tx,      ty + th);
    gle::attribf(x+w, y+h); gle::attribf(tx + tw, ty + th);
    gle::end();
}

void renderbackgroundview(int w, int h, const char *caption, Texture *mapshot, const char *mapname, const char *mapinfo)
{
#if 0
    static int lastupdate = -1, lastw = -1, lasth = -1;
    static float backgroundu = 0, backgroundv = 0;
    if((renderedframe && !mainmenu && lastupdate != lastmillis) || lastw != w || lasth != h)
    {
        lastupdate = lastmillis;
        lastw = w;
        lasth = h;

        backgroundu = rndscale(1);
        backgroundv = rndscale(1);
    }
    else if(lastupdate != lastmillis) lastupdate = lastmillis;

    hudmatrix.ortho(0, w, h, 0, -1, 1);
    resethudmatrix();
    resethudshader();

    gle::defvertex(2);
    gle::deftexcoord0();

    settexture("media/interface/background.png", 0, false);
    float bu = w*0.67f/256.0f, bv = h*0.67f/256.0f;
    bgquad(0, 0, w, h, backgroundu, backgroundv, bu, bv);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    settexture("media/interface/shadow.png", 3, false);
    bgquad(0, 0, w, h);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    float lh = 0.5f*std::min(w, h), lw = lh*2,
          lx = 0.5f*(w - lw), ly = 0.5f*(h*0.5f - lh);
    settexture((maxtexsize ? std::min(maxtexsize, hwtexsize) : hwtexsize) >= 1024 && (hudw > 1280 || hudh > 800) ? "<premul>media/interface/logo_1024.png" : "<premul>media/interface/logo.png", 3, false);
    bgquad(lx, ly, lw, lh);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(caption)
    {
        int tw = text_width(caption);
        float tsz = 0.08f*std::min(w, h)/FONTH,
              tx = 0.5f*(w - tw*tsz), ty = h/2.0f - FONTH*tsz;
        pushhudmatrix();
        hudmatrix.translate(tx, ty, 0);
        hudmatrix.scale(tsz, tsz, 1);
        flushhudmatrix();
        draw_text(caption, 0, 0);
        pophudmatrix();
    }
    if(mapshot || mapname)
    {
        float infowidth = 14*FONTH, sz = 0.35f*std::min(w, h), msz = (0.85f*std::min(w, h) - sz)/(infowidth + FONTH), x = 0.5f*w, y = ly+lh - sz/15, mx = 0, my = 0, mw = 0, mh = 0;
        if(mapinfo)
        {
            text_boundsf(mapinfo, mw, mh, infowidth);
            x -= 0.5f*mw*msz;
            if(mapshot && mapshot!=notexture)
            {
                x -= 0.5f*FONTH*msz;
                mx = sz + FONTH*msz;
            }
        }
        if(mapshot && mapshot!=notexture)
        {
            x -= 0.5f*sz;
            resethudshader();
            glBindTexture(GL_TEXTURE_2D, mapshot->id);
            bgquad(x, y, sz, sz);
        }
        if(mapname)
        {
            float tw = text_widthf(mapname), tsz = sz/(8*FONTH), tx = std::max(0.5f*(mw*msz - tw*tsz), 0.0f);
            pushhudmatrix();
            hudmatrix.translate(x+mx+tx, y, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(mapname, 0, 0);
            pophudmatrix();
            my = 1.5f*FONTH*tsz;
        }
        if(mapinfo)
        {
            pushhudmatrix();
            hudmatrix.translate(x+mx, y+my, 0);
            hudmatrix.scale(msz, msz, 1);
            flushhudmatrix();
            draw_text(mapinfo, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, -1, infowidth);
            pophudmatrix();
        }
    }

    glDisable(GL_BLEND);

    gle::disable();
#endif // 0
}

void setbackgroundinfo(const char *caption = NULL, Texture *mapshot = NULL, const char *mapname = NULL, const char *mapinfo = NULL)
{
    renderedframe = false;
    copystring(backgroundcaption, caption ? caption : "");
    backgroundmapshot = mapshot;
    copystring(backgroundmapname, mapname ? mapname : "");
    if(mapinfo != backgroundmapinfo)
    {
        DELETEA(backgroundmapinfo);
        if(mapinfo) backgroundmapinfo = newstring(mapinfo);
    }
}

void renderbackground(const char *caption, Texture *mapshot, const char *mapname, const char *mapinfo, bool restore, bool force)
{
   if(!inbetweenframes && !force) return;

    if(!restore || force) stopsounds(); // stop sounds while loading

    int w = screenw, h = screenh;
    if(forceaspect) w = int(ceil(h*forceaspect));
    getbackgroundres(w, h);
    gettextres(w, h);

    static int lastupdate = -1, lastw = -1, lasth = -1;
    static float backgroundu = 0, backgroundv = 0, detailu = 0, detailv = 0;
    static int numdecals = 0;
    static struct decal { float x, y, size; int side; } decals[12];
    if((renderedframe && !mainmenu && lastupdate != lastmillis) || lastw != w || lasth != h)
    {
        lastupdate = lastmillis;
        lastw = w;
        lasth = h;

        backgroundu = rndscale(1);
        backgroundv = rndscale(1);
        detailu = rndscale(1);
        detailv = rndscale(1);
        numdecals = sizeof(decals)/sizeof(decals[0]);
        numdecals = numdecals/3 + rnd((numdecals*2)/3 + 1);
        float maxsize = min(w, h)/16.0f;
        loopi(numdecals)
        {
            decal d = { rndscale(w), rndscale(h), maxsize/2 + rndscale(maxsize/2), rnd(2) };
            decals[i] = d;
        }
    }
    else if(lastupdate != lastmillis) lastupdate = lastmillis;

    loopi(restore ? 1 : 3)
    {
        hudmatrix.ortho(0, w, h, 0, -1, 1);
        resethudmatrix();

        hudshader->set();
        gle::colorf(1, 1, 1);

        gle::defvertex(2);
        gle::deftexcoord0();

        settexture("data/background.png", 0);
        float bu = w*0.67f/256.0f + backgroundu, bv = h*0.67f/256.0f + backgroundv;
        bgquad(0, 0, w, h, 0, 0, bu, bv);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        settexture("data/background_detail.png", 0);
        float du = w*0.8f/512.0f + detailu, dv = h*0.8f/512.0f + detailv;
        bgquad(0, 0, w, h, 0, 0, du, dv);
        settexture("data/background_decal.png", 3);
        gle::begin(GL_QUADS);
        loopj(numdecals)
        {
            float hsz = decals[j].size, hx = clamp(decals[j].x, hsz, w-hsz), hy = clamp(decals[j].y, hsz, h-hsz), side = decals[j].side;
            gle::attribf(hx-hsz, hy-hsz); gle::attribf(side,   0);
            gle::attribf(hx+hsz, hy-hsz); gle::attribf(1-side, 0);
            gle::attribf(hx+hsz, hy+hsz); gle::attribf(1-side, 1);
            gle::attribf(hx-hsz, hy+hsz); gle::attribf(side,   1);
        }
        gle::end();
        float lh = 0.5f*min(w, h), lw = lh*2,
              lx = 0.5f*(w - lw), ly = 0.5f*(h*0.5f - lh);
        settexture((maxtexsize ? min(maxtexsize, hwtexsize) : hwtexsize) >= 1024 && (screenw > 1280 || screenh > 800) ? "data/logo_1024.png" : "data/logo.png", 3);
        bgquad(lx, ly, lw, lh);
        if(caption)
        {
            int tw = text_width(caption);
            float tsz = 0.04f*min(w, h)/FONTH,
                  tx = 0.5f*(w - tw*tsz), ty = h - 0.075f*1.5f*min(w, h) - 1.25f*FONTH*tsz;
            pushhudmatrix();
            hudmatrix.translate(tx, ty, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(caption, 0, 0);
            pophudmatrix();
        }
        if(mapshot || mapname)
        {
            int infowidth = 12*FONTH;
            float sz = 0.35f*min(w, h), msz = (0.75f*min(w, h) - sz)/(infowidth + FONTH), x = 0.5f*(w-sz), y = ly+lh - sz/15;
            if(mapinfo)
            {
                int mw, mh;
                text_bounds(mapinfo, mw, mh, infowidth);
                x -= 0.5f*(mw*msz + FONTH*msz);
            }
            if(mapshot && mapshot!=notexture)
            {
                glBindTexture(GL_TEXTURE_2D, mapshot->id);
                bgquad(x, y, sz, sz);
            }
            else
            {
                int qw, qh;
                text_bounds("?", qw, qh);
                float qsz = sz*0.5f/max(qw, qh);
                pushhudmatrix();
                hudmatrix.translate(x + 0.5f*(sz - qw*qsz), y + 0.5f*(sz - qh*qsz), 0);
                hudmatrix.scale(qsz, qsz, 1);
                flushhudmatrix();
                draw_text("?", 0, 0);
                pophudmatrix();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            settexture("data/mapshot_frame.png", 3);
            bgquad(x, y, sz, sz);
            if(mapname)
            {
                int tw = text_width(mapname);
                float tsz = sz/(8*FONTH),
                      tx = 0.9f*sz - tw*tsz, ty = 0.9f*sz - FONTH*tsz;
                if(tx < 0.1f*sz) { tsz = 0.1f*sz/tw; tx = 0.1f; }
                pushhudmatrix();
                hudmatrix.translate(x+tx, y+ty, 0);
                hudmatrix.scale(tsz, tsz, 1);
                flushhudmatrix();
                draw_text(mapname, 0, 0);
                pophudmatrix();
            }
            if(mapinfo)
            {
                pushhudmatrix();
                hudmatrix.translate(x+sz+FONTH*msz, y, 0);
                hudmatrix.scale(msz, msz, 1);
                flushhudmatrix();
                draw_text(mapinfo, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, -1, infowidth);
                pophudmatrix();
            }
        }
        glDisable(GL_BLEND);
        if(!restore) swapbuffers(false);
    }

    if(!restore) setbackgroundinfo(caption, mapshot, mapname, mapinfo);
}

void restorebackground(int w, int h)
{
    if(renderedframe) return;
    renderbackgroundview(w, h, backgroundcaption[0] ? backgroundcaption : NULL, backgroundmapshot, backgroundmapname[0] ? backgroundmapname : NULL, backgroundmapinfo);
}

float loadprogress = 0;

void renderprogressview(int w, int h, float bar, const char *text)   // also used during loading
{
#if 0
    hudmatrix.ortho(0, w, h, 0, -1, 1);
    resethudmatrix();
    resethudshader();

    gle::defvertex(2);
    gle::deftexcoord0();

    float fh = 0.060f*std::min(w, h), fw = fh*15,
          fx = renderedframe ? w - fw - fh/4 : 0.5f*(w - fw),
          fy = renderedframe ? fh/4 : h - fh*1.5f;
    settexture("media/interface/loading_frame.png", 3, false);
    bgquad(fx, fy, fw, fh);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float bw = fw*(512 - 2*8)/512.0f, bh = fh*20/32.0f,
          bx = fx + fw*8/512.0f, by = fy + fh*6/32.0f,
          su1 = 0/32.0f, su2 = 8/32.0f, sw = fw*8/512.0f,
          eu1 = 24/32.0f, eu2 = 32/32.0f, ew = fw*8/512.0f,
          mw = bw - sw - ew,
          ex = bx+sw + std::max(mw*bar, fw*8/512.0f);
    if(bar > 0)
    {
        settexture("media/interface/loading_bar.png", 3, false);
        bgquad(bx, by, sw, bh, su1, 0, su2-su1, 1);
        bgquad(bx+sw, by, ex-(bx+sw), bh, su2, 0, eu1-su2, 1);
        bgquad(ex, by, ew, bh, eu1, 0, eu2-eu1, 1);
    }

    if (text)
    {
        int tw = text_width(text);
        float tsz = bh*0.6f/FONTH;
        if(tw*tsz > mw) tsz = mw/tw;
        pushhudmatrix();
        hudmatrix.translate(bx+sw, by + (bh - FONTH*tsz)/2, 0);
        hudmatrix.scale(tsz, tsz, 1);
        flushhudmatrix();
        draw_text(text, 0, 0);
        pophudmatrix();
    }

    glDisable(GL_BLEND);

    gle::disable();
#endif // 0
}

void renderprogress(float bar, const char *text, GLuint tex, bool background)   // also used during loading
{
    if (g_lite) return;
#if 0
    if(!inbetweenframes || drawtex) return;

    //clientkeepalive();      // make sure our connection doesn't time out while loading maps etc.

    #ifdef __APPLE__
    interceptkey(SDLK_UNKNOWN); // keep the event queue awake to avoid 'beachball' cursor
    #endif

    int w = hudw, h = hudh;
    if(forceaspect) w = int(ceil(h*forceaspect));
    getbackgroundres(w, h);
    gettextres(w, h);

    if(background) restorebackground(w, h);
    renderprogressview(w, h, bar, text);

    swapbuffers(false);
#endif // 0
}
ICOMMAND(renderprogress, "fsi", (float *bar, char *text, int *background), renderprogress(*bar, text, (*background) != 0); );

VARNP(relativemouse, userelativemouse, 0, 0, 1);

bool shouldgrab = false, grabinput = false, minimized = false, canrelativemouse = true, relativemouse = false;
int keyrepeatmask = 0, textinputmask = 0;
Uint32 textinputtime = 0;
VAR(textinputfilter, 0, 5, 1000);

void keyrepeat(bool on, int mask)
{
    if(on) keyrepeatmask |= mask;
    else keyrepeatmask &= ~mask;
}

void textinput(bool on, int mask)
{
    if (on)
    {
        if(!textinputmask)
        {
            SDL_StartTextInput();
            textinputtime = SDL_GetTicks();
        }
        textinputmask |= mask;
    }
    else
    {
        textinputmask &= ~mask;
        if(!textinputmask)
        {
            SDL_StopTextInput();
        }
    }
}

void inputgrab(bool on)
{
    if (!userelativemouse)
    {
        shouldgrab = false;
        SDL_ShowCursor(SDL_TRUE);
        return;
    }
    if(on)
    {
        SDL_ShowCursor(SDL_FALSE);
        if (canrelativemouse && userelativemouse)
        {
            if(SDL_SetRelativeMouseMode(SDL_TRUE) >= 0)
            {
                SDL_SetWindowGrab(screen, SDL_TRUE);
                relativemouse = true;
            }
            else
            {
                SDL_SetWindowGrab(screen, SDL_FALSE);
                canrelativemouse = false;
                relativemouse = false;
            }
        }
    }
    else
    {
        SDL_ShowCursor(SDL_TRUE);
        if (relativemouse)
        {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_SetWindowGrab(screen, SDL_FALSE);
            relativemouse = false;
        }
    }
    shouldgrab = false;
}

bool initwindowpos = false;

void setfullscreen(bool enable)
{
    if(!screen) return;
    //initwarning(enable ? "fullscreen" : "windowed");
    SDL_SetWindowFullscreen(screen, enable ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    if(!enable)
    {
        SDL_SetWindowSize(screen, scr_w, scr_h);
        if(initwindowpos)
        {
            SDL_SetWindowPosition(screen, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            initwindowpos = false;
        }
    }
}

VARF(fullscreen, 0, 0, 1, setfullscreen(fullscreen!=0));

void screenres(int w, int h)
{
    scr_w = clamp(w, SCR_MINW, SCR_MAXW);
    scr_h = clamp(h, SCR_MINH, SCR_MAXH);
    if(screen)
    {
        if(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN)
            gl_resize();
        else
            SDL_SetWindowSize(screen, scr_w, scr_h);
    }
    else
    {
        initwarning("screen resolution");
    }
}

ICOMMAND(screenres, "ii", (int *w, int *h), screenres(*w, *h));

static void setgamma(int val)
{
    if(screen && SDL_SetWindowBrightness(screen, val/100.0f) < 0) conoutf("Could not set gamma: %s", SDL_GetError());
}

static int curgamma = 100;
VARFP(gamma, 30, 100, 300,
{
    if(initing || gamma == curgamma) return;
    curgamma = gamma;
    setgamma(curgamma);
});

void restoregamma()
{
    if(initing || curgamma == 100) return;
    setgamma(curgamma);
}

void cleargamma()
{
    if(curgamma != 100 && screen) SDL_SetWindowBrightness(screen, 1.0f);
}

VAR(dbgmodes, 0, 0, 1);

static std::string GetTitle()
{
#ifdef _KINECT_GAME
    std::string dev = kinect_device_uri;
    std::string ret = std::string(
#ifdef _DEBUG
        "SDLGL 2 Debug"
#else
        "SDLGL 2 Release"
#endif
      );
    if (dev.length()) ret += ": " + dev;
    return ret;
#else
    return "SDLGL2";
#endif // _KINECT_GAME
}

void setupscreen()
{
    DestroyGLContext();

    SDL_DisplayMode desktop;
    if(SDL_GetDesktopDisplayMode(0, &desktop) < 0)
		fatal("failed querying desktop display mode: %s", SDL_GetError());
	else
	{
		desktopw = desktop.w;
		desktoph = desktop.h;
		logoutf("desktopw:%d desktoph:%d", desktopw, desktoph);
	}

    if(scr_h < 0) scr_h = SCR_DEFAULTH;
    if(scr_w < 0) scr_w = (scr_h*desktopw)/desktoph;
    scr_w = std::min(scr_w, desktopw);
    scr_h = std::min(scr_h, desktoph);
	logoutf("scr_w:%d scr_h:%d", scr_w, scr_h);

#ifdef __WINRT__
	winx = winy = 0;
	//fullscreen = 1;
#if 1
	//ApplicationView::GetForCurrentView()->SetDesiredBoundsMode(Windows::UI::ViewManagement::ApplicationViewBoundsMode::UseCoreWindow);
	ApplicationView::GetForCurrentView()->SetDesiredBoundsMode(Windows::UI::ViewManagement::ApplicationViewBoundsMode::UseVisible);
	ApplicationView::GetForCurrentView()->SuppressSystemOverlays = true;
#endif
#endif

    int winw = scr_w, winh = scr_h, flags = SDL_WINDOW_RESIZABLE;
    if(fullscreen)
    {
        winw = desktopw;
        winh = desktoph;
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        initwindowpos = true;
    }

    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifdef __ANDROID__ // Needed on ANDROID because resuming the app will show black!
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

    //SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    screen = SDL_CreateWindow(GetTitle().c_str(),
        winx, winy, winw, winh, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | flags);
	if (!screen)
	{
		fatal("failed to create OpenGL window: %s", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}

    SDL_SetWindowMinimumSize(screen, SCR_MINW, SCR_MINH);
    SDL_SetWindowMaximumSize(screen, SCR_MAXW, SCR_MAXH);

    // TEST
    //static const struct { int major, minor; } coreversions[] = { { 4, 0 }, { 3, 3 }, { 3, 2 }, { 3, 1 }, { 3, 0 }, {2, 1} };
    static const struct { int major, minor; } coreversions[] = { {2, 1} };
	//static const struct { int major, minor; } coreversions[] = { { 9, 3 } };
    loopi(sizeof(coreversions)/sizeof(coreversions[0]))
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, coreversions[i].major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, coreversions[i].minor);
#if !defined(__ANDROID__) && !defined(__WINRT__)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        //SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
        glcontext = SDL_GL_CreateContext(screen);
        if(glcontext)
        {
            logoutf("GL version %d.%d successfully created", coreversions[i].major, coreversions[i].minor);
            glversion = coreversions[i].major*100 + coreversions[i].minor*10;
            break;
        }
    }
    if(!glcontext)
    {
        logoutf("Trying default profile ... ");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
        glcontext = SDL_GL_CreateContext(screen);
		if (!glcontext)
		{
			fatal("failed to create OpenGL context: %s", SDL_GetError());
			std::exit(EXIT_FAILURE);
		}
        glversion = 210;
    }

    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);

	auto oldscreenw = screenw, oldscreenh = screenh;
	if (screen)
		SDL_GetWindowSize(screen, &screenw, &screenh);
	if (oldscreenw != screenw || oldscreenh != screenh)
	{
		logoutf("SDL_GetWindowSize changed: oldscreenw:%d oldscreenh:%d screenw:%d screenh:%d", oldscreenw, oldscreenh, screenw, screenh);
	}

#if defined(_WIN32) && !defined(__WINRT__)
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK )
        fatal("%s", (std::string("Error initializing GLEW! ") + (const char *)glewGetErrorString(glewError )).c_str());
#endif
}

void resetgl()
{
    //clearchanges(CHANGE_GFX|CHANGE_SHADERS);

    renderbackground("resetting OpenGL");

    extern void cleanupva();
    extern void cleanupparticles();
    extern void cleanupdecals();
    extern void cleanupblobs();
    extern void cleanupsky();
    extern void cleanupmodels();
    extern void cleanupprefabs();
    extern void cleanuplightmaps();
    extern void cleanupblendmap();
    extern void cleanshadowmap();
    extern void cleanreflections();
    extern void cleanupglare();
    extern void cleanupdepthfx();

    recorder::cleanup();

    cleanupva();
    cleanupparticles();
    //cleanupstains();
    cleanupsky();
    cleanupmodels();
    cleanupprefabs();

    cleanuptextures();

    cleanupblendmap();

    cleanupshaders();
    cleanupgl();

    setupscreen();

    inputgrab(grabinput);

    gl_init();

    inbetweenframes = false;
    if(!reloadtexture(*notexture) ||
       !reloadtexture("data/logo.png") ||
       !reloadtexture("data/logo_1024.png") ||
       !reloadtexture("data/background.png") ||
       !reloadtexture("data/background_detail.png") ||
       !reloadtexture("data/background_decal.png") ||
       !reloadtexture("data/mapshot_frame.png") ||
       !reloadtexture("data/loading_frame.png") ||
       !reloadtexture("data/loading_bar.png"))
        fatal("failed to reload core texture");
    reloadfonts();
    logoutf("reloadfonts");
    inbetweenframes = true;
    renderbackground("Loading ...");
    logoutf("renderbackground");
    restoregamma();
    logoutf("restoregamma");

    reloadshaders();
    logoutf("reloadshaders");
    reloadtextures();
    logoutf("reloadtextures");

    initlights();
    logoutf("initlights");
    allchanged(true);

    logoutf("resetgl done");
}

COMMAND(resetgl, "");

vector<SDL_Event> events;

void pushevent(const SDL_Event &e)
{
    events.add(e);
}

static bool filterevent(const SDL_Event &event)
{
    switch(event.type)
    {
        case SDL_MOUSEMOTION:
            if(grabinput && !relativemouse && !(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
            {
                if(event.motion.x == screenw / 2 && event.motion.y == screenh / 2)
                    return false;  // ignore any motion events generated by SDL_WarpMouse
                #ifdef __APPLE__
                if(event.motion.y == 0)
                    return false;  // let mac users drag windows via the title bar
                #endif
            }
            break;
    }
    return true;
}

static inline bool pollevent(SDL_Event &event)
{
    while(SDL_PollEvent(&event))
    {
        if(filterevent(event)) return true;
    }
    return false;
}

bool interceptkey(int sym)
{
    static int lastintercept = SDLK_UNKNOWN;
    int len = lastintercept == sym ? events.length() : 0;
    SDL_Event event;
    while(pollevent(event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION: break;
            default: pushevent(event); break;
        }
    }
    lastintercept = sym;
    if(sym != SDLK_UNKNOWN) for(int i = len; i < events.length(); i++)
    {
        if(events[i].type == SDL_KEYDOWN && events[i].key.keysym.sym == sym) { events.remove(i); return true; }
    }
    return false;
}

//static void ignoremousemotion()
//{
//    SDL_Event e;
//    SDL_PumpEvents();
//    while(SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION));
//}

static void resetmousemotion()
{
    if(grabinput && !relativemouse && userelativemouse && !(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
    {
        SDL_WarpMouseInWindow(screen, screenw / 2, screenh / 2);
    }
}

static void checkmousemotion(int &dx, int &dy)
{
    loopv(events)
    {
        SDL_Event &event = events[i];
        if(event.type != SDL_MOUSEMOTION)
        {
            if(i > 0) events.remove(0, i);
            return;
        }
        dx += event.motion.xrel;
        dy += event.motion.yrel;
    }
    events.setsize(0);
    SDL_Event event;
    while(pollevent(event))
    {
        if(event.type != SDL_MOUSEMOTION)
        {
            events.add(event);
            return;
        }
        dx += event.motion.xrel;
        dy += event.motion.yrel;
    }
}

void check_doubleclick(bool pressed)
{
    if (pressed)
    {
        int diff = totalmillis - _clickMs;
        if (diff < doubleclicktime)
        {
            _doubleClick = true;
            logoutf("Double click. diff(%d ms)", diff);
            processkey(-6, true);
        }
        else
        {
            //logoutf("Normal click. diff(%d ms)", diff);
            _doubleClick = false;
            processkey(0x40000052, false);
            processkey(-6, false);
        }
        _clickMs = totalmillis;
    }
    else if (_doubleClick)
    {
        processkey(-6, false);
        _doubleClick = false;
        processkey(0x40000052, false);
    }
}

// BEGIN PROCESS EVENTS functions
void ProcessMouseMoved(float x, float y);
void ProcessMousePressed(uint8_t button, float x, float y)
{
	ProcessMouseMoved(x, y);
	switch (button)
	{
	case SDL_BUTTON_LEFT:
		check_doubleclick(true);
		processkey(-1, true);
		break;
	case SDL_BUTTON_MIDDLE: processkey(-2, true); break;
	case SDL_BUTTON_RIGHT: processkey(-3, true); break;
	case SDL_BUTTON_X1: processkey(-6, true); break;
	case SDL_BUTTON_X2: processkey(-7, false); break;
	}
}
void ProcessMouseMoved(float x, float y)
{
    if(grabinput)
    {
        float dx = x - cursorx, dy = y - cursory;
        int ix = dx*screenw, iy = dy*screenh;
        //checkmousemotion(ix, iy);
        if(!g3d_movecursor(ix, iy))
            mousemove(ix, iy);
    }
    else if(shouldgrab) inputgrab(grabinput = true);
}
void ProcessMouseReleased(uint8_t button, float x, float y)
{
	switch (button)
	{
	case SDL_BUTTON_LEFT:
		check_doubleclick(false);
		processkey(-1, false);
		break;
	case SDL_BUTTON_MIDDLE: processkey(-2, false); break;
	case SDL_BUTTON_RIGHT: processkey(-3, false); break;
	case SDL_BUTTON_X1: processkey(-6, false); break;
	case SDL_BUTTON_X2: processkey(-7, false); break;
	}
}
void ProcessKey(SDL_Keycode sym, bool down, bool repeat)
{
	if (keyrepeatmask || !repeat)
		processkey(sym, down);
}
// END PROCESS EVENTS functions

// BEGIN External Input support
VAR(showExternalInputDialog, 0, 0, 1);
SVAR(inputalias, "");
bool SetExternalInput(const char *input)
{
	if (!inputalias || strlen(inputalias) == 0)
	{
		logoutf("ERROR: inputalias is not set! input:'%s'", input);
		assert(inputalias && strlen(inputalias) > 0);
		return false;
	}
	setsvar(inputalias, input);
	writecfg();

	return true;
}
// END External Input support

void checkinput()
{
    SDL_Event event;
    //int lasttype = 0, lastbut = 0;
    bool mousemoved = false;
    while(events.length() || pollevent(event))
    {
        if(events.length()) event = events.remove(0);

        //logoutf("SDL event:%d", event.type);

        switch(event.type)
        {
            case SDL_QUIT:
                quit();
                return;

            case SDL_TEXTINPUT:
				//logoutf("SDL_TEXTINPUT");
                if(textinputmask && int(event.text.timestamp-textinputtime) >= textinputfilter)
                {
                    uchar buf[SDL_TEXTINPUTEVENT_TEXT_SIZE+1];
                    size_t len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)event.text.text, strlen(event.text.text));
					//logoutf("process text: '%s'", buf);
                    if(len > 0) { buf[len] = '\0'; processtextinput((const char *)buf, len); }
                }
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
				ProcessKey(event.key.keysym.sym, event.key.state == SDL_PRESSED, event.key.repeat != 0);
                break;

            case SDL_WINDOWEVENT:
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE:
                        quit();
                        break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        shouldgrab = true;
                        if (userelativemouse) break;
                    case SDL_WINDOWEVENT_ENTER:
                    {
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // test
#ifndef __WINRT__
                        if (!userelativemouse && screenw && screenh)
                        {
                            SDL_GetMouseState(&sposx, &sposy);
							logoutf("SDL_WINDOWEVENT_ENTER: sposx:%d sposy:%d screenw:%d screenh:%d", sposx, sposy, screenw, screenh);
                            setcursorpos(sposx, sposy, false);
                            mousemoved = true;
                        }
#endif
                        inputgrab(grabinput = true);
                        break;
                    }

                    case SDL_WINDOWEVENT_LEAVE:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    {
                        inputgrab(grabinput = false);
                        break;
                    }

                    case SDL_WINDOWEVENT_MINIMIZED:
                        // TODO: XXX: TEST
//#warning Disabled minimization
                        //minimized = true;
                        break;

                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_RESTORED:
                        minimized = false;
                        break;

                    case SDL_WINDOWEVENT_RESIZED:
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        SDL_GetWindowSize(screen, &screenw, &screenh);
						logoutf("SDL_WINDOWEVENT_SIZE_CHANGED: screenw:%d screenh:%d", screenw, screenh);
                        if(!(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
                        {
                            scr_w = clamp(screenw, SCR_MINW, SCR_MAXW);
                            scr_h = clamp(screenh, SCR_MINH, SCR_MAXH);
                        }
                        gl_resize();
                        logoutf("Swapping window workaround ...");
                        SDL_GL_SwapWindow(screen);
                        break;
                }
                break;

#if !defined(__ANDROID__) //&& !defined(__WINRT__)
            case SDL_MOUSEMOTION:
                if(grabinput)
                {
                    int dx = 0, dy = 0;
                    if (userelativemouse)
                    {
                        dx = event.motion.xrel, dy = event.motion.yrel;
                        checkmousemotion(dx, dy);
                    }
                    else
                    {
#ifndef __WINRT__
                        int posx = 0, posy = 0;
                        SDL_GetMouseState(&posx, &posy);
                        if (sposx == -1) sposx = posx;
                        if (sposy == -1) sposy = posy;
                        dx = posx - sposx; dy = posy - sposy;
						//logoutf("SDL_MOUSEMOTION: screenw:%d screenh:%d", screenw, screenh);
						//logoutf("SDL_MOUSEMOTION: posx:%d posy:%d sposx:%d sposy:%d dx:%d dy:%d screenw:%d screenh:%d", posx, posy, sposx, sposy, dx, dy, screenw, screenh);
						//logoutf("SDL_MOUSEMOTION: dx:%d dy:%d screenw:%d screenh:%d", dx, dy, screenw, screenh);
                        sposx = posx; sposy = posy;
#endif
                    }

					if (!g3d_movecursor(dx, dy))
					{
						mousemove(dx, dy);
					}
                    mousemoved = true;
                }
                else if(shouldgrab) inputgrab(grabinput = true);
                break;
#else
            case SDL_FINGERMOTION:
				logoutf("finger motion: x:%f y:%f dx:%f dy:%f", event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy);
                ProcessMouseMoved(event.tfinger.x, event.tfinger.y);
                break;
#endif // __ANDROID__

#if !defined(__ANDROID__) //&& !defined(__WINRT__)
            case SDL_MOUSEBUTTONDOWN:
				ProcessMousePressed(event.button.button, cursorx, cursory);
				break;
            case SDL_MOUSEBUTTONUP:
				ProcessMouseReleased(event.button.button, cursorx, cursory);
                break;
#else
            case SDL_FINGERDOWN:
				logoutf("finger down: x:%f y:%f dx:%f dy:%f sw:%d sh:%d", event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy, screenw, screenh);
                ProcessMousePressed(SDL_BUTTON_LEFT, event.tfinger.x, event.tfinger.y);
                break;
            case SDL_FINGERUP:
				logoutf("finger up: x:%f y:%f dx:%f dy:%f sw:%d sh:%d", event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy, screenw, screenh);
                ProcessMouseReleased(SDL_BUTTON_LEFT, event.tfinger.x, event.tfinger.y);
                break;

#endif // __ANDROID__

            case SDL_MOUSEWHEEL:
                if(event.wheel.y > 0) { processkey(-4, true); processkey(-4, false); }
                else if(event.wheel.y < 0) { processkey(-5, true); processkey(-5, false); }
                break;

            case SDL_DROPFILE:
                break;
        }
    }
    if(mousemoved) resetmousemotion();
}

void swapbuffers(bool overlay)
{
    recorder::capture(overlay);
    assert(screen);
	if (screen)
	{
		if (glcontext)
			SDL_GL_MakeCurrent(screen, glcontext);
		SDL_GL_SwapWindow(screen);
	}
}

VAR(menufps, 0, 60, 1000);
VARP(maxfps, 0, 125, 1000);

void limitfps(int &millis, int curmillis)
{
    int limit = (mainmenu || minimized) && menufps ? (maxfps ? std::min(maxfps, menufps) : menufps) : maxfps;
    if(!limit) return;
    static int fpserror = 0;
    int delay = 1000/limit - (millis-curmillis);
    if(delay < 0) fpserror = 0;
    else
    {
        fpserror += 1000%limit;
        if(fpserror >= limit)
        {
            ++delay;
            fpserror -= limit;
        }
        if(delay > 0)
        {
            SDL_Delay(delay);
            millis += delay;
        }
    }
}

/*
#if !defined(__WINRT__) && defined(WIN32) && !defined(_DEBUG) && !defined(__GNUC__)
void stackdumper(unsigned int type, EXCEPTION_POINTERS *ep)
{
    if(!ep) fatal("unknown type");
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    CONTEXT *context = ep->ContextRecord;
    char out[512];
    formatstring(out, "SDLGL2 Win32 Exception: 0x%x [0x%x]\n\n", er->ExceptionCode, er->ExceptionCode==EXCEPTION_ACCESS_VIOLATION ? er->ExceptionInformation[1] : -1);
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
#ifdef _AMD64_
    STACKFRAME64 sf = {{context->Rip, 0, AddrModeFlat}, {}, {context->Rbp, 0, AddrModeFlat}, {context->Rsp, 0, AddrModeFlat}, 0};
    while(::StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), &sf, context, NULL, ::SymFunctionTableAccess, ::SymGetModuleBase, NULL))
    {
        union { IMAGEHLP_SYMBOL64 sym; char symext[sizeof(IMAGEHLP_SYMBOL64) + sizeof(string)]; };
        sym.SizeOfStruct = sizeof(sym);
        sym.MaxNameLength = sizeof(symext) - sizeof(sym);
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(line);
        DWORD64 symoff;
        DWORD lineoff;
        if(SymGetSymFromAddr64(GetCurrentProcess(), sf.AddrPC.Offset, &symoff, &sym) && SymGetLineFromAddr64(GetCurrentProcess(), sf.AddrPC.Offset, &lineoff, &line))
#else
    STACKFRAME sf = {{context->Eip, 0, AddrModeFlat}, {}, {context->Ebp, 0, AddrModeFlat}, {context->Esp, 0, AddrModeFlat}, 0};
    while(::StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &sf, context, NULL, ::SymFunctionTableAccess, ::SymGetModuleBase, NULL))
    {
        union { IMAGEHLP_SYMBOL sym; char symext[sizeof(IMAGEHLP_SYMBOL) + sizeof(string)]; };
        sym.SizeOfStruct = sizeof(sym);
        sym.MaxNameLength = sizeof(symext) - sizeof(sym);
        IMAGEHLP_LINE line;
        line.SizeOfStruct = sizeof(line);
        DWORD symoff, lineoff;
        if(SymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &symoff, &sym) && SymGetLineFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &lineoff, &line))
#endif
        {
            char *del = strrchr(line.FileName, '\\');
            concformatstring(out, "%s - %s [%d]\n", sym.Name, del ? del + 1 : line.FileName, line.LineNumber);
        }
    }
    fatal(out);
}
#endif
*/

#define MAXFPSHISTORY 60

int fpspos = 0, fpshistory[MAXFPSHISTORY];

void resetfpshistory()
{
    loopi(MAXFPSHISTORY) fpshistory[i] = 1;
    fpspos = 0;
}

void updatefpshistory(int millis)
{
    fpshistory[fpspos++] = std::max(1, std::min(1000, millis));
    if(fpspos>=MAXFPSHISTORY) fpspos = 0;
}

void getframemillis(float &avg, float &bestdiff, float &worstdiff)
{
    int total = fpshistory[MAXFPSHISTORY-1], best = total, worst = total;
    loopi(MAXFPSHISTORY-1)
    {
        int millis = fpshistory[i];
        total += millis;
        if(millis < best) best = millis;
        if(millis > worst) worst = millis;
    }

    avg = total/float(MAXFPSHISTORY);
    best = best - avg;
    worstdiff = avg - worst;
}

void getfps(int &fps, int &bestdiff, int &worstdiff)
{
    int total = fpshistory[MAXFPSHISTORY-1], best = total, worst = total;
    loopi(MAXFPSHISTORY-1)
    {
        int millis = fpshistory[i];
        total += millis;
        if(millis < best) best = millis;
        if(millis > worst) worst = millis;
    }

    fps = (1000*MAXFPSHISTORY)/total;
    bestdiff = 1000/best-fps;
    worstdiff = fps-1000/worst;
}

void getfps_(int *raw)
{
    if(*raw) floatret(1000.0f/fpshistory[(fpspos+MAXFPSHISTORY-1)%MAXFPSHISTORY]);
    else
    {
        int fps, bestdiff, worstdiff;
        getfps(fps, bestdiff, worstdiff);
        intret(fps);
    }
}

COMMANDN(getfps, getfps_, "i");

bool inbetweenframes = false, renderedframe = true;

static int findarg(int argc, char **argv, const char *str)
{
    for(int i = 1; i<argc; i++) if(strstr(argv[i], str)==argv[i]) return i;
    return -1;
}

VAR(numcpus, 1, 1, 16);

VAR(numpreviewplayers, 1, 1, 6);

static bool _mainStarted = false;
unsigned long GUI_threadId = 0;

bool Main_Start(int argc, char **argv)
{
	if (_mainStarted)
	{
		logoutf("ERROR: Main was already started!");
		return true;
	}

	GUI_threadId = SDL_GetThreadID(nullptr);

	int dedicated = 0;
	char *load = NULL, *initscript = NULL;

	extern std::recursive_mutex *_sdlLogMutex;
	_sdlLogMutex = new std::recursive_mutex();

	// Test for write access
#if 0
	{
		FileIO fi;
		if (!fi.Open("testfile", "w"))
		{
			logoutf("TEST WRITE ACCESS: Could not write testfile :(");
		}
		else
		{
			logoutf("TEST WRITE ACCESS: COULD write testfile! :)");
		}
	}
#endif // 0

	// Get assets_folder
	execfile("assets.cfg", RootLocation);
	// Init FileIO
	FileIO::Init();
	initing = INIT_RESET;
	for (int i = 1; i<argc; i++)
	{
		if (argv[i][0] == '-') switch (argv[i][1])
		{
		case 'u':
		{
			const char *dir = sethomedir(&argv[i][2]);
			if (dir) logoutf("Using home directory: %s", dir);
			break;
		}
		}
	}
	execfile(CONFIG_PATH "/init.cfg");

	// Try to root folder
	FileIO fi;
	if (!fi.Open(NOTEXTURE_PATH, "rb")) my_chdir("..");
	else fi.Close();

#ifndef __ANDROID__
	createdir("obj");
	createdir("faces");
#endif // __ANDROID__

	bool setLogFile = false;
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-') switch (argv[i][1])
		{
		case 'u': /* parsed first */ break;
		case 'k':
		{
			const char *dir = addpackagedir(&argv[i][2]);
			if (dir) logoutf("Adding package directory: %s", dir);
			break;
		}
		case 'g': logoutf("Setting log file: %s", &argv[i][2]); setlogfile(&argv[i][2]); setLogFile = true; break;
		case 'd': dedicated = atoi(&argv[i][2]); if (dedicated <= 0) dedicated = 2; break;
		case 'w': scr_w = clamp(atoi(&argv[i][2]), SCR_MINW, SCR_MAXW); if (findarg(argc, argv, "-h") < 0) scr_h = -1; break;
		case 'h': scr_h = clamp(atoi(&argv[i][2]), SCR_MINH, SCR_MAXH); if (findarg(argc, argv, "-w") < 0) scr_w = -1; break;
		case 'f': fullscreen = atoi(&argv[i][2]); break;
		case 'l':
		{
			char pkgdir[] = "media/";
			load = strstr(path(&argv[i][2]), path(pkgdir));
			if (load) load += sizeof(pkgdir) - 1;
			else load = &argv[i][2];
			break;
		}
		case 'x': initscript = &argv[i][2]; break;
		default:
			if (!serveroption(argv[i])) gameargs.add(argv[i]);
			break;
		}
		else
		{
#define _OPEN_URI(ext, prefix) \
            if (strstr(argv[i], ext)) \
            { \
                execfile(CONFIG_PATH "/init.cfg"); \
                std::string code = "kinect_device_uri \"" prefix; code += argv[i]; code += '\"'; \
                execute(code.c_str()); \
            } \
            else

			_OPEN_URI(".dat", "rec://")
				_OPEN_URI(".face", "face://")
				_OPEN_URI(".depth", "depth://")
				gameargs.add(argv[i]); // last else
#undef _OPEN_URI
		}
	}
	if (!setLogFile)
		setlogfile(appLogPath());

	logoutf("FileIO: Write folder path: '%s'", FileIO::GetWritePath());

	numcpus = clamp(SDL_GetCPUCount(), 1, 16);

	if (dedicated <= 1)
	{
		logoutf("init: sdl");
		Uint32 flags = SDL_INIT_TIMER | SDL_INIT_VIDEO;
		// TODO: XXX
		if (!nosound)
			flags |= SDL_INIT_AUDIO;
		if (SDL_Init(flags) < 0)
				fatal("Unable to initialize SDL: %s", SDL_GetError());

		logoutf("init: img");
		if (!IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP))
			fatal("Unable to initialize IMG: %s", SDL_GetError());
	}

	logoutf("init: game");
	game::parseoptions(gameargs);
	//initserver(dedicated>0, dedicated>1);  // never returns if dedicated
	ASSERT(dedicated <= 1);
	game::initclient();

	logoutf("init: video");
	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "0");
#if !defined(WIN32) && !defined(__APPLE__)
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#endif
	setupscreen();
	SDL_ShowCursor(SDL_FALSE);
	if (relativemouse) // Do not warp mouse with non relative mouse
		SDL_WarpMouseInWindow(screen, screenw / 2, screenh / 2);
	SDL_StopTextInput(); // workaround for spurious text-input events getting sent on first text input toggle?

    // Enable file drop
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	logoutf("init: gl");
	gl_checkextensions();
	logoutf("hwtexsize:%d", hwtexsize);
	logoutf("init: gl_init");
	gl_init();

#if 1
	FileIO io;
	if (!io.Open(NOTEXTURE_PATH, "rb"))
	{
		conoutf("RAW io failed for notexture");
		exit(EXIT_FAILURE);
	}
#endif

	notexture = textureload(NOTEXTURE_PATH);
	if (!notexture)
	{
		fatal("could not find core textures");
		exit(EXIT_FAILURE);
	}

	// Random seed
	seedMT(5489U + time(NULL));

	logoutf("init: console");
	if (!execfile(CONFIG_PATH "/stdlib.cfg")) fatal("cannot find data files (you are running from the wrong folder, try .bat file in the main folder)");   // this is the first file we load.
	if (!execfile(CONFIG_PATH "/font.cfg")) fatal("cannot find font definitions");
	if (!setfont("default")) fatal("no default font specified");

	inbetweenframes = true;
	renderbackground("Loading ...");

	logoutf("init: world");
	camera1 = player_g = game::iterdynents(0);
	emptymap(0, true, NULL, false);

	if (!nosound)
	{
		logoutf("init: sound");
		initsound();
	}

	logoutf("init: cfg");
	initing = INIT_LOAD;
    execfile("data/keymap.cfg");
    execfile("data/stdedit.cfg");
    execfile("data/sounds.cfg");
    execfile("data/menus.cfg");
    execfile("data/heightmap.cfg");
    execfile("data/blendbrush.cfg");
    defformatstring(gamecfgname, "data/game_%s.cfg", game::gameident());
    execfile(gamecfgname);
    if(game::savedservers()) execfile(game::savedservers(), false);

	identflags |= IDF_PERSIST;
	// First try to execute saved from write location and it case it fails, try from assets location
	//if (!execfile(savedconfig(), WriteLocation) && !execfile(savedconfig(), AssetsLocation))
	if (!execfile(savedconfig(), WriteLocation))
	{
		execfile(defaultconfig());
		writecfg(restoreconfig());
	}

#if 1
    // This is needed in case something depenfding on this is called from autoexec
	aspect = forceaspect ? forceaspect : screenw/float(screenh);
    fovy = 20;
    //curfov = 2.0f*atan2(tan(fovy/2.0f*RAD), 1.0f/aspect)/RAD;
    //farplane = 1024;
    //vieww = hudw;
    //viewh = hudh;
    //ldrscale = 1.0f;
    //ldrscaleb = ldrscale/255;
#endif // 1

	execfile(CONFIG_PATH "/autoexec.cfg");

	// Update title
	if (screen) SDL_SetWindowTitle(screen, GetTitle().c_str());

	identflags &= ~IDF_PERSIST;

	initing = INIT_GAME;
	game::loadconfigs();

	initing = NOT_INITING;

	logoutf("init: render");
	restoregamma();
	loadshaders();
    initparticles();
	identflags |= IDF_PERSIST;

	logoutf("init: mainloop");

	if (execfile(CONFIG_PATH "/once.cfg")) remove(findfile(CONFIG_PATH "once.cfg", "rb"));

	if (load)
	{
		logoutf("init: localconnect");
		//localconnect();
		game::changemap(load);
	}

	if (initscript) execute(initscript);

	initmumble();
	resetfpshistory();

	inputgrab(grabinput = true);
	//ignoremousemotion();

	_mainStarted = true;

	return true;
}

void Main_LoopOnce()
{
	if (!_mainStarted)
	{
		logoutf("ERROR: Main was not started");
		Main_Start(0, nullptr);
		return;
	}

#if 1
	static int frames = 0;
	int millis = getclockmillis();
	//limitfps(millis, totalmillis);
	elapsedtime = millis - totalmillis;
	static int timeerr = 0;
	int scaledtime = game::scaletime(elapsedtime) + timeerr;
	curtime = scaledtime / 100;
	timeerr = scaledtime % 100;
	//if (curtime>200) curtime = 200;
	if (game::ispaused()) curtime = 0;
	lastmillis += curtime;
	totalmillis = millis;
	//updatetime();

	// TEST
	//glViewport(0, 0, screenw, screenh);
	//conoutf("glViewport: sw:%d sh:%d", screenw, screenh);

	checkinput();
	menuprocess();
	tryedit();

	if (lastmillis)
        game::updateworld();

	checksleep(lastmillis);

	//serverslice(false, 0);

	if (frames) updatefpshistory(elapsedtime);
	frames++;

	// miscellaneous general game effects
	if (!mainmenu)
    {
        recomputecamera();
        updateparticles();
    }

    updatesounds();

	if (minimized) return; // No drawing when minimized

	inbetweenframes = false;

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // test

    if (mainmenu)
        gl_drawmainmenu();
    else
        gl_drawframe();

    g3d_render();
    gl_drawhud();

	swapbuffers();
	renderedframe = inbetweenframes = true;
#else
    static int frames = 0;
    int millis = getclockmillis();
    limitfps(millis, totalmillis);
    elapsedtime = millis - totalmillis;
    static int timeerr = 0;
    int scaledtime = game::scaletime(elapsedtime) + timeerr;
    curtime = scaledtime/100;
    timeerr = scaledtime%100;
    if(!multiplayer(false) && curtime>200) curtime = 200;
    if(game::ispaused()) curtime = 0;
    lastmillis += curtime;
    totalmillis = millis;
    updatetime();

    checkinput();
    menuprocess();
    tryedit();

    if(lastmillis) game::updateworld();

    checksleep(lastmillis);

    serverslice(false, 0);

    if(frames) updatefpshistory(elapsedtime);
    frames++;

    // miscellaneous general game effects
    recomputecamera();
    updateparticles();
    updatesounds();

    //if(minimized) continue;

    inbetweenframes = false;
    if(mainmenu)
        gl_drawmainmenu();
    else
        gl_drawframe();
    swapbuffers();
    renderedframe = inbetweenframes = true;
#endif // 0
}

int main(int argc, char **argv)
{
    /*#ifdef WIN32
    //atexit((void (__cdecl *)(void))_CrtDumpMemoryLeaks);
    #ifndef _DEBUG
    #ifndef __GNUC__
    __try {
    #endif
    #endif
    #endif*/

	if (!Main_Start(argc, argv))
	{
		conoutf("Init failed");
		return -1;
	}

    // Main loop
    for(;;)
		Main_LoopOnce();

    ASSERT(0);
    return EXIT_FAILURE;

    /*#if defined(WIN32) && !defined(_DEBUG) && !defined(__GNUC__)
    } __except(stackdumper(0, GetExceptionInformation()), EXCEPTION_CONTINUE_SEARCH) { return 0; }
    #endif */
}

// BEGIN Rating

#ifdef __ANDROID__

#include <jni.h>

extern "C"
{

static jmethodID java_ShowRating = nullptr;
static jmethodID java_ShowAds = nullptr;
static jmethodID java_Notify = nullptr;
static JavaVM *java_VM = nullptr;
static jclass java_ActivityClass = nullptr;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    SDL_Log("JNI_OnLoad");
    java_VM = vm;
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL Java_org_common_SDLActivity_nativeRegister
(JNIEnv* env, jclass jcls)
{
    SDL_Log("nativeRegister");

    java_ActivityClass = (jclass)(env->NewGlobalRef(jcls));
    java_ShowRating = env->GetStaticMethodID(java_ActivityClass, "ShowRating", "()V");
    if (!java_ShowRating)
        SDL_Log("ERROR: SDLActivity.ShowRating() method not found!");
    java_ShowAds = env->GetStaticMethodID(java_ActivityClass, "ShowAds", "()V");
    if (!java_ShowAds)
        SDL_Log("ERROR: SDLActivity.ShowAds() method not found!");
    java_Notify = env->GetStaticMethodID(java_ActivityClass, "Notify", "(Ljava/lang/String;)V");
    if (!java_Notify)
        SDL_Log("ERROR: SDLActivity.Notify() method not found!");
}

JNIEXPORT void JNICALL Java_org_common_SDLActivity_nativeSetBannerHeight
(JNIEnv* env, jclass jcls, jfloat val)
{
    SDL_Log("Banner height:%f", val);
}

JNIEXPORT void JNICALL Java_org_common_SDLActivity_nativeIgnoreRating
(JNIEnv* env, jclass jcls)
{
}

} // extern
#endif // ANDROID

void MainShowRatingDialog()
{
}

void MainShowAds()
{
}

void Notify(const char *message)
{
#ifdef __ANDROID__
    if (!java_VM)
    {
        SDL_Log("ERROR: no java_VM");
        return;
    }
    if (java_ActivityClass && java_Notify)
    {
        JNIEnv *env = nullptr;
        int status = java_VM->AttachCurrentThread(&env, NULL);
        if(status < 0)
        {
            SDL_Log("ERROR: failed to attach current thread");
            return;
        }
        if (!env)
        {
            SDL_Log("ERROR: Environment is NULL!");
            return;
        }

        jstring jStringParam = env->NewStringUTF(message);
        if (!jStringParam)
        {
            SDL_Log( "ERROR: Failed to alloc param string in java." );
            return;
        };
        env->CallStaticVoidMethod(java_ActivityClass, java_Notify, jStringParam);
        env->DeleteLocalRef(jStringParam);
    }
    else
    {
        SDL_Log("ERROR: Could not call Notify Java method!");
    }
#endif // __ANDROID__
#ifdef __WINRT__
	// See the Toasts SDK Sample for other ways of generating toasts.
	auto toastXml = ref new Windows::Data::Xml::Dom::XmlDocument();
	std::wstringstream ss;
	ss <<
		L"<toast launch = \"app-defined-string\">\
			<visual>\
				<binding template = \"ToastGeneric\">\
					<text>" << message << "</text>\
					<image placement = \"appLogoOverride\" src = \"A.png\"/>\
				</binding>\
			</visual>\
		</toast>";

	toastXml->LoadXml(ref new Platform::String(ss.str().c_str()));

	auto toast = ref new Windows::UI::Notifications::ToastNotification(toastXml);
	// Raise the toast.
	Windows::UI::Notifications::ToastNotificationManager::CreateToastNotifier()->Show(toast);
#endif // __WINRT__
}
ICOMMAND(app_notify, "s", (char *message), Notify(message));

void MainDetachCurrentThread()
{
#ifdef __ANDROID__
    JNIEnv *env = nullptr;
    bool attached = false;
    if (!java_VM)
    {
        SDL_Log("ERROR: no java_VM");
        return;
    }
    auto status = java_VM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED)
    {
        SDL_Log("INFO: Not attached");
        return;
    }
    SDL_Log("INFO: Detaching ...");
    java_VM->DetachCurrentThread();
#endif // __ANDROID__
}

// End Rating and Ads
