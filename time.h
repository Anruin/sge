#pragma once
#include "typedefs.h"

/** Initializes the time service. */
Bool Time_Initialize();

/** Updates the time service frame returning the delta time for other services to use. */
U32 Time_Tick();

/** Clears and frees memory. */
void Time_Shutdown();

/** Returns time passed from the last frame. */
U32 Time_GetDeltaTime();

/** Returns current average FPS rate. */
F32 Time_GetFramesPerSecond();
