// implementation of generic tools

#include "engine.h"

#include "FileIO.h"

#ifdef _WIN32
#include <windows.h> // for Sleep
#include <Winsock2.h> // for htonl
#else
#include <unistd.h> // for usleep
#include <arpa/inet.h> // for htonl
#endif // __ANDROID__

#include <sstream>

extern const char *appLogPath();

////////////////////////// strings ////////////////////////////////////////

static string tmpstr[4];
static int tmpidx = 0;

char *tempformatstring(const char *fmt, ...)
{
    tmpidx = (tmpidx+1)%4;
    char *buf = tmpstr[tmpidx];

    va_list v;
    va_start(v, fmt);
    vformatstring(buf, fmt, v);
    va_end(v);

    return buf;
}

////////////////////////// rnd numbers ////////////////////////////////////////

#define N (624)
#define M (397)
#define K (0x9908B0DFU)

static uint state[N];
static int next = N;

void seedMT(uint seed)
{
    state[0] = seed;
    for(uint i = 1; i < N; i++)
        state[i] = seed = 1812433253U * (seed ^ (seed >> 30)) + i;
    next = 0;
}

uint randomMT()
{
    int cur = next;
    if(++next >= N)
    {
        if(next > N)
        {
            seedMT(5489U + time(NULL));
            cur = next++;
        }
        else next = 0;
    }
    uint y = (state[cur] & 0x80000000U) | (state[next] & 0x7FFFFFFFU);
    state[cur] = y = state[cur < N-M ? cur + M : cur + M-N] ^ (y >> 1) ^ (-int(y & 1U) & K);
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9D2C5680U;
    y ^= (y << 15) & 0xEFC60000U;
    y ^= (y >> 18);
    return y;
}

#undef N
#undef M
#undef K

///////////////////////// network ///////////////////////

// all network traffic is in 32bit ints, which are then compressed using the following simple scheme (assumes that most values are small).

template<class T>
static inline void putint_(T &p, int n)
{
    if(n<128 && n>-127) p.put(n);
    else if(n<0x8000 && n>=-0x8000)
    {
        p.put(0x80);
        p.put(n);
        p.put(n>>8);
    }
    else
    {
        p.put(0x81);
        p.put(n);
        p.put(n>>8);
        p.put(n>>16);
        p.put(n>>24);
    }
}
void putint(ucharbuf &p, int n)
{
    putint_(p, n);
}
void putint(vector<uchar> &p, int n)
{
    putint_(p, n);
}
void putint(packetbuf &p, int n) { putint_(p, n); }

int getint(ucharbuf &p)
{
    int c = (signed char)p.get();
    if(c==-128)
    {
        int n = p.get();
        n |= char(p.get())<<8;
        return n;
    }
    else if(c==-127)
    {
        int n = p.get();
        n |= p.get()<<8;
        n |= p.get()<<16;
        return n | (p.get()<<24);
    }
    else
    {
        return c;
    }
}

// much smaller encoding for unsigned integers up to 28 bits, but can handle signed
template<class T>
static inline void putuint_(T &p, int n)
{
    if(n < 0 || n >= (1<<21))
    {
        p.put(0x80 | (n & 0x7F));
        p.put(0x80 | ((n >> 7) & 0x7F));
        p.put(0x80 | ((n >> 14) & 0x7F));
        p.put(n >> 21);
    }
    else if(n < (1<<7)) p.put(n);
    else if(n < (1<<14))
    {
        p.put(0x80 | (n & 0x7F));
        p.put(n >> 7);
    }
    else
    {
        p.put(0x80 | (n & 0x7F));
        p.put(0x80 | ((n >> 7) & 0x7F));
        p.put(n >> 14);
    }
}
void putuint(ucharbuf &p, int n)
{
    putuint_(p, n);
}
void putuint(vector<uchar> &p, int n)
{
    putuint_(p, n);
}
void putuint(packetbuf &p, int n) { putuint_(p, n); }

int getuint(ucharbuf &p)
{
    int n = p.get();
    if(n & 0x80)
    {
        n += (p.get() << 7) - 0x80;
        if(n & (1<<14)) n += (p.get() << 14) - (1<<14);
        if(n & (1<<21)) n += (p.get() << 21) - (1<<21);
        if(n & (1<<28)) n |= -1<<28;
    }
    return n;
}

template<class T>
static inline void putfloat_(T &p, float f)
{
    lilswap(&f, 1);
    p.put((uchar *)&f, sizeof(float));
}
void putfloat(ucharbuf &p, float f)
{
    putfloat_(p, f);
}
void putfloat(vector<uchar> &p, float f)
{
    putfloat_(p, f);
}
void putfloat(packetbuf &p, float f) { putfloat_(p, f); }

