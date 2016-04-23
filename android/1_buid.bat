@echo off

set NDK_MODULE_PATH=%CD%\..\3rdparty\SDL\
set NDK_LIBS_OUT=%CD%\app\src\main\jniLibs\

echo NDK_MODULE_PATH is set to %NDK_MODULE_PATH%
echo NDK_LIBS_OUT is set to %NDK_LIBS_OUT%

rem call copy_assets.bat

rem ndk-build V=1 NDK_DEBUG=1
rem ndk-build -j8 NDK_DEBUG=1
rem ndk-build -j16 V=0 NDK_DEBUG=0
rem ndk-build V=1 -j8 NDK_DEBUG=1

rem ndk-build
ndk-build -j8
