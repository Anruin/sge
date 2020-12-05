#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <cglm/cglm.h>
#include <SDL_log.h>
#include <SDL_video.h>
#include <SDL_ttf.h>

#include "typedefs.h"
#include "render.h"
#include "shader.h"
#include "text.h"

#pragma region Settings
#define FONT_PROGRAM_ID 0
#define CHUNK_PROGRAM_ID 1

static const pStr FontVertexShaderPath = "assets/shaders/font_vs.glsl";
static const pStr FontFragmentShaderPath = "assets/shaders/font_fs.glsl";

static const pStr ChunkVertexShaderPath = "assets/shaders/vs.glsl";
static const pStr ChunkFragmentShaderPath = "assets/shaders/fs.glsl";
static const pStr ChunkTexturePath = "assets/textures/texture.dds";

static const pStr TextureUniformName = "texture";
static const pStr TransformMatrixUniformName = "MVP";
static const pStr ModelMatrixUniformName = "M";
static const pStr CameraPositionUniformName = "eyePosition";

static const char* DefaultWindowTitle = "Shquarkz Game Engine";
const int DefaultWindowWidth = 1140;
const int DefaultWindowHeight = 855;

/** Four-character codes. */
#define FOURCC_DXT1 0x31545844
#define FOURCC_DXT3 0x33545844
#define FOURCC_DXT5 0x35545844

#pragma endregion

#pragma region Private Fields
/** Window title. */
const pStr* WindowTitle;

/** Main window. */
SDL_Window* pSDL_Window;
/** Open GL context. */
SDL_GLContext pSDL_GlContext;
/** SDL Renderer for simple drawing. */
SDL_Renderer* pSDL_Renderer;

SDL_Rect TextRect = {0, 0, 0, 0};
pStr TextLine = "";
SDL_Texture* TextTexture = NULL;

/** Render service initialization flag. */
Bool bInitialized;

/** Shader programs. */
U32 ShaderPrograms[8] = {0};

/** Texture Id. */
U32 ChunkTextureId;

/** Projection matrix. */
mat4 Projection;
/** View matrix. */
mat4 View;

/** Texture uniform. */
U32 TextureUniformId;
/** Transform matrix uniform. */
U32 TransformMatrixUniformId;
/** Model matrix uniform. */
U32 ModelMatrixUniformId;

/** Camera position. */
vec3 CameraPosition;

/** Camera forward vector. */
vec3 CameraForward;
/** Camera right vector. */
vec3 CameraRight;
/** Camera up vector. */
vec3 CameraUp;

/** Camera frustum near clipping plane. */
F32 CameraNearClipDistance;
/** Camera frustum far clipping plane. */
F32 CameraFarClipDistance;
/** Camera uniform. */
U32 CameraUniformId;

/** Vertex array. */
U32 DefaultVertexArrayId;
#pragma endregion

#pragma region Private Function Declarations
/** Loads DDS texture. */
static U32 Render_LoadTexture(pStr TexturePath);

/** Clears memory. */
static void Render_Cleanup();

#if _DEBUG
static void GLAPIENTRY Render_OpenGlMessageCallback(const GLenum Source, const GLenum Type, const GLuint Id, GLenum Severity, GLsizei Length, const GLchar* Message,
                                                    const void* UserParam) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, source = %d, id = %d, message = %s\n",
                 (Type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), Type, Severity, Source, Id, Message);
}
#endif

#pragma endregion

#pragma region Public Function Definitions

void Render_Initialize() {
    // Initialize SDL video.
    if (SDL_Init(0) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL.");
        return;
    }

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL video.");
        return;
    }

    // Configure OpenGL.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create SDL window.
    const U32 ContextFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    pSDL_Window = SDL_CreateWindow(DefaultWindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DefaultWindowWidth, DefaultWindowHeight, ContextFlags);

    pSDL_Renderer = SDL_CreateRenderer(pSDL_Window, -1, SDL_RENDERER_ACCELERATED);
    if (pSDL_Renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL renderer.");
        return;
    }

    // Create OpenGL context.
    pSDL_GlContext = SDL_GL_CreateContext(pSDL_Window);
    if (pSDL_GlContext == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render service SDL GL context was null @ %s", __FUNCTION__);
        return;
    }

    // Initialize GLEW.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize GLEW");
        return;
    }