float getfloat(ucharbuf &p)
{
    float f;
    p.get((uchar *)&f, sizeof(float));
    return lilswap(f);
}

template<class T>
static inline void sendstring_(const char *t, T &p)
{
    while(*t) putint(p, *t++);
    putint(p, 0);
}
void sendstring(const char *t, ucharbuf &p)
{
    sendstring_(t, p);
}
void sendstring(const char *t, vector<uchar> &p)
{
    sendstring_(t, p);
}
void sendstring(const char *t, packetbuf &p) { sendstring_(t, p); }

void getstring(char *text, ucharbuf &p, size_t len)
{
    char *t = text;
    do
    {
        if(t>=&text[len])
        {
            text[len-1] = 0;
            return;
        }
        if(!p.remaining())
        {
            *t = 0;
            return;
        }
        *t = getint(p);
    }
    while(*t++);
}

void filtertext(char *dst, const char *src, bool whitespace, size_t len)
{
    for(int c = uchar(*src); c; c = uchar(*++src))
    {
        if(c == '\f')
        {
            if(!*++src) break;
            continue;
        }
        if(iscubeprint(c) || (iscubespace(c) && whitespace))
        {
            *dst++ = c;
            if(!--len) break;
        }
    }
    *dst = '\0';
}

void ipmask::parse(const char *name)
{
    union
    {
        uchar b[sizeof(uint32_t)];
        uint32_t i;
    } ipconv, maskconv;
    ipconv.i = 0;
    maskconv.i = 0;
    loopi(4)
    {
        char *end = NULL;
        int n = strtol(name, &end, 10);
        if(!end) break;
        if(end > name)
        {
            ipconv.b[i] = n;
            maskconv.b[i] = 0xFF;
        }
        name = end;
        while(int c = *name)
        {
            ++name;
            if(c == '.') break;
            if(c == '/')
            {
                int range = clamp(int(strtol(name, NULL, 10)), 0, 32);
                mask = range ? htonl(0xFFffFFff << (32 - range)) : maskconv.i;
                ip = ipconv.i & mask;
                return;
            }
        }
    }
    ip = ipconv.i;
    mask = maskconv.i;
}

int ipmask::print(char *buf) const
{
    char *start = buf;
    union
    {
        uchar b[sizeof(uint32_t)];
        uint32_t i;
    } ipconv, maskconv;
    ipconv.i = ip;
    maskconv.i = mask;
    int lastdigit = -1;
    loopi(4) if(maskconv.b[i])
    {
        if(lastdigit >= 0) *buf++ = '.';
        loopj(i - lastdigit - 1)
        {
            *buf++ = '*';
            *buf++ = '.';
        }
        buf += sprintf(buf, "%d", ipconv.b[i]);
        lastdigit = i;
    }
    uint32_t bits = ~htonl(mask);
    int range = 32;
    for(; (bits&0xFF) == 0xFF; bits >>= 8) range -= 8;
    for(; bits&1; bits >>= 1) --range;
    if(!bits && range%8) buf += sprintf(buf, "/%d", range);
    return int(buf-start);
}

void sleepms(int ms)
{
    if (ms > 10000 || ms < 0)
    {
        conoutf("WARN: Sleeping for %d ms!", ms);
        assert(ms > 0);
    }
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

#if defined(_MSC_VER) && _MSC_VER < 1900
int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}
#endif

unsigned long rand_xorshf96()            //period 2^96-1
{
    //static unsigned long x = 123456789, y = 362436069, z = 521288629;
    static unsigned long x = std::rand(), y = std::rand(), z = std::rand();
    unsigned long t;

    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
}

static int clockrealbase = 0, clockvirtbase = 0;
static void clockreset() { clockrealbase = SDL_GetTicks(); clockvirtbase = totalmillis; }
VARFP(clockerror, 990000, 1000000, 1010000, clockreset());
VARFP(clockfix, 0, 0, 1, clockreset());

int getclockmillis()
{
    return SDL_GetTicks();
    /*int millis = SDL_GetTicks() - clockrealbase;
    if(clockfix) millis = int(millis*(double(clockerror)/1000000));
    millis += clockvirtbase;
    return std::max(millis, totalmillis);*/
}

// LOGGING
FileIO *logfile = NULL;

void closelogfile()
{
    if(logfile)
    {
		logfile->Close();
		delete logfile;
        logfile = NULL;
    }
}

