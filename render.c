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
#include "font.h"
#include "texture.h"
#include "time.h"

#pragma region Settings
#define SHADER_PROGRAM_ID_FONT 0
#define SHADER_PROGRAM_ID_CHUNK 1

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

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

    ShaderPrograms[SHADER_PROGRAM_ID_FONT] = Shader_LoadProgram(FontVertexShaderPath, FontFragmentShaderPath, StrEmpty);
    if (ShaderPrograms[SHADER_PROGRAM_ID_FONT] == InvalidId) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shaders.");
        return;
    }

    ShaderPrograms[SHADER_PROGRAM_ID_CHUNK] = Shader_LoadProgram(ChunkVertexShaderPath, ChunkFragmentShaderPath, StrEmpty);
    if (ShaderPrograms[SHADER_PROGRAM_ID_CHUNK] == InvalidId) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shaders.");
        return;
    }

    ChunkTextureId = Texture_LoadDDS(ChunkTexturePath);

    glGenVertexArrays(1, &DefaultVertexArrayId);
    glBindVertexArray(DefaultVertexArrayId);

    // todo load chunks

    glUseProgram(ShaderPrograms[SHADER_PROGRAM_ID_CHUNK]);

    ModelMatrixUniformId = glGetUniformLocation(ShaderPrograms[SHADER_PROGRAM_ID_CHUNK], ModelMatrixUniformName);
    if (ModelMatrixUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", TextureUniformName);
        return;
    }

    TransformMatrixUniformId = glGetUniformLocation(ShaderPrograms[SHADER_PROGRAM_ID_CHUNK], TransformMatrixUniformName);
    if (TransformMatrixUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", TransformMatrixUniformName);
        return;
    }

    CameraUniformId = glGetUniformLocation(ShaderPrograms[SHADER_PROGRAM_ID_CHUNK], CameraPositionUniformName);
    if (CameraUniformId == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load uniform by name: %s", CameraUniformId);
        return;
    }

    TextureUniformId = glGetUniformLocation(ShaderPrograms[SHADER_PROGRAM_ID_CHUNK], TextureUniformName);
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

    TTF_Font* Font = Font_GetFont();
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

void Render_Scene() {
    if (!bInitialized) {
        return;
    }

    /** Use the shader program. */
    glUseProgram(ShaderPrograms[SHADER_PROGRAM_ID_CHUNK]);

    // // 0. copy our vertices array in a buffer for OpenGL to use
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // // 1. then set the vertex attributes pointers
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);  
    // // 2. use our shader program when we want to render an object
    // glUseProgram(shaderProgram);
    // // 3. now draw the object 
    // someOpenGLFunctionThatDrawsOurTriangle();   

    float TriangleVertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    U32 VertexBuffer;
    glGenBuffers(1, &VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof TriangleVertices, TriangleVertices, GL_STATIC_DRAW);

    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof F32, 0);
    // glEnableVertexAttribArray(0);

    /** Send information to the shader program. */
    // glUniform3f(CameraUniformId, CameraPosition[0], CameraPosition[1], CameraPosition[2]);

    /** Bind texture to the texture unit 0. */
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(TextureUniformId, TextureId);

    /** Set texture sampler to texture unit 0. */
    // glUniform1i(TextureUniformId, 0);

    // if (pSDL_Renderer != NULL && TextTexture != NULL) {
    // SDL_RenderCopy(pSDL_Renderer, TextTexture, NULL, &TextRect);
    // }
}

void Render_HUD() {
    if (!bInitialized) {
        return;
    }

    char Buffer[1024] = {0};

    SDL_snprintf(Buffer, 1024, "%.3f", Time_GetFramesPerSecond());

    Render_DrawText(Buffer);
}

void Render_Tick() {
    if (!bInitialized) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(COLOR_BYTE(10), COLOR_BYTE(9), COLOR_BYTE(80), COLOR_BYTE(255));

    Render_Scene();
    Render_HUD();

    SDL_GL_SwapWindow(pSDL_Window);
}
#pragma endregion

#pragma region Private Function Definitions
void Render_Cleanup() {
    SDL_GL_DeleteContext(pSDL_GlContext);

    // todo clean chunks
    glDeleteVertexArrays(1, &DefaultVertexArrayId);
    glBindVertexArray(0);
    glDeleteProgram(ShaderPrograms[SHADER_PROGRAM_ID_FONT]);
    glDeleteProgram(ShaderPrograms[SHADER_PROGRAM_ID_CHUNK]);
    glDeleteTextures(1, &TextureUniformId);
}
#pragma endregion
