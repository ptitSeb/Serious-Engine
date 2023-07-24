/* Copyright (c) 2002-2012 Croteam Ltd. All rights reserved. */

/* rcg10142001 Implemented. */


// !!! FIXME: rcg10142001 This should really be using CTStrings...


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/param.h>
#include <unistd.h>

#include <Engine/Engine.h>
#include <Engine/Base/FileSystem.h>

#include "SDL.h"

ENGINE_API CFileSystem *_pFileSystem = NULL;

// Stolen from SDL2/src/filesystem/unix/SDL_sysfilesystem.c
static char * readSymLink(const char *path)
{
    char *retval = NULL;
    ssize_t len = 64;
    ssize_t rc = -1;

    while (1)
    {
        char *ptr = (char *) SDL_realloc(retval, (size_t) len);
        if (ptr == NULL) {
            SDL_OutOfMemory();
            break;
        }

        retval = ptr;

        rc = readlink(path, retval, len);
        if (rc == -1) {
            break;  /* not a symlink, i/o error, etc. */
        } else if (rc < len) {
            retval[rc] = '\0';  /* readlink doesn't null-terminate. */

            /* try to shrink buffer... */
            ptr = (char *) SDL_realloc(retval, strlen(retval) + 1);
            if (ptr != NULL)
                retval = ptr;  /* oh well if it failed. */

            return retval;  /* we're good to go. */
        }

        len *= 2;  /* grow buffer, try again. */
    }

    SDL_free(retval);
    return NULL;
}

class CUnixFileSystem : public CFileSystem
{
public:
    CUnixFileSystem(const char *argv0, const char *gamename);
    virtual ~CUnixFileSystem(void);
    virtual void GetExecutablePath(char *buf, ULONG bufSize);
    virtual void GetUserDirectory(char *buf, ULONG bufSize);
    virtual CDynamicArray<CTString> *FindFiles(const char *dir,
                                               const char *wildcard);
protected:
    char *exePath;
    char *userDir;
};

CFileSystem *CFileSystem::GetInstance(const char *argv0, const char *gamename)
{
    return(new CUnixFileSystem(argv0, gamename));
}


const char *CFileSystem::GetDirSeparator(void)
{
    return("/");
}


BOOL CFileSystem::IsDummyFile(const char *fname)
{
    return( (strcmp(fname, ".") == 0) || (strcmp(fname, "..") == 0) );
}


BOOL CFileSystem::Exists(const char *fname)
{
    struct stat s;
    if (stat(fname, &s) == -1)
        return(FALSE);

    return(TRUE);
}


BOOL CFileSystem::IsDirectory(const char *fname)
{
    struct stat s;
    if (stat(fname, &s) == -1)
        return(FALSE);

    return(S_ISDIR(s.st_mode) ? TRUE : FALSE);
}


CUnixFileSystem::CUnixFileSystem(const char *argv0, const char *gamename)
{
    exePath = readSymLink("/proc/self/exe");
    userDir = SDL_GetPrefPath("Serious Engine", gamename);
}


CUnixFileSystem::~CUnixFileSystem(void)
{
    SDL_free(userDir);
    SDL_free(exePath);
}


void CUnixFileSystem::GetExecutablePath(char *buf, ULONG bufSize)
{
    SDL_snprintf(buf, bufSize, "%s", exePath);
}


void CUnixFileSystem::GetUserDirectory(char *buf, ULONG bufSize)
{
    SDL_snprintf(buf, bufSize, "%s", userDir);
}


CDynamicArray<CTString> *CUnixFileSystem::FindFiles(const char *dir,
                                                   const char *wildcard)
{
    CDynamicArray<CTString> *retval = new CDynamicArray<CTString>;
    DIR *d = opendir(dir);

    if (d != NULL)
    {
        struct dirent *dent;
        while ((dent = readdir(d)) != NULL)
        {
            CTString str(dent->d_name);
            if (str.Matches(wildcard))
                *retval->New() = str;
        }
        closedir(d);
    }

    return(retval);
}

// end of UnixFileSystem.cpp ...


