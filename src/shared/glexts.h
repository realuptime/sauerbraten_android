#pragma once

// Used only internally!
#ifdef GL_QUADS
#undef GL_QUADS
#endif
#define GL_QUADS 0xFFF

#define glBlendEquation_ glBlendEquation
#define glGenQueries_ glGenQueries
#define glDeleteQueries_ glDeleteQueries
#define glBeginQuery_ glBeginQuery
#define glEndQuery_ glEndQuery
#define glGetQueryObjectiv_ glGetQueryObjectiv
#define glGetQueryObjectuiv_ glGetQueryObjectuiv

#define glActiveTexture_ glActiveTexture
#define glDeleteRenderbuffers_ glDeleteRenderbuffers
#define glGenRenderbuffers_ glGenRenderbuffers
#define glCheckFramebufferStatus_ glCheckFramebufferStatus
#define glGetBufferSubData_ glGetBufferSubData

#define glGetUniformLocation_ glGetUniformLocation
#define glGetUniformBlockIndex_ glGetUniformBlockIndex
#define glGetUniformIndices_ glGetUniformIndices
#define glGetActiveUniformBlockiv_ glGetActiveUniformBlockiv
#define glGetActiveUniformsiv_ glGetActiveUniformsiv
#define glUniformBlockBinding_ glUniformBlockBinding
#define glCreateProgram_ glCreateProgram
#define glAttachShader_ glAttachShader
#define glBindAttribLocation_ glBindAttribLocation
#define glBindFragDataLocation_ glBindFragDataLocation
#define glLinkProgram_ glLinkProgram
#define glUseProgram_ glUseProgram
#define glDeleteProgram_ glDeleteProgram
#define glGetActiveUniform_ glGetActiveUniform
#define glDeleteFramebuffers_  glDeleteFramebuffers
#define glGenFramebuffers_ glGenFramebuffers

#define glUniform1f_ glUniform1f
#define glUniform2f_ glUniform2f
#define glUniform3f_ glUniform3f
#define glUniform4f_ glUniform4f
#define glUniform1i_ glUniform1i
#define glUniform2i_ glUniform2i
#define glUniform3i_ glUniform3i
#define glUniform4i_ glUniform4i
#define glUniform1fv_ glUniform1fv
#define glUniform2fv_ glUniform2fv
#define glUniform3fv_ glUniform3fv
#define glUniform4fv_ glUniform4fv
#define glUniform1iv_ glUniform1iv
#define glUniform2iv_ glUniform2iv
#define glUniform3iv_ glUniform3iv
#define glUniform4iv_ glUniform4iv
#define glUniformMatrix2fv_ glUniformMatrix2fv
#define glUniformMatrix3fv_ glUniformMatrix3fv
#define glUniformMatrix4fv_ glUniformMatrix4fv

#define glBindFramebuffer_ glBindFramebuffer

#define glBindRenderbuffer_ glBindRenderbuffer
#define glRenderbufferStorage_ glRenderbufferStorage
#define glFramebufferRenderbuffer_ glFramebufferRenderbuffer
#define glFramebufferTexture2D_ glFramebufferTexture2D
#define glBlitFramebuffer_ glBlitFramebuffer

#define glVertexAttrib1f_ glVertexAttrib1f
#define glVertexAttrib2f_ glVertexAttrib2f
#define glVertexAttrib3f_ glVertexAttrib3f
#define glVertexAttrib4f_ glVertexAttrib4f

#define glVertexAttrib2fv_ glVertexAttrib2fv
#define glVertexAttrib3fv_ glVertexAttrib3fv
#define glVertexAttrib4fv_ glVertexAttrib4fv

#define glVertexAttrib4Nub_ glVertexAttrib4Nub
#define glVertexAttrib4Nubv_ glVertexAttrib4Nubv

#define glEnableVertexAttribArray_ glEnableVertexAttribArray
#define glDisableVertexAttribArray_ glDisableVertexAttribArray
#define glVertexAttribPointer_ glVertexAttribPointer

