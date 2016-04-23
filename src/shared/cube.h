#ifndef __CUBE_H__
#define __CUBE_H__

#define _FILE_OFFSET_BITS 64

#ifdef __GNUC__
#define gamma __gamma
#endif

#include <algorithm>
using std::max;
using std::min;
using std::swap;

#define D DIM_D
#define C DIM_C
#define R DIM_R

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#ifdef __GNUC__
#undef gamma
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #ifdef _WIN32_WINNT
  #undef _WIN32_WINNT
  #endif
  #define _WIN32_WINNT 0x0500
  #include "windows.h"
  #ifndef _WINDOWS
    #define _WINDOWS
  #endif
  #ifndef __GNUC__
    #include <eh.h>
    #include <dbghelp.h>
    #include <intrin.h>
  #endif
  #define ZLIB_DLL
#endif

//#ifndef STANDALONE
#if 1
  #ifdef __APPLE__
    #include "SDL2/SDL.h"
    #include "SDL2/SDL_opengl.h"
    #define main SDL_main
  #else
    #include <SDL.h>


    #ifdef __ANDROID__
        #include <SDL.h>
        #define GL_GLEXT_PROTOTYPES
        #include <GLES2/gl2ext.h>
    #elif defined(_UNIVERSAL_APP_)
        #include <memory>
        #include <wrl.h>
        // Enable function definitions in the GL headers below
        #define GL_GLEXT_PROTOTYPES
        // OpenGL ES includes
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
        // EGL includes
        #include <EGL/egl.h>
        #include <EGL/eglext.h>
        #include <EGL/eglplatform.h>
        // ANGLE include for Windows Store
        #include <angle_windowsstore.h>
        #include <SDL.h>
    #else // Desktop
        #include <SDL.h>
#ifndef STANDALONE
        #include <GL/glew.h>
#else
	typedef uint32_t GLuint;
#endif
    #endif // __ANDROID__

  #endif // APPLE
#endif // STANDALONE

#include <enet/enet.h>

#include <zlib.h>

#include "tools.h"
#include "geom.h"
#include "ents.h"
#include "command.h"

#ifndef STANDALONE
#include "glexts.h"
#include "glemu.h"
#endif

#include "iengine.h"
#include "igame.h"

#endif

