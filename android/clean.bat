@echo off

set NDK_MODULE_PATH=%CD%\..\3rdparty\SDL\
set NDK_LIBS_OUT=%CD%\app\src\main\jniLibs\

ndk-build clean