#define glBindBuffer_ glBindBuffer
#define glGenBuffers_ glGenBuffers
#define glBufferData_ glBufferData
#define glDrawRangeElements_ glDrawRangeElements
#define glMapBufferRange_ glMapBufferRange
#define glUnmapBuffer_ glUnmapBuffer

#define glBufferSubData_ glBufferSubData
#define glMultiDrawArrays_ glMultiDrawArrays
#define glGenVertexArrays_ glGenVertexArrays
#define glBindVertexArray_ glBindVertexArray
#define glDeleteBuffers_ glDeleteBuffers
#define glDeleteVertexArrays_ glDeleteVertexArrays
#define glGetStringi_ glGetStringi

#define glGetShaderiv_ glGetShaderiv
#define glGetProgramiv_ glGetProgramiv
#define glCreateShader_ glCreateShader
#define glShaderSource_ glShaderSource
#define glCompileShader_ glCompileShader
#define glDeleteShader_ glDeleteShader
#define glCompressedTexImage1D_ glCompressedTexImage1D

#ifndef HAVE_GLES
#define HAVE_GLES
#endif // HAVE_GLES

#ifdef HAVE_GLES

#ifndef GL_TEXTURE_2D_MULTISAMPLE
#define GL_TEXTURE_2D_MULTISAMPLE GL_TEXTURE_2D
#endif // GL_TEXTURE_2D_MULTISAMPLE

#if 1

#ifdef GL_RGB5
#undef GL_RGB5
#endif // GL_RGB5
#define GL_RGB5 GL_RGB

#ifndef GL_RG16F
#define GL_RG16F 0x822F
#endif

#ifndef GL_RG16F
#define GL_RG16F 0xFFF0
#endif // GL_RG16F

#ifndef GL_RG
#define GL_RG 0x8227
#endif // GL_RG

#ifndef GL_R16F
#define GL_R16F 0x822D
#endif

#ifndef GL_RG16F
#define GL_RG16F 0x822F
#endif // GL_RG16F

#ifndef GL_RGB16F
#define GL_RGB16F 0x881B
#endif // GL_RGB16F

#ifndef GL_RGBA16F
#define GL_RGBA16F 0x881A
#endif // GL_RGBA16F

#ifndef GL_RGB32F
#define GL_RGB32F 0x8815
#endif // GL_RGB32F

#ifndef GL_RGBA32F
#define GL_RGBA32F 0x8814
#endif // GL_RGBA32F

#ifndef GL_RGB8
#define GL_RGB8 GL_RGB8_OES
#endif

// BE CAREFULL!!!
#ifdef GL_RGBA8
#undef GL_RGBA8
#endif // GL_RGBA8
#define GL_RGBA8 GL_RGBA
//#ifndef GL_RGBA8
//#define GL_RGBA8 GL_RGBA8_OES
//#endif

#ifndef GL_R11F_G11F_B10F
#define GL_R11F_G11F_B10F 0x8C3A
#endif // GL_R11F_G11F_B10F

#ifndef GL_R32F
#define GL_R32F 0x822E
#endif // GL_R32F

#ifndef GL_RG8
#define GL_RG8 0x8237
#endif // GL_RG8

#ifndef GL_R8
#define GL_R8 0x8229
#endif // GL_R8

#ifndef GL_RED
#define GL_RED 0x1903
#endif // GL_RED

#ifndef GL_GREEN
#define GL_GREEN 0x1904
#endif // GL_GREEN

#ifndef GL_RG32F
#define GL_RG32F 0x8230
#endif // GL_RG32F

#ifndef GL_RGBA8
#define GL_RGBA8 GL_RGB
#endif // GL_RGBA8

#ifndef GL_RGB8I
#define GL_RGB8I 0x8D8F
#endif // GL_RGB8I

#ifndef GL_R8UI
#define GL_R8UI 0x8232
#endif

#ifndef GL_R8I
#define GL_R8I 0x8231
#endif

#ifndef GL_R16I
#define GL_R16I 0x21
#endif

#ifndef GL_R32I
#define GL_R32I 0x22
#endif

