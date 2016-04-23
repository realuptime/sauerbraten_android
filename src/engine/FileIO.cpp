#include "engine.h"
#include "FileIO.h"

#if defined(_UNIVERSAL_APP_) || defined(__ANDROID__)
#define LOGF(...)
#else
#define LOGF logoutf
#endif // _UNIVERSAL_APP_

#ifdef _UNIVERSAL_APP_
//#undef logoutf
//#define logoutf conoutf
#endif // _UNIVERSAL_APP_

SVAR(assets_folder, "");
static char write_folder[1024] = {0,};

const char *FileIO::GetWritePath()
{
	return write_folder;
}

std::string FileIO::TransformPath(const std::string &filename, const char *mode, FileLocation location)
{
    std::string name;
	if (location == WriteLocation || (mode && strchr(mode, 'w'))) // Write
	{
		name = GetWritePath();
		name += '/';
		name += filename;
	}
	else if (location == AssetsLocation && assets_folder && assets_folder[0]) // Read
	{
		name = assets_folder;
		name += '/';
		name += filename;
	}
	else // Root
		name = filename;
    return name;
}

bool FileIO::Open(const std::string &filename, const char *mode, FileLocation location, int bufferSize)
{
    std::string name = TransformPath(filename, mode, location);

	if (location == WriteLocation || (mode && strchr(mode, 'w'))) // Write
	{
		name = GetWritePath();
		name += '/';
		name += filename;
	}
	else if (location == AssetsLocation && assets_folder && assets_folder[0]) // Read
	{
		name = assets_folder;
		name += '/';
		name += filename;
	}
	else // Root
		name = filename;

#ifndef STANDALONE
    if (!(_file = SDL_RWFromFile(name.c_str(), mode)))
#else
    if (!(_file = fopen(name.c_str(), mode)))
#endif
    {
        logoutf("%s: FileIO : Failed to open in '%s' mode: %s", name.c_str(), mode, SDL_GetError());
        return false;
    }
    _buffSize = bufferSize;
    LOGF("%s : FileIO: Successfully opened in '%s' mode", name.c_str(), mode);
    _valid = true;
    return true;
}

size_t FileIO::Read(int8_t *buff, size_t size, size_t maxnum)
{
    if (!_valid || !_file) return 0;
#ifndef STANDALONE
    size_t ret = _file->read(_file, buff, size, maxnum);
#else
    size_t ret = fread(buff, size, maxnum, _file);
#endif
    //conoutf("Read:%d. toRead:%d", ret, maxnum);
    _eof = ret == 0;
    return ret;
}

size_t FileIO::Write(const int8_t *buff, size_t size, size_t num)
{
	if (!_valid || !_file) return 0;
#ifndef STANDALONE
    return _file->write(_file, buff, size, num);
#else
    return fwrite(buff, size, num, _file);
#endif
}

size_t FileIO::PutC(int c)
{
    if (!_valid || !_file) return 0;
    char cc[1] { (char)c };
#ifndef STANDALONE
    return _file->write(_file, cc, 1, 1);
#else
    return fwrite(cc, 1, 1, _file);
#endif // STANDALONE
}

Sint64 FileIO::Seek(Sint64 pos, int whence)
{
    if (!_valid || !_file) return 0;
#ifndef STANDALONE
    return _file->seek(_file, pos, whence);
#else
    return fseek(_file, pos, whence);
#endif
}

int FileIO::GetC()
{
    if (!_valid || !_file) return 0;
    char c;
#ifndef STANDALONE
    return SDL_RWread(_file, &c, 1, 1) == 1 ? c : EOF;
#else
    return fread(&c, 1, 1, _file) == 1 ? c : EOF;
#endif
}

