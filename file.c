#include "file.h"
#include "typedefs.h"
#include <stdio.h>
#include <stdlib.h>

#if OS_WINDOWS
#include <io.h>
#include "windows.h"
#endif

long File_GetSize(const pStr FileName) {
    struct stat Stat;

    if (stat(FileName, &Stat) != 0) {
        fprintf(stderr, "File: %s\n", FileName);
        perror("Can't get file size");
        return -1;
    }

    return Stat.st_size;
}

pStr File_Read(const pStr FileName, U64* OutLength) {
#if OS_WINDOWS

    const __int64 FileSize = File_GetSize(FileName);

    if (FileSize == -1) {
        fprintf(stderr, "File: %s\n", FileName);
        perror("Can't get file size");
        return NULL;
    }

    const pStr Buffer = malloc(FileSize);
    if (Buffer == NULL) {
        perror("Can't allocate buffer");
        return NULL;
    }

    FILE* pFile = fopen(FileName, "rb");
    fseek(pFile, 0, SEEK_SET); /* same as rewind(f); */

    fread(Buffer, 1, FileSize, pFile);
    fclose(pFile);

    Buffer[FileSize] = '\0';

    if (OutLength != NULL) {
        *OutLength = FileSize;
    }

    return Buffer;

#else
    perror("Not implemented");
    return NULL;
#endif
}