void setlogfile(const char *fname)
{
    closelogfile();
    if(fname && fname[0])
    {
        fname = findfile(fname, "w");
        if (fname)
		{
			logfile = new FileIO();
			if (!logfile->Open(fname, "w", WriteLocation))
			{
				delete logfile;
				logfile = nullptr;
			}
		}
    }
}

void writelog(FileIO *file, const char *buf)
{
    static uchar ubuf[512];
    size_t len = strlen(buf), carry = 0;
    while(carry < len)
    {
        size_t numu = encodeutf8(ubuf, sizeof(ubuf)-1, &((const uchar *)buf)[carry], len - carry, &carry);
        if(carry >= len) ubuf[numu++] = '\n';
        //fwrite((const int8_t *)ubuf, 1, numu, stdout);
        file->Write((const int8_t *)ubuf, 1, numu);
		file->Flush();
    }
}

#define LOGSTRLEN 512
void writelogv(FileIO *file, const char *fmt, va_list args)
{
    static char buf[LOGSTRLEN];
    vformatstring(buf, fmt, args, sizeof(buf));
    writelog(file, buf);
}

std::string &trimString(std::string &str)
{
   return leftTrimString(rightTrimString(str));
}

std::string &leftTrimString(std::string &str)
{
  auto it2 =  std::find_if( str.begin() , str.end() , [](char ch){ return ch != ' '; } );
  str.erase( str.begin() , it2);
  return str;
}

std::string &rightTrimString(std::string &str)
{
  auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return ch != ' '; } );
  str.erase( it1.base() , str.end() );
  return str;
}

bool strICaseCmd(char const *i_str1, char const *i_str2)
{
  uint32_t size = strlen(i_str1);

  if(size != strlen(i_str2))
    return false;

  for(uint32_t idx = 0; idx < size; ++idx)
        if (toupper(i_str1[idx]) != toupper(i_str2[idx]))
            return false;

  return true;
}

const char *TimeStampToDate(time_t t)
{
    static char ret[100] = {0,};
    static struct tm ts;
    my_localtime_r(&t, &ts);
    strftime(ret, sizeof(ret), "%y/%m/%d %H:%M:%S", &ts);
    return ret;
}

float fixang(float ang)
{
    if(ang >= 360)
        return fmod(ang, 360);
    if (ang < 0)
        return 360 - fmod(-ang, 360);
    return ang;
}

std::string GetFileContents(const char *filename, FileLocation location, int pos)
{
    if (!filename)
        return "";
    static std::stringstream contents;
    FileIO io;
    contents.str("");
    if (!io.Open(filename, "rt", location))
    {
        contents << "HEADER: ERROR: Could not open " << filename; // Fail to open
        return contents.str();
    }
    if (io.Seek(0, RW_SEEK_END) == -1)
    {
        contents << "HEADER: ERROR: Fail to seek to the end of " << filename; // Cannot seek
        return contents.str();
    }
    auto fileSize = io.Offset();
    contents << "__FILEC\n";
    contents << "HEADER: NAME: " << filename << " bytes\n";
    contents << "HEADER: SIZE: " << fileSize << " bytes\n";

    bool seekToBegin = true;
    if (pos < 0)
    {
        if (io.Seek(pos, RW_SEEK_END) == -1)
        {
            contents << "HEADER: ERROR: Fail to seek (pos < 0) " << filename << ". Seeking to the beginning instead ... \n"; // Cannot seek
            seekToBegin = true; // Seek to begin
        }
        else
            seekToBegin = false;
    }
    if (seekToBegin)
    {
        if (io.Seek(0, RW_SEEK_SET) == -1)
        {
            contents << "HEADER: ERROR: Fail to seek to the beginning of " << filename; // Cannot seek
            return contents.str();
        }
    }

    // Header is finished
    contents << "\n\n";
    auto beginContents = contents.str().length();

    int8_t buf[1024];
    while (!io.Eof())
    {
        size_t readRet = io.Read(buf, 1, sizeof(buf));
        if (readRet == 0)
            break;
        contents.write((const char *)buf, readRet);
        if (pos != 0 && (contents.str().length() - beginContents) > (size_t)std::abs(pos))
            break;
    }
    return contents.str();
}

ICOMMAND(getfile, "sii", (char *filename, int *location, int *pos), result(GetFileContents(filename, (FileLocation)*location, *pos).c_str()));
#ifndef STANDALONE
ICOMMAND(getlog, "i", (int *pos), result(GetFileContents(appLogPath(), WriteLocation, *pos).c_str()) );
#endif