size_t FileIO::ReadBuffered(int8_t *buf, int count)
{
    if (!_buff)
    {
        _buff = new int8_t[_buffSize];
        assert(_buff);
        if (!_buff) return -1;
    }

    if (_buffPos == _buffLen)
    {
        size_t read = Read(_buff, 1, _buffSize);
        if (read <= 0)
        {
            _eof = true;
            return read;
        }
        _buffLen = read;
        _buffPos = 0;
    }
    count = std::min((size_t)count, _buffLen - _buffPos);
    if (count)
    {
        memcpy(buf, _buff + _buffPos, count);
        _buffPos += count;
        assert(_buffPos <= _buffLen);
    }
    return count;
}

// fgets equivalent for SDL_rwops
int FileIO::GetS(int8_t *buf, int count)
{
    if (count == 0) return 0;
    if (!_valid || !_file) return 0;

    buf[count - 1] = '\0';
    int i = 0;
    for (i = 0; i < count - 1; i++)
    {
        int readRet = ReadBuffered(buf + i, 1);
        if (readRet != 1)
        {
            if (readRet != 0) logoutf("WARNING: FileIO::GetS: SDL_RWread(1) returned %d", readRet);
            if (i == 0) return 0;
            break;
        }
        if (buf[i] == '\n') break;
    }
    buf[i] = '\0';

    return i;
}

void FileIO::Close()
{
    _valid = false;
    _eof = false;
    if (_file)
    {
#ifndef STANDALONE
        _file->close(_file);
#else
        fclose(_file);
#endif
        _file = nullptr;
    }
    if (_buff)
    {
        delete [] _buff;
        _buff = nullptr;
    }
}

size_t FileIO::Printf(const char *fmt, ...)
{
    va_list v;
    va_start(v, fmt);
    char buf[4096];

    int result = my_vsnprintf(buf, sizeof(buf), fmt, v);

    if (result > 0)
        result = Write((int8_t *)buf, 1, result);
    else
        result = 0;
    va_end(v);
    return result;
}

FILE *FileIO::GetFP()
{
    //return _file->stdio.fp;
    return NULL;
}

Sint64 FileIO::Offset()
{
    if (!_valid || !_file) return -1;
#ifndef STANDALONE
    return SDL_RWtell(_file);
#else
    return ftell(_file);
#endif
}

bool FileIO::Init()
{
    write_folder[0] = 0;
#ifdef __WINRT__
	int result = WideCharToMultiByte(0, 0, (LPCWSTR)Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data(), -1, (LPSTR)write_folder, sizeof(write_folder), NULL, NULL);
	if (result <= 0)
    {
        logoutf("ERROR: WideCharToMultiByte to ANSI failed in GetWritePath");
	}
	else
	{
#ifndef STANDALONE
		SDL_Log("FileIO: WINRT: Write folder path: '%s'", write_folder);
#endif
		return true;
	}
#endif // __WINRT__
    if (assets_folder && assets_folder[0])
    {
        strncpy(write_folder, assets_folder, sizeof(write_folder));
    }
    else
    {
        write_folder[0] = '.';
        write_folder[1] = 0;
    }
#ifndef STANDALONE
    SDL_Log("FileIO: Write folder path: '%s'", write_folder);
#endif
    return true;
}

int FileIO::Flush()
{
    if (!_valid || !_file) return 0;
#ifdef STANDALONE
    return fflush(_file);
#else
    return 0; // Not supported
#endif
}

#if !defined(__ANDROID__) && !defined(__WINRT__) && !defined(STANDALONE)
#include <windows.h>
void FileIO::DeleteAllFiles(const char *folderPath, FileLocation fileLocation)
{
    char fileFound[1024] = {0,};
    WIN32_FIND_DATA info;
    HANDLE hp;
    std::string trans = TransformPath(folderPath, "w", fileLocation);
    sprintf(fileFound, "%s\\*.*", trans.c_str());
    hp = FindFirstFile(fileFound, &info);
    do
    {
        sprintf(fileFound,"%s\\%s", trans.c_str(), info.cFileName);
        logoutf("Deleting file %s ...", fileFound);
        DeleteFile(fileFound);
    } while(FindNextFile(hp, &info));
    FindClose(hp);
}
#else
void FileIO::DeleteAllFiles(const char* folderPath, FileLocation fileLocation)
{}
#endif // __ANDROID__