#ifndef GL_RG8UI
#define GL_RG8UI 0x23
#endif

#ifndef GL_RG16UI
#define GL_RG16UI 0x24
#endif

#ifndef GL_RG32UI
#define GL_RG32UI 0x25
#endif

#ifndef GL_RG8I
#define GL_RG8I 0x26
#endif

#ifndef GL_RG16I
#define GL_RG16I 0x27
#endif

#ifndef GL_RG32I
#define GL_RG32I 0x28
#endif

#ifndef GL_RG_INTEGER
#define GL_RG_INTEGER 29
#endif

#ifndef GL_RGBA8I
#define GL_RGBA8I 0x15
#endif

#ifndef GL_RGB8UI
#define GL_RGB8UI 0xffff
#endif

#ifndef GL_RGBA8UI
#define GL_RGBA8UI 0x10
#endif

#ifndef GL_RGB16I
#define GL_RGB16I 0xf0f4
#endif

#ifndef GL_RGBA16I
#define GL_RGBA16I 0x11
#endif

#ifndef GL_RGB16UI
#define GL_RGB16UI 0xfff1
#endif

#ifndef GL_R16UI
#define GL_R16UI 0x18
#endif

#ifndef GL_RGBA16UI
#define GL_RGBA16UI 0x12
#endif

#ifndef GL_RGB32I
#define GL_RGB32I 0x00f3
#endif

#ifndef GL_RGBA32I
#define GL_RGBA32I 0x13
#endif

#ifndef GL_RGB_INTEGER
#define GL_RGB_INTEGER 0x00f4
#endif

#ifndef GL_RGBA_INTEGER
#define GL_RGBA_INTEGER 0x16
#endif

#ifndef GL_RED_INTEGER
#define GL_RED_INTEGER 0x8D94
#endif

#ifndef GL_RGB32UI
#define GL_RGB32UI 0xf0f2
#endif

#ifndef GL_R32UI
#define GL_R32UI 0x8236
#endif

#ifndef GL_RGBA32UI
#define GL_RGBA32UI 0x8D70
#endif

#ifndef GL_RGB10
#define GL_RGB10 0x8052
#endif // GL_RGB10

#ifndef GL_R16
#define GL_R16 0x822A
#endif // GL_R16

#ifndef GL_RGB16
#define GL_RGB16 0x8054
#endif // GL_RGB16

#ifndef GL_RG8
#define GL_RG8 0x822B
#endif // GL_RG8
// END Colors
#endif // 0

#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif // GL_DEPTH_COMPONENT24

#ifndef GL_DEPTH_COMPONENT32
#define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
#endif // GL_DEPTH_COMPONENT32

/*#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
#endif

#ifndef GL_DEPTH_STENCIL8
#define GL_DEPTH_STENCIL8 GL_DEPTH_STENCIL
#endif*/

#ifndef GL_UNSIGNED_INT_24_8
#define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES
#endif

#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D GL_TEXTURE_3D_OES
#endif // GL_TEXTURE_3D

#ifdef GL_MAP_WRITE_BIT
#undef GL_MAP_WRITE_BIT
#endif
#define GL_MAP_WRITE_BIT 0

#ifdef GL_MAP_INVALIDATE_RANGE_BIT
#undef GL_MAP_INVALIDATE_RANGE_BIT
#endif
#define GL_MAP_INVALIDATE_RANGE_BIT 0
#ifdef GL_MAP_UNSYNCHRONIZED_BIT
#undef GL_MAP_UNSYNCHRONIZED_BIT
#endif
#define GL_MAP_UNSYNCHRONIZED_BIT 0

#ifndef GL_LUMINANCE8_ALPHA8
#define GL_LUMINANCE8_ALPHA8 GL_LUMINANCE_ALPHA
#endif // GL_LUMINANCE8_ALPHA8

#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8 GL_LUMINANCE
#endif // GL_LUMINANCE8

#ifndef GL_POLYGON_OFFSET_LINE
#define GL_POLYGON_OFFSET_LINE GL_POLYGON_OFFSET_FILL
#endif // GL_POLYGON_OFFSET_LINE

