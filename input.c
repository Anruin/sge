#include "typedefs.h"
#include "input.h"

#include <SDL.h>
#include <SDL_log.h>

#include "application.h"

#pragma region Defaults
#pragma endregion

/** The input service instance. */
static FInputService* InputServiceInstance = NULL;

#pragma region Private Function Declarations
#pragma endregion

#pragma region Public Function Definitions
void DefaultInputHandler(const SDL_Event* Event) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%x: %d", Event->key.keysym.sym, Event->type == SDL_KEYDOWN ? 1 : 0);
}

void DefaultApplicationExitHandler(const SDL_Event* Event) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Requested application exit.");
    Application_Get()->bShutdownRequested = 1;
}

FInputService* InputService_Get() {
    if (InputServiceInstance != NULL) {
        return InputServiceInstance;
    }

    InputServiceInstance = calloc(1, sizeof *InputServiceInstance);
    if (InputServiceInstance == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for the input service");
        return NULL;
    }

    InputService_Initialize(InputServiceInstance);

    return InputServiceInstance;
}

void InputService_Initialize(FInputService* pInputService) {
    if (pInputService == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "pInputService was nullptr");
    }

    pInputService->InputBindings[0] = (FInputBinding){SDLK_ESCAPE, DefaultApplicationExitHandler};
    pInputService->InputBindings[1] = (FInputBinding){SDLK_w, DefaultInputHandler};
    pInputService->InputBindings[2] = (FInputBinding){SDLK_a, DefaultInputHandler};
    pInputService->InputBindings[3] = (FInputBinding){SDLK_s, DefaultInputHandler};
    pInputService->InputBindings[4] = (FInputBinding){SDLK_d, DefaultInputHandler};
}

void InputService_Shutdown(FInputService* pInputService) {
    if (pInputService != NULL) {
        free(pInputService);
    }
    pInputService = NULL;
}

void InputService_Tick(FInputService* pInputService, F32 DeltaTime) {
}

void InputService_KeyEvent(const SDL_Event* Event) {

    const FInputService* pInputService = InputService_Get();

    if (pInputService == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "pInputService was nullptr");
        return;
    }

    // Ignore events not related to keyboard.
    if (Event->type < SDL_KEYDOWN || Event->type > SDL_MULTIGESTURE) {
        return;
    }

    const int BindingsCount = sizeof pInputService->InputBindings / sizeof pInputService->InputBindings[0];
    for (int Index = 0; Index < BindingsCount; Index++) {
        const FInputBinding InputBinding = pInputService->InputBindings[Index];

        if (InputBinding.Keycode != SDLK_UNKNOWN && InputBinding.Keycode == Event->key.keysym.sym) {
            InputBinding.Handler(Event);
        }
    }
}

#pragma endregion

#pragma region Private Function Definitions

#pragma endregion
