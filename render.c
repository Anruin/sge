#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <cglm/cglm.h>
#include <SDL_log.h>
#include <SDL_video.h>

#include "typedefs.h"
#include "render.h"
#include "file.h"

#pragma region Defaults
static const pStr DefaultVertexShaderPath = "assets/vs.glsl";
static const pStr DefaultFragmentShaderPath = "assets/fs.glsl";
static const pStr DefaultTexturePath = "assets/texture.dds";

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

/** Render service initialization flag. */
Bool bInitialized;

/** Shader program Id. */
U32 ProgramId;
/** Texture Id. */
U32 TextureId;

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
/** Loads and compiles shaders. */
static U32 RenderService_LoadShaders(pStr VertexShaderPath, pStr FragmentShaderPath);

/** Loads DDS texture. */
static U32 RenderService_LoadTexture(pStr TexturePath);

/** Clears memory. */
static void RenderService_Cleanup();

#if _DEBUG
static void GLAPIENTRY Render_OpenGlMessageCallback(const GLenum Source, const GLenum Type, const GLuint Id, GLenum Severity, GLsizei Length, const GLchar* Message,
                                                    const void* UserParam) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, source = %d, id = %d, message = %s\n",
                 (Type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), Type, Severity, Source, Id, Message);
}
#endif

#pragma endregion

#pragma region Public Function Definitions

void RenderService_Initialize() {
    // Initialize SDL video.
    const I32 Error = SDL_Init(SDL_INIT_VIDEO);
    if (Error < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL events.");
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

    ProgramId = RenderService_LoadShaders(DefaultVertexShaderPath, DefaultFragmentShaderPath);
    if (ProgramId == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shaders.");
        return;
    }

    TextureId = RenderService_LoadTexture(DefaultTexturePath);

    glGenVertexArrays(1, &DefaultVertexArrayId);
    glBindVertexArray(DefaultVertexArrayId);

    // todo load chunks

    glUseProgram(ProgramId);

    ModelMatrixUniformId = glGetUniformLocation(ProgramId, ModelMatrixUniformName);
    if (ModelMatrixUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", TextureUniformName);
        return;
    }

    TransformMatrixUniformId = glGetUniformLocation(ProgramId, TransformMatrixUniformName);
    if (TransformMatrixUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", TransformMatrixUniformName);
        return;
    }

    CameraUniformId = glGetUniformLocation(ProgramId, CameraPositionUniformName);
    if (CameraUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", CameraUniformId);
        return;
    }

    TextureUniformId = glGetUniformLocation(ProgramId, TextureUniformName);
    if (TextureUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", TextureUniformName);
        return;
    }

    bInitialized = True;
}

void RenderService_Shutdown() {
    SDL_DestroyWindow(pSDL_Window);
    RenderService_Cleanup();
}

SDL_Window* RenderService_GetSDLWindow() {
    return pSDL_Window;
}

void RenderService_Tick() {
    if (!bInitialized) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(COLOR_BYTE(10), COLOR_BYTE(9), COLOR_BYTE(8), 0.f);

    /** Use the shader program. */
    glUseProgram(ProgramId);

    /** Send information to the shader program. */
    glUniform3f(CameraUniformId, CameraPosition[0], CameraPosition[1], CameraPosition[2]);

    /** Bind texture to the texture unit 0. */
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(TextureUniformId, TextureId);

    /** Set texture sampler to texture unit 0. */
    // glUniform1i(TextureUniformId, 0);

    SDL_GL_SwapWindow(pSDL_Window);
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
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", VertexShaderLog);

            free(VertexShaderLog);
        }

        return False;
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
            const pStr FragmentShaderLog = malloc(FragmentShaderLogLength);
            if (FragmentShaderLog == NULL) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to allocate memory for vertex shader log");
            }

            glGetShaderInfoLog(VertexShaderId, FragmentShaderLogLength, &FragmentShaderLogLength, FragmentShaderLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", FragmentShaderLog);

            free(FragmentShaderLog);
        }

        return 0;
    }

    const U32 _ProgramId = glCreateProgram();
    glAttachShader(_ProgramId, VertexShaderId);
    glAttachShader(_ProgramId, FragmentShaderId);
    glLinkProgram(_ProgramId);

    I32 ProgramLinkStatus = GL_FALSE;
    glGetProgramiv(_ProgramId, GL_LINK_STATUS, &ProgramLinkStatus);

    if (ProgramLinkStatus == GL_FALSE) {
        I32 ProgramLinkLogLength;
        glGetProgramiv(FragmentShaderId, GL_INFO_LOG_LENGTH, &ProgramLinkLogLength);

        if (ProgramLinkLogLength > 0) {
            const pStr ProgramLinkLog = malloc(ProgramLinkLogLength);
            if (ProgramLinkLog == NULL) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to allocate memory for program link log");
            }

            glGetShaderInfoLog(VertexShaderId, ProgramLinkLogLength, &ProgramLinkLogLength, ProgramLinkLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Program link log: %s", ProgramLinkLog);

            free(ProgramLinkLog);
        }

        return 0;
    }

    glDeleteShader(VertexShaderId);
    glDeleteShader(FragmentShaderId);

    return _ProgramId;
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
    glDeleteTextures(1, &TextureUniformId);
}
#pragma endregion