#if _DEBUG
    // Enable debug output and bind the callback.
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(Render_OpenGlMessageCallback, 0);
#endif

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", glGetString(GL_VERSION));

    // Setup camera.
    glm_vec3_zero(CameraPosition);
    glm_vec3_copy((vec3){0, 0, -1}, CameraForward);
    glm_vec3_copy((vec3){1, 0, 0}, CameraRight);
    glm_vec3_copy((vec3){0, 1, 0}, CameraUp);

    // Define perspective camera view matrix.
    glm_perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.f, Projection);

    // Define camera look at matrix.
    glm_lookat(CameraPosition, CameraForward, CameraUp, View);

    // Fill window with the background color.
    glClearColor(0.f, 0.f, 0.f, 0.f);

    // Enable depth test.
    glEnable(GL_DEPTH_TEST);
    // Draw the triangle if it is closer to the camera than the previous one.
    glDepthFunc(GL_LESS);
    // Cull triangles which normals are not facing the camera.
    glEnable(GL_CULL_FACE);

    ShaderPrograms[FONT_PROGRAM_ID] = Shader_LoadProgram(FontVertexShaderPath, FontFragmentShaderPath, StrEmpty);
    if (ShaderPrograms[FONT_PROGRAM_ID] == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shaders.");
        return;
    }

    ShaderPrograms[CHUNK_PROGRAM_ID] = Shader_LoadProgram(ChunkVertexShaderPath, ChunkFragmentShaderPath, StrEmpty);
    if (ShaderPrograms[CHUNK_PROGRAM_ID] == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shaders.");
        return;
    }

    ChunkTextureId = Render_LoadTexture(ChunkTexturePath);

    glGenVertexArrays(1, &DefaultVertexArrayId);
    glBindVertexArray(DefaultVertexArrayId);

    // todo load chunks

    glUseProgram(ShaderPrograms[CHUNK_PROGRAM_ID]);

    ModelMatrixUniformId = glGetUniformLocation(ShaderPrograms[CHUNK_PROGRAM_ID], ModelMatrixUniformName);
    if (ModelMatrixUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", TextureUniformName);
        return;
    }

    TransformMatrixUniformId = glGetUniformLocation(ShaderPrograms[CHUNK_PROGRAM_ID], TransformMatrixUniformName);
    if (TransformMatrixUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", TransformMatrixUniformName);
        return;
    }

    CameraUniformId = glGetUniformLocation(ShaderPrograms[CHUNK_PROGRAM_ID], CameraPositionUniformName);
    if (CameraUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", CameraUniformId);
        return;
    }

    TextureUniformId = glGetUniformLocation(ShaderPrograms[CHUNK_PROGRAM_ID], TextureUniformName);
    if (TextureUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", TextureUniformName);
        return;
    }

    bInitialized = True;
}

void Render_Shutdown() {
    SDL_DestroyWindow(pSDL_Window);
    Render_Cleanup();

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

SDL_Window* Render_GetSDLWindow() {
    return pSDL_Window;
}

void Render_DrawText(const pStr Text) {
    TextLine = Text;

    const SDL_Color TextColor = {1, 0, 0, 1};

    TTF_Font* Font = TextService_GetFont();
    if (Font == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get a font!");
        return;
    }

    SDL_Texture* pTexture = Render_RenderTextToTexture(Text, TextColor, 0, 0, Font);

    U32 VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(F32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(F32), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    F32 OutWidth, OutHeight;
    SDL_GL_BindTexture(pTexture, &OutWidth, &OutHeight);
}

SDL_Texture* Render_RenderTextToTexture(const pStr Text, const SDL_Color Color, const I32 X, const I32 Y, TTF_Font* Font) {
    // Render the text.
    SDL_Surface* pSurface = TTF_RenderText_Blended(Font, Text, Color);
    if (pSurface == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_RenderText");
        return NULL;
    }

    // Create a texture.
    SDL_Texture* pTexture = SDL_CreateTextureFromSurface(pSDL_Renderer, pSurface);
    if (pTexture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateTexture");
    }

    SDL_FreeSurface(pSurface);

    return pTexture;
}

void Render_Tick() {
    if (!bInitialized) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(COLOR_BYTE(10), COLOR_BYTE(9), COLOR_BYTE(8), 0.f);

    /** Use the shader program. */
    glUseProgram(ShaderPrograms[CHUNK_PROGRAM_ID]);

    /** Send information to the shader program. */
    glUniform3f(CameraUniformId, CameraPosition[0], CameraPosition[1], CameraPosition[2]);

    /** Bind texture to the texture unit 0. */
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(TextureUniformId, TextureId);

    /** Set texture sampler to texture unit 0. */
    // glUniform1i(TextureUniformId, 0);

    // if (pSDL_Renderer != NULL && TextTexture != NULL) {
    // SDL_RenderCopy(pSDL_Renderer, TextTexture, NULL, &TextRect);
    // }

    SDL_GL_SwapWindow(pSDL_Window);
}
#pragma endregion

#pragma region Private Function Definitions
U32 Render_LoadTexture(const pStr TexturePath) {
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

void Render_Cleanup() {
    SDL_GL_DeleteContext(pSDL_GlContext);

    // todo clean chunks
    glDeleteVertexArrays(1, &DefaultVertexArrayId);
    glBindVertexArray(0);
    glDeleteProgram(ShaderPrograms[CHUNK_PROGRAM_ID]);
    glDeleteTextures(1, &TextureUniformId);
}
#pragma endregion
