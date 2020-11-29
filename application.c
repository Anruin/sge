#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "application.h"

#include <SDL.h>
#include <SDL_log.h>

#include "render.h"
#include "input.h"
#include "test.h"
#include "time.h"

typedef enum {
    EVENT_RUN_GAME_LOOP = 1
} EApplicationEventType;

#pragma region Private Variables
static Bool bShutdownRequested = 0;
#pragma endregion

#pragma region Private Function Declarations
/** Handles SDL initialization at the application startup. Creates an SDL window. */
static I32 Application_InitializeSDL();

/** Advances game step by pushing a frame event to SDL. */
static void Application_AdvanceGameStep();

/** Advances game simulation by a single frame. */
static void Application_Tick();

/** Handles SDL events. */
static void Application_HandleEvent(SDL_Event Event);
#pragma endregion

#pragma region Public Function Definitions
void Application_Initialize() {
    const I32 InitSDLResult = Application_InitializeSDL();
    if (InitSDLResult != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }

    TimeService_Initialize();
    RenderService_Initialize();
    InputService_Initialize();

    Application_AdvanceGameStep();
}

void Application_Run() {
    SDL_Event Event;

    // Main game loop.
    while (!bShutdownRequested && SDL_WaitEvent(&Event)) {
        switch (Event.type) {
        case SDL_USEREVENT:
            Application_HandleEvent(Event);
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
            InputService_HandleEvent(&Event);
            break;

        case SDL_QUIT:
            bShutdownRequested = TRUE;
            break;

        default:
            break;
        }
    }

    Test_Run();
}

void Application_RequestShutdown() {
}

void Application_Shutdown() {
    SDL_Quit();
}

#pragma endregion

#pragma region Private Function Definitions
I32 Application_InitializeSDL() {
    const I32 Error = SDL_Init(SDL_INIT_EVENTS);
    if (Error < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL events.");
        return Error;
    }

    return Error;
}

void Application_AdvanceGameStep() {
    SDL_Event Event;
    SDL_UserEvent UserEvent;

    UserEvent.timestamp = 0;
    UserEvent.windowID = SDL_GetWindowID(RenderService_GetSDLWindow());
    UserEvent.type = SDL_USEREVENT;
    UserEvent.code = EVENT_RUN_GAME_LOOP;
    UserEvent.data1 = 0;
    UserEvent.data2 = 0;

    Event.type = SDL_USEREVENT;
    Event.user = UserEvent;

    SDL_PushEvent(&Event);
}

void Application_Tick() {
    U32 DeltaTime = TimeService_Tick();

    if (DeltaTime <= 0.0f) {
        DeltaTime = 0;
    }

    RenderService_Tick(DeltaTime);

    Application_AdvanceGameStep();
}

void Application_HandleEvent(const SDL_Event Event) {
    switch (Event.user.code) {
    case EVENT_RUN_GAME_LOOP:
        Application_Tick();
        break;

    default:
        break;
    }
}
#pragma endregion

#ifdef __cplusplus
    }
#endif
