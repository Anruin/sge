#pragma once
#include <SDL_events.h>
#include <SDL_keycode.h>
#include "typedefs.h"

typedef enum {
    SQZ_AXIS_UNKNOWN = 0,
    SQZ_AXIS_MOUSE_X,
    SQZ_AXIS_MOUSE_Y,
} EInputAxis;

typedef void (*InputHandler)(const SDL_Event* Event);
typedef void (*InputAxisHandler)(const SDL_Event* Event, EInputAxis Axis, F32 Value);

typedef struct {
    SDL_Keycode Keycode;
    InputHandler Handler;
} FInputKeyBinding;

typedef struct {
    EInputAxis Axis;
    InputAxisHandler Handler;
} FInputAxisBinding;

static void DefaultInputHandler(const SDL_KeyboardEvent* Event);
static void DefaultAxisHandler(const SDL_KeyboardEvent* Event, EInputAxis Axis, F32 Value);
static void DefaultApplicationExitHandler(const SDL_KeyboardEvent* Event);

typedef struct {
    FInputKeyBinding InputKeyBindings[8];
    FInputAxisBinding InputAxisBindings[8];
    I32 MouseCaptured;
} FInputService;

/** Returns the input service singleton. */
FInputService* InputService_Get();

/** Initializes the input service. Loads and compiles shaders, loads textures, initializes camera and matrices, creates vertex arrays. */
void InputService_Initialize(FInputService* pInputService);

/** Updates the input service frame. */
void InputService_Tick(FInputService* pInputService, F32 DeltaTime);

/** Processes key events received from the SDL. */
void InputService_HandleEvent(FInputService* pInputService, const SDL_Event* Event);

/** Clears the initialized buffers, shaders, textures and frees memory. */
void InputService_Shutdown(FInputService* pInputService);
