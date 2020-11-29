#include "typedefs.h"
#include "render.h"

#include <SDL.h>

#include "file.h"

#include <string.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <cglm/cglm.h>
#include <SDL_log.h>
#include <SDL_video.h>

#pragma region Defaults
static const pStr DefaultVertexShaderPath = "assets/vs.glsl";
static const pStr DefaultFragmentShaderPath = "assets/fs.glsl";
static const pStr DefaultTexturePath = "assets/texture.dds";
#pragma endregion

#pragma region Defaults
static const char* DefaultWindowTitle = "Shquarkz by Jod and Anry";
const int DefaultWindowWidth = 1140;
const int DefaultWindowHeight = 855;
#pragma endregion

#pragma region Private Fields
SDL_Window* pSDL_Window;
const pStr* Title;
I32 bMouseCaptured;
SDL_GLContext pSDL_GlContext;

Bool bInitialized;

U32 ProgramId;
U32 TextureId;
U32 Texture;

mat4 Projection;
mat4 View;
U32 TransformMatrixId;
U32 ModelMatrixId;

vec3 CameraPosition;
vec3 CameraForward;
vec3 CameraRight;
vec3 CameraUp;
F32 CameraDistance;
F32 CameraClippingDistance;
U32 CameraId;
U32 DefaultVertexArrayId;
#pragma endregion

#pragma region Private Function Declarations
/** Loads and compiles shaders. */
static U32 RenderService_LoadShaders(pStr VertexShaderPath, pStr FragmentShaderPath);

/** Loads DDS texture. */
static U32 RenderService_LoadTexture(pStr TexturePath);

/** Clears memory. */
static void RenderService_Cleanup();
#pragma endregion

#pragma region Public Function Definitions
void RenderService_Initialize() {
    const I32 Error = SDL_Init(SDL_INIT_VIDEO);
    if (Error < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL events.");
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    const U32 ContextFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    pSDL_Window = SDL_CreateWindow(DefaultWindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DefaultWindowWidth, DefaultWindowHeight, ContextFlags);

    pSDL_GlContext = SDL_GL_CreateContext(pSDL_Window);

    if (pSDL_GlContext == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render service SDL GL context was null @ %s", __FUNCTION__);
        return;
    }

    /** Initialize GLEW. */
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize GLEW");
        return;
    }

    glm_vec3_zero(CameraPosition);
    glm_vec3_copy((vec3){0, 0, -1}, CameraForward);
    glm_vec3_copy((vec3){1, 0, 0}, CameraRight);
    glm_vec3_copy((vec3){0, 1, 0}, CameraUp);

    /** Define perspective camera view matrix. */
    glm_perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.f, Projection);

    /** Define camera look at matrix. */
    glm_lookat(CameraPosition, CameraForward, CameraUp, View);

    /** Fill window with the background color. */
    glClearColor(0.f, 0.f, 0.f, 0.f);

    /** Enable depth test. */
    glEnable(GL_DEPTH_TEST);
    /** Draw the triangle if it is closer to the camera than the previous one. */
    glDepthFunc(GL_LESS);
    /** Cull triangles which normals are not facing the camera. */
    glEnable(GL_CULL_FACE);

    ProgramId = RenderService_LoadShaders(DefaultVertexShaderPath, DefaultFragmentShaderPath);
    Texture = RenderService_LoadTexture(DefaultTexturePath);
    TextureId = glGetUniformLocation(ProgramId, "texture");
    TransformMatrixId = glGetUniformLocation(ProgramId, "transformMatrix");
    ModelMatrixId = glGetUniformLocation(ProgramId, "modelMatrix");

    glGenVertexArrays(1, &DefaultVertexArrayId);
    glBindVertexArray(DefaultVertexArrayId);

    // todo load chunks

    glUseProgram(ProgramId);
    CameraId = glGetUniformLocation(ProgramId, "cameraPosition");

    bInitialized = TRUE;
}