#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
#endif // GL_CLAMP_TO_BORDER

#ifndef GL_BGRA
#define GL_BGRA GL_BGRA_EXT
#endif // GL_BGRA

#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW 0x0503
#endif // GL_STACK_OVERFLOW

#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW 0x0504
#endif // GL_STACK_UNDERFLOW

#ifndef GL_OUT_OF_MEMORY
#define GL_OUT_OF_MEMORY 0x0505
#endif // GL_OUT_OF_MEMORY

#ifndef GL_MAX_DRAW_BUFFERS
#define GL_MAX_DRAW_BUFFERS 0x8824
#endif // GL_MAX_DRAW_BUFFERS

#ifndef GL_MAX_DUAL_SOURCE_DRAW_BUFFERS
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS 0xFFFF
#endif // GL_MAX_DUAL_SOURCE_DRAW_BUFFERS

#ifndef GLdouble
#define GLdouble GLfloat
#endif // GLdouble

#ifndef GL_UNSIGNED_INT_VEC2
#define GL_UNSIGNED_INT_VEC2 0x8DC6
#endif // GL_UNSIGNED_INT_VEC2

#ifndef GL_UNSIGNED_INT_VEC3
#define GL_UNSIGNED_INT_VEC3 0x8DC7
#endif // GL_UNSIGNED_INT_VEC3

#ifndef GL_UNSIGNED_INT_VEC4
#define GL_UNSIGNED_INT_VEC4 0x8DC8
#endif // GL_UNSIGNED_INT_VEC4

#ifndef GL_MIN_PROGRAM_TEXEL_OFFSET
#define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
#endif // GL_MIN_PROGRAM_TEXEL_OFFSET

#ifndef GL_MAX_PROGRAM_TEXEL_OFFSET
#define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
#endif // GL_MAX_PROGRAM_TEXEL_OFFSET

#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R GL_TEXTURE_WRAP_R_OES
#endif // GL_TEXTURE_WRAP_R

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif // GL_READ_FRAMEBUFFER

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif // GL_DRAW_FRAMEBUFFER

#ifndef GL_PIXEL_PACK_BUFFER
#define GL_PIXEL_PACK_BUFFER 0x88EB
#endif // GL_PIXEL_PACK_BUFFER

#ifndef GL_DYNAMIC_COPY
#define GL_DYNAMIC_COPY 0x88EA
#endif // GL_DYNAMIC_COPY

#ifndef GL_TEXTURE_SAMPLES
#define GL_TEXTURE_SAMPLES GL_TEXTURE_SAMPLES_IMG
#endif // GL_TEXTURE_SAMPLES

#ifndef GL_MAX_SAMPLES
#define GL_MAX_SAMPLES GL_MAX_SAMPLES_IMG
#endif // GL_MAX_SAMPLES

#ifndef GL_MAX_COLOR_TEXTURE_SAMPLES
#define GL_MAX_COLOR_TEXTURE_SAMPLES 0x910E
#endif // GL_MAX_COLOR_TEXTURE_SAMPLES

#ifndef GL_MAX_DEPTH_TEXTURE_SAMPLES
#define GL_MAX_DEPTH_TEXTURE_SAMPLES 0x910F
#endif // GL_MAX_DEPTH_TEXTURE_SAMPLES

#ifndef GL_SAMPLE_POSITION
#define GL_SAMPLE_POSITION 0x8E50
#endif // GL_SAMPLE_POSITION

#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR 0x1004
#endif // GL_TEXTURE_BORDER_COLOR

#ifndef GL_TEXTURE_COMPARE_MODE
#define GL_TEXTURE_COMPARE_MODE 0x884C
#endif // GL_TEXTURE_COMPARE_MODE

#ifndef GL_COMPARE_REF_TO_TEXTURE
#define GL_COMPARE_R_TO_TEXTURE_ARB 0x884E
#define GL_COMPARE_REF_TO_TEXTURE GL_COMPARE_R_TO_TEXTURE_ARB
#endif // GL_COMPARE_REF_TO_TEXTURE

