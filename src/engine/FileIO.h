#pragma once

#include <string>
#include <cstdio>
#include <assert.h>
#include <stdio.h>

//#define HAVE_STDIO_H

#include <SDL_rwops.h>

class FileIO
{
public:
    bool Open(const std::string &filename, const char *mode, FileLocation location = AssetsLocation, int bufferSize = BufferSize);
    size_t Read(int8_t *buff, size_t size, size_t maxnum);
    size_t ReadBuffered(int8_t *buf, int count);
    size_t Write(const int8_t *buff, size_t size, size_t num);
    bool IsValid() { return _valid; }
    bool Eof() { return _eof; }
    int Flush();
    size_t Printf(const char *fmt, ...);
    FILE *GetFP();
    Sint64 Offset();
    size_t PutC(int c);
    int GetC();
    // fgets equivalent for SDL_rwops
    int GetS(int8_t *buf, int count);
    Sint64 Seek(Sint64 pos, int whence);
    void Close();
    virtual ~FileIO() { Close(); }
#ifndef STANDALONE
	SDL_RWops *RW() { return _file; }
#endif // STANDALONE

    static const int BufferSize = 1024 * 2;
    static bool Init();
    static std::string TransformPath(const std::string &filename, const char *mode, FileLocation location);
	static const char *GetWritePath();
	static void DeleteAllFiles(const char* folderPath, FileLocation fileLocation);

private:
#ifndef STANDALONE
    SDL_RWops *_file = nullptr;
#else
    FILE *_file = nullptr;
#endif
    bool _valid = false;
    bool _eof = false;

    int8_t *_buff = nullptr;
    size_t _buffPos = 0;
    size_t _buffLen = 0;
    size_t _buffSize = 0;
};


