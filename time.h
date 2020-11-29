#pragma once
#include "typedefs.h"

/** Initializes the time service. */
Bool TimeService_Initialize();

/** Updates the time service frame returning the delta time for other services to use. */
U32 TimeService_Tick();

/** Clears and frees memory. */
void TimeService_Shutdown();

/** Returns time passed from the last frame. */
U32 TimeService_GetDeltaTime();