#ifndef GL_TEXTURE_COMPARE_FUNC
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#endif // GL_TEXTURE_COMPARE_FUNC

#ifndef GL_SAMPLE_MASK
#define GL_SAMPLE_MASK 0x8E51
#endif // GL_SAMPLE_MASK

#ifndef GL_DEPTH_BOUNDS_TEST_EXT
#define GL_DEPTH_BOUNDS_TEST_EXT 0x8890
#endif // GL_DEPTH_BOUNDS_TEST_EXT

#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif // GL_LINE

#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif // GL_FILL

#ifndef GL_UNPACK_ROW_LENGTH
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#endif // GL_UNPACK_ROW_LENGTH

#ifndef GL_TEXTURE_1D
#define GL_TEXTURE_1D 0x0DE0
#endif

#if 1
// FUNCTIONS

#ifndef glGenVertexArrays
//#define glGenVertexArrays glGenVertexArraysOES
#define glGenVertexArrays(...)
#endif // glGenVertexArrays

#ifdef glMapBufferRange
#undef glMapBufferRange
#endif
#define glMapBufferRange(a, b, c, d) (void *)0

#ifndef glMapBuffer
#define glMapBuffer glMapBufferOES
#endif // glMapBufferOES

#ifndef glUnmapBuffer
#define glUnmapBuffer glUnmapBufferOES
#endif // glUnmapBuffer

#ifndef glBindVertexArray
//#define glBindVertexArray glBindVertexArrayOES
#define glBindVertexArray(...)
#endif // glBindVertexArray

#ifndef glDeleteVertexArrays
//#define glDeleteVertexArrays glDeleteVertexArraysOES
#define glDeleteVertexArrays(...)
#endif // glDeleteVertexArrays

#ifndef glDepthRange
#define glDepthRange glDepthRangef
#endif // glDepthRange

#ifndef glGenQueries
#define glGenQueries(a,b)
#endif // glGenQueries

#ifndef glDeleteQueries
#define glDeleteQueries(a,b)
#endif // glDeleteQueries

#ifndef glTexImage3D
//#define glTexImage3D glTexImage3DOES
#define glTexImage3D(...)
#endif // glTexImage3DOES

#ifndef glBlitFramebuffer
#define glBlitFramebuffer(...)
#endif // glBlitFramebuffer

#ifndef glTexImage2DMultisample
#define glTexImage2DMultisample(...)
#endif // glTexImage2DMultisample

#ifndef glGetTexLevelParameteriv
#define glGetTexLevelParameteriv(...)
#endif // glGetTexLevelParameteriv

#ifndef glRenderbufferStorageMultisample
#define glRenderbufferStorageMultisample(...)
#endif // glRenderbufferStorageMultisample

#ifndef glGetMultisamplefv
#define glGetMultisamplefv(...)
#endif // glGetMultisamplefv

#ifndef _UNIVERSAL_APP_
#ifndef glFramebufferTexture3D
#define glFramebufferTexture3D glFramebufferTexture3DOES
#endif // glFramebufferTexture3D
#else
#define glFramebufferTexture3D(...)
#endif

#ifndef glDrawBuffer
#define glDrawBuffer(...)
#endif // glDrawBuffer

#ifndef glReadBuffer
#define glReadBuffer(...)
#endif // glReadBuffer

#ifndef glDepthBoundsEXT
#define glDepthBoundsEXT(...)
#endif // glDepthBoundsEXT

#ifndef glSampleMaski
#define glSampleMaski(...)
#endif // glSampleMaski

#ifndef glCopyImageSubData
#define glCopyImageSubData(...)
#endif // glCopyImageSubData

#ifndef _UNIVERSAL_APP_
#ifndef glCopyTexSubImage3D
#define glCopyTexSubImage3D glCopyTexSubImage3DOES
#endif // glCopyTexSubImage3D
#else
#define glCopyTexSubImage3D(...)
#endif

#ifndef glPolygonMode
//#define glPolygonMode(...)
#endif // glPolygonMode

#endif // 0

#endif // 1
