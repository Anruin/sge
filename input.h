#pragma once
#include <SDL_events.h>
#include <SDL_keycode.h>
#include "typedefs.h"

/** Input axis types. */
typedef enum {
    INPUT_AXIS_UNKNOWN = 0,
    INPUT_AXIS_MOUSE_X,
    INPUT_AXIS_MOUSE_Y,
} EInputAxis;

/** Input action handler. */
typedef void (*InputActionHandler)(const SDL_Event* Event);

/** Input axis handler. */
typedef void (*InputAxisHandler)(const SDL_Event* Event, EInputAxis Axis, I32 Value);

/** Input action binding. */
typedef struct {
    SDL_Keycode Keycode;
    InputActionHandler Handler;
} FInputActionBinding;

/** Input axis binding. */
typedef struct {
    EInputAxis Axis;
    InputAxisHandler Handler;
} FInputAxisBinding;

/** Initializes the input service. Configures key and mouse bindings. */
void Input_Initialize();

/** Processes key events received from the SDL. */
void Input_HandleEvent(const SDL_Event* Event);
