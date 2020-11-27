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

void DefaultAxisHandler(const SDL_Event* Event, const EInputAxis Axis, const F32 Value) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%x: %.3f", Axis, Value);
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

    SDL_SetRelativeMouseMode(SDL_TRUE);

    pInputService->InputKeyBindings[0] = (FInputKeyBinding){SDLK_ESCAPE, DefaultApplicationExitHandler};
    pInputService->InputKeyBindings[1] = (FInputKeyBinding){SDLK_w, DefaultInputHandler};
    pInputService->InputKeyBindings[2] = (FInputKeyBinding){SDLK_a, DefaultInputHandler};
    pInputService->InputKeyBindings[3] = (FInputKeyBinding){SDLK_s, DefaultInputHandler};
    pInputService->InputKeyBindings[4] = (FInputKeyBinding){SDLK_d, DefaultInputHandler};

    pInputService->InputAxisBindings[0] = (FInputAxisBinding){SQZ_AXIS_MOUSE_X, DefaultAxisHandler};
    pInputService->InputAxisBindings[1] = (FInputAxisBinding){SQZ_AXIS_MOUSE_Y, DefaultAxisHandler};
}

void InputService_Shutdown(FInputService* pInputService) {
    if (pInputService != NULL) {
        free(pInputService);
    }
    pInputService = NULL;
}

void InputService_Tick(FInputService* pInputService, F32 DeltaTime) {
    FApplication* pApplication = Application_Get();

    if (pApplication == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "pApplication is null @ %s", __FUNCTION__);
    }
}

void InputService_HandleEvent(FInputService* pInputService, const SDL_Event* Event) {
    if (pInputService == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "pInputService was nullptr @ %s", __FUNCTION__);
        return;
    }
    
    if (Event->type == SDL_KEYDOWN || Event->type == SDL_KEYUP || Event->type == SDL_MOUSEBUTTONDOWN || Event->type == SDL_MOUSEBUTTONUP) {
        const int BindingsCount = sizeof pInputService->InputKeyBindings / sizeof pInputService->InputKeyBindings[0];
        for (int Index = 0; Index < BindingsCount; Index++) {
            const FInputKeyBinding InputKeyBinding = pInputService->InputKeyBindings[Index];

            if (InputKeyBinding.Keycode != SDLK_UNKNOWN && InputKeyBinding.Keycode == Event->key.keysym.sym) {
                InputKeyBinding.Handler(Event);
            }
        }
    } else if (Event->type == SDL_MOUSEMOTION) {
        const int BindingsCount = sizeof pInputService->InputAxisBindings / sizeof pInputService->InputAxisBindings[0];

        for (int Index = 0; Index < BindingsCount; Index++) {
            const FInputAxisBinding InputAxisBinding = pInputService->InputAxisBindings[Index];

            if (InputAxisBinding.Axis == SQZ_AXIS_UNKNOWN) {
                continue;
            }

            if (InputAxisBinding.Axis == SQZ_AXIS_MOUSE_X) {
                InputAxisBinding.Handler(Event, SQZ_AXIS_MOUSE_X, Event->motion.xrel);
            } else if (InputAxisBinding.Axis == SQZ_AXIS_MOUSE_Y) {
                InputAxisBinding.Handler(Event, SQZ_AXIS_MOUSE_Y, Event->motion.yrel);
            }
        }
    }

    // Ignore events not related to keyboard or mouse.
    if (Event->type < SDL_KEYDOWN || Event->type > SDL_MOUSEWHEEL) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Non related event received: %d @ %s", Event->type, __FUNCTION__);
        return;
    }
}

#pragma endregion

#pragma region Private Function Definitions

#pragma endregion
