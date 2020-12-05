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
#include "font.h"
#include "time.h"

typedef enum {
    EVENT_RUN_GAME_LOOP = 1
} EApplicationEventType;

#pragma region Private Variables
static Bool bInitialized = False;
static Bool bShutdownRequested = False;
#pragma endregion

#pragma region Private Function Declarations
/** Advances game step by pushing a frame event to SDL. */
static void Application_AdvanceGameStep();

/** Advances game simulation by a single frame. */
static void Application_Tick();

/** Handles SDL events. */
static void Application_HandleEvent(SDL_Event Event);
#pragma endregion

#pragma region Public Function Definitions
Bool Application_Initialize() {
    if (bInitialized) {
        return bInitialized;
    }

    if (SDL_Init(0) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s.", SDL_GetError());
        return False;
    }

    if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0) {
        SDL_Log("Unable to initialize SDL event subsystem: %s", SDL_GetError());
        return False;
    }

    Time_Initialize();
    Font_Initialize();
    Render_Initialize();
    Input_Initialize();

    bInitialized = True;

    Application_AdvanceGameStep();

    return bInitialized;
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
            Input_HandleEvent(&Event);
            break;

        case SDL_QUIT:
            bShutdownRequested = True;
            break;

        default:
            break;
        }
    }

    Test_Run();
}

void Application_RequestShutdown() {
    bShutdownRequested = True;
}

void Application_Shutdown() {
    Render_Shutdown();
    Time_Shutdown();

    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    SDL_Quit();
}

#pragma endregion

#pragma region Private Function Definitions
void Application_AdvanceGameStep() {
    SDL_Event Event;
    SDL_UserEvent UserEvent;

    UserEvent.timestamp = 0;
    UserEvent.windowID = SDL_GetWindowID(Render_GetSDLWindow());
    UserEvent.type = SDL_USEREVENT;
    UserEvent.code = EVENT_RUN_GAME_LOOP;
    UserEvent.data1 = 0;
    UserEvent.data2 = 0;

    Event.type = SDL_USEREVENT;
    Event.user = UserEvent;

    SDL_PushEvent(&Event);
}

void Application_Tick() {
    U32 DeltaTime = Time_Tick();

    if (DeltaTime <= 0.0f) {
        DeltaTime = 0;
    }

    char Buffer[1024] = {0};

    sprintf(Buffer, "%.3f", Time_GetFramesPerSecond());

    Render_Tick();

    Render_DrawText(Buffer);

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
