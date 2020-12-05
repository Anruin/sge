#include "texture.h"

#include <stdio.h>
#include <GL/glew.h>

/** Four-character codes. */
#define FOURCC_DXT1 0x31545844
#define FOURCC_DXT3 0x33545844
#define FOURCC_DXT5 0x35545844

U32 Texture_LoadDDS(const pStr TexturePath) {
    U8 Header[124];

    /** Try to open the file. */
    FILE* pFile = fopen(TexturePath, "rb");
    if (pFile == NULL) {
        printf("Unable to open file %s\n", TexturePath);
        return 0;
    }

    /** Check the file type. */
    char FileType[4];

    fread(FileType, 1, 4, pFile);
    if (strncmp(FileType, "DDS ", 4) != 0) {
        printf("Bad file type %s, DDS expected", FileType);
        fclose(pFile);
        return 0;
    }

    /** Get the surface header. */
    fread(&Header, 124, 1, pFile);

    U32 Height = *(U32*)&Header[8];
    U32 Width = *(U32*)&Header[12];
    const U32 LinearSize = *(U32*)&Header[16];
    const U32 MipMapCount = *(U32*)&Header[24];
    const U32 FourCC = *(U32*)&Header[80];

    const U32 BufferSize = MipMapCount > 1 ? LinearSize * 2 : LinearSize;
    U8* Buffer = (U8*)malloc(BufferSize * sizeof(U8));
    fread(Buffer, 1, BufferSize, pFile);

    fclose(pFile);

    U32 Format;
    switch (FourCC) {
    case FOURCC_DXT1:
        Format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        Format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        Format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        printf("Bad file code %#x, expected DXT1, DXT3 or DXT5", FourCC);
        free(Buffer);
        return 0;
    }

    // Create an OpenGL texture.
    GLuint TextureId;
    glGenTextures(1, &TextureId);

    // Bind the new texture so that all next texture handling functions will modify this one. 
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const U32 BlockSize = (Format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    U32 Offset = 0;

    // Load mipmaps.
    for (U32 Level = 0; Level < MipMapCount && (Width || Height); ++Level) {
        const U32 Size = (Width + 3) / 4 * ((Height + 3) / 4) * BlockSize;

        Offset += Size;
        Width /= 2;
        Height /= 2;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    free(Buffer);

    return TextureId;
}