void RenderService_Shutdown() {
    SDL_DestroyWindow(pSDL_Window);
}

SDL_Window* RenderService_GetSDLWindow() {
    return pSDL_Window;
}

void RenderService_Tick(U32 DeltaTime) {
    if (!bInitialized) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /** Use the shader program. */
    glUseProgram(ProgramId);

    /** Send information to the shader program. */
    glUniform3f(CameraId, CameraPosition[0], CameraPosition[1], CameraPosition[2]);
    /** Bind texture to the texture unit 0. */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(TextureId, 0);
    /** Set texture sampler to texture unit 0. */
    glUniform1i(TextureId, 0);

    // todo: update chunks

    const GLenum Error = glGetError();
    if (Error != GL_NO_ERROR) {
        printf("GL error %#X\n", Error);
    }
}
#pragma endregion

#pragma region Private Function Definitions
U32 RenderService_LoadShaders(const pStr VertexShaderPath, const pStr FragmentShaderPath) {
    // Compile vertex shader.
    const GLuint VertexShaderId = glCreateShader(GL_VERTEX_SHADER);

    U64 VertexShaderCodeLength;
    const pStr VertexShaderCode = File_Read(VertexShaderPath, &VertexShaderCodeLength);
    if (VertexShaderCode == NULL) {
        printf("Can't load vertex shader");
        return 0;
    }

    glShaderSource(VertexShaderId, 1, &VertexShaderCode, NULL);
    glCompileShader(VertexShaderId);

    I32 VertexShaderCompileStatus = GL_FALSE;
    glGetShaderiv(VertexShaderId, GL_COMPILE_STATUS, &VertexShaderCompileStatus);

    if (VertexShaderCompileStatus == GL_FALSE) {
        I32 VertexShaderLogLength;
        glGetShaderiv(VertexShaderId, GL_INFO_LOG_LENGTH, &VertexShaderLogLength);

        if (VertexShaderLogLength > 0) {
            const pStr VertexShaderLog = malloc(VertexShaderLogLength);
            if (VertexShaderLog == NULL) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to allocate memory for vertex shader log");
            }

            glGetShaderInfoLog(VertexShaderId, VertexShaderLogLength, &VertexShaderLogLength, VertexShaderLog);
        }
    }

    // Compile fragment shader.
    const GLuint FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    U64 FragmentShaderCodeLength;
    const pStr FragmentShaderCode = File_Read(FragmentShaderPath, &FragmentShaderCodeLength);
    if (FragmentShaderCode == NULL) {
        printf("Can't load fragment shader");
        return 0;
    }

    glShaderSource(FragmentShaderId, 1, &FragmentShaderCode, NULL);
    glCompileShader(FragmentShaderId);

    I32 FragmentShaderCompileStatus = GL_FALSE;
    glGetShaderiv(FragmentShaderId, GL_COMPILE_STATUS, &FragmentShaderCompileStatus);

    if (FragmentShaderCompileStatus == GL_FALSE) {
        I32 FragmentShaderLogLength;
        glGetShaderiv(FragmentShaderId, GL_INFO_LOG_LENGTH, &FragmentShaderLogLength);

        if (FragmentShaderLogLength > 0) {
            const pStr VertexShaderLog = malloc(FragmentShaderLogLength);
            if (VertexShaderLog == NULL) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to allocate memory for vertex shader log");
            }

            glGetShaderInfoLog(VertexShaderId, FragmentShaderLogLength, &FragmentShaderLogLength, VertexShaderLog);
        }
    }

    return 0;
}

U32 RenderService_LoadTexture(const pStr TexturePath) {
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

void RenderService_Cleanup() {
    SDL_GL_DeleteContext(pSDL_GlContext);

    // todo clean chunks
    glDeleteVertexArrays(1, &DefaultVertexArrayId);
    glBindVertexArray(0);
    glDeleteProgram(ProgramId);
    glDeleteTextures(1, &TextureId);
}
#pragma endregion
