﻿#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <GL/glew.h>
#include "application.h"

#include "graphics.h"
#include "test.h"

#define EVENT_RUN_GAME_LOOP 1

#pragma region Defaults
static const char* DefaultWindowTitle = "Shquarkz by Jod and Anry";
const int DefaultWindowWidth = 1140;
const int DefaultWindowHeight = 855;
#pragma endregion

/** The application instance. */
static FApplication* ApplicationInstance = NULL;

#pragma region Private Function Definitions
/** Handles SDL initialization at the application startup. Creates an SDL window. */
static I32 Application_InitializeSDL(FApplication* pApplication, uint32_t WindowWidth, uint32_t WindowHeight, uint32_t WindowFlags);

/** Advances game step by pushing a frame event to SDL. */
static void Application_AdvanceGameStep(FApplication* pApplication);

/** Advances game simulation by a single frame. */
static void Application_Tick(FApplication* pApplication);

/** Handles SDL events. */
static void Application_HandleSDLEvent(FApplication* pApplication, SDL_Event Event);
#pragma endregion

#pragma region Public Function Definitions
FApplication* Application_Get() {
    if (ApplicationInstance != NULL) {
        return ApplicationInstance;
    }

    ApplicationInstance = calloc(1, sizeof *ApplicationInstance);
    if (ApplicationInstance == NULL) {
        perror("Failed to allocate memory for the application instance");
        return NULL;
    }

    Application_Initialize(ApplicationInstance);

    return ApplicationInstance;
}

void Application_Initialize(FApplication* pApplication) {
    if (pApplication == NULL) {
        perror("pApplication is nullptr");
        return;
    }

    const U32 ContextFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    const I32 InitSDLResult = Application_InitializeSDL(pApplication, DefaultWindowWidth, DefaultWindowHeight, ContextFlags);
    if (InitSDLResult != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }

    pApplication->LastTime = SDL_GetTicks();

    /** Initialize subsystems. */

    /** Initialize the render service. */
    RenderService_Get();
    // Physics
    // Input

    Application_AdvanceGameStep(pApplication);
}

    void Application_Run(FApplication* pApplication) {
    SDL_Event Event;

    Test_Run();

    // Main game loop.
    while (!pApplication->bShutdownRequested && SDL_WaitEvent(&Event)) {
        switch (Event.type) {
        case SDL_USEREVENT:
            Application_HandleSDLEvent(pApplication, Event);
            break;

        case SDL_KEYDOWN:
            // todo: handle shutdown
            break;

        case SDL_QUIT:
            pApplication->bShutdownRequested = 1;
            break;

        default:
            break;
        }
    }
}

    void Application_Shutdown(FApplication* pApplication) {
    SDL_GL_DeleteContext(pApplication->pSDL_GlContext);
    SDL_DestroyWindow(pApplication->pSDL_Window);
    SDL_Quit();
}

#pragma endregion

#pragma region Private Function Definitions
#pragma endregion


I32 Application_InitializeSDL(FApplication* pApplication, const uint32_t WindowWidth, const uint32_t WindowHeight, const uint32_t WindowFlags) {
    const I32 Error = SDL_Init(SDL_INIT_EVERYTHING);
    if (Error < 0) {
        printf_s("Unable to initialize SDL.");
        return Error;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    pApplication->pSDL_Window = SDL_CreateWindow(DefaultWindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowWidth, WindowHeight, WindowFlags);
    pApplication->pSDL_GlContext = SDL_GL_CreateContext(pApplication->pSDL_Window);

    pApplication->Window = (FApplicationWindow){DefaultWindowTitle, DefaultWindowWidth, DefaultWindowHeight};

    return Error;
}

void Application_AdvanceGameStep(FApplication* pApplication) {
    SDL_Event Event;
    SDL_UserEvent UserEvent;

    UserEvent.timestamp = 0;
    UserEvent.windowID = SDL_GetWindowID(pApplication->pSDL_Window);
    UserEvent.type = SDL_USEREVENT;
    UserEvent.code = EVENT_RUN_GAME_LOOP;
    UserEvent.data1 = 0;
    UserEvent.data2 = 0;

    Event.type = SDL_USEREVENT;
    Event.user = UserEvent;

    SDL_PushEvent(&Event);
}

void Application_Tick(FApplication* pApplication) {
    const uint32_t CurrentTime = SDL_GetTicks();

    float DeltaTime = CurrentTime - pApplication->LastTime / 1000.0f;
    pApplication->LastTime = CurrentTime;

    if (DeltaTime <= 0.0f) {
        DeltaTime = 0.01f;
    } else if (DeltaTime > 0.1f) {
        DeltaTime = 0.1f;
    }

    if (SDL_GetMouseFocus() == pApplication->pSDL_Window) {
        SDL_WarpMouseInWindow(pApplication->pSDL_Window, DefaultWindowWidth / 2, DefaultWindowHeight / 2);
    }

    Application_AdvanceGameStep(pApplication);
}

void Application_HandleSDLEvent(FApplication* pApplication, const SDL_Event Event) {
    switch (Event.user.code) {
    case EVENT_RUN_GAME_LOOP:
        Application_Tick(pApplication);
        break;

    default:
        break;
    }
}

#ifdef __cplusplus
    }
#endif
