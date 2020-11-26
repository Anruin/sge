#pragma once
#include <SDL_events.h>
#include <SDL_keycode.h>
#include "typedefs.h"

typedef void (*InputHandler)(const SDL_Event* Event);

typedef struct {
    SDL_Keycode Keycode;
    InputHandler Handler;
} FInputBinding;

static void DefaultInputHandler(const SDL_Event* Event);

static void DefaultApplicationExitHandler(const SDL_Event* Event);

typedef struct {
    FInputBinding InputBindings[8];
} FInputService;

/** Returns the input service singleton. */
FInputService* InputService_Get();

/** Initializes the input service. Loads and compiles shaders, loads textures, initializes camera and matrices, creates vertex arrays. */
void InputService_Initialize(FInputService* pInputService);

/** Updates the input service frame. */
void InputService_Tick(FInputService* pInputService, F32 DeltaTime);

/** Processes key events received from the SDL. */
void InputService_KeyEvent(const SDL_Event* Event);

/** Clears the initialized buffers, shaders, textures and frees memory. */
void InputService_Shutdown(FInputService* pInputService);
