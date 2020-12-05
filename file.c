#include "file.h"

#include <SDL_rwops.h>

#include "typedefs.h"
#include <stdio.h>
#include <stdlib.h>

#if OS_WINDOWS
#include <windows.h>
#include <sys/stat.h>
#endif

I64 File_GetSize(const pStr FileName) {
    struct stat Stat;

    if (stat(FileName, &Stat) != 0) {
        fprintf(stderr, "File: %s\n", FileName);
        perror("Can't get file size");
        return -1;
    }

    return Stat.st_size;
}

pStr File_ReadText(const pStr Path, I64* OutLength) {
    SDL_RWops* Context = SDL_RWFromFile(Path, "rb");
    if (Context == NULL) {
        return NULL;
    }

    const I64 Size = SDL_RWsize(Context);
    const pStr Result = (pStr)malloc(Size + 1);

    I64 TotalBytesRead = 0, BytesRead = 1;
    pStr Buffer = Result;
    while (TotalBytesRead < Size && BytesRead != 0) {
        BytesRead = SDL_RWread(Context, Buffer, 1, (Size - TotalBytesRead));
        TotalBytesRead += BytesRead;
        Buffer += BytesRead;
    }

    SDL_RWclose(Context);
    if (TotalBytesRead != Size) {
        free(Result);
        return NULL;
    }

    Result[TotalBytesRead] = '\0';

    if (OutLength != NULL) {
        *OutLength = TotalBytesRead;
    }

    return Result;
}
