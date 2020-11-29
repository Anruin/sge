#include "typedefs.h"
#include "input.h"

#include <SDL.h>
#include <SDL_log.h>

#include "application.h"

#pragma region Defaults
#pragma endregion

/** Input action bindings. */
static FInputActionBinding InputActionBindings[8];

/** Input axis bindings. */
static FInputAxisBinding InputAxisBindings[8];

#pragma region Private Function Declarations
static void DefaultInputHandler(const SDL_Event* Event);
static void DefaultAxisHandler(const SDL_Event* Event, EInputAxis Axis, I32 Value);
static void DefaultApplicationExitHandler(const SDL_Event* Event);
#pragma endregion

#pragma region Public Function Definitions
void InputService_Initialize() {
    SDL_SetRelativeMouseMode(SDL_TRUE);

    InputActionBindings[0] = (FInputActionBinding){SDLK_ESCAPE, DefaultApplicationExitHandler};
    InputActionBindings[1] = (FInputActionBinding){SDLK_w, DefaultInputHandler};
    InputActionBindings[2] = (FInputActionBinding){SDLK_a, DefaultInputHandler};
    InputActionBindings[3] = (FInputActionBinding){SDLK_s, DefaultInputHandler};
    InputActionBindings[4] = (FInputActionBinding){SDLK_d, DefaultInputHandler};

    InputAxisBindings[0] = (FInputAxisBinding){INPUT_AXIS_MOUSE_X, DefaultAxisHandler};
    InputAxisBindings[1] = (FInputAxisBinding){INPUT_AXIS_MOUSE_Y, DefaultAxisHandler};
}

void InputService_HandleEvent(const SDL_Event* Event) {
    if (Event->type == SDL_KEYDOWN || Event->type == SDL_KEYUP || Event->type == SDL_MOUSEBUTTONDOWN || Event->type == SDL_MOUSEBUTTONUP) {
        const int BindingsCount = sizeof InputActionBindings / sizeof InputActionBindings[0];
        for (int Index = 0; Index < BindingsCount; Index++) {
            const FInputActionBinding InputKeyBinding = InputActionBindings[Index];

            if (InputKeyBinding.Keycode != SDLK_UNKNOWN && InputKeyBinding.Keycode == Event->key.keysym.sym) {
                InputKeyBinding.Handler(Event);
            }
        }
    } else if (Event->type == SDL_MOUSEMOTION) {
        const int BindingsCount = sizeof InputAxisBindings / sizeof InputAxisBindings[0];

        for (int Index = 0; Index < BindingsCount; Index++) {
            const FInputAxisBinding InputAxisBinding = InputAxisBindings[Index];

            if (InputAxisBinding.Axis == INPUT_AXIS_UNKNOWN) {
                continue;
            }

            if (InputAxisBinding.Axis == INPUT_AXIS_MOUSE_X) {
                InputAxisBinding.Handler(Event, INPUT_AXIS_MOUSE_X, Event->motion.xrel);
            } else if (InputAxisBinding.Axis == INPUT_AXIS_MOUSE_Y) {
                InputAxisBinding.Handler(Event, INPUT_AXIS_MOUSE_Y, Event->motion.yrel);
            }
        }
    }
}
#pragma endregion

#pragma region Private Function Definitions
void DefaultInputHandler(const SDL_Event* Event) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Input Action: %x: %d", Event->key.keysym.sym, Event->type == SDL_KEYDOWN ? 1 : 0);
}

void DefaultAxisHandler(const SDL_Event* Event, const EInputAxis Axis, const I32 Value) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Input Axis: %x: %d", Axis, Value);
}

void DefaultApplicationExitHandler(const SDL_Event* Event) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Requested application exit.");
    Application_RequestShutdown();
}
#pragma endregion
