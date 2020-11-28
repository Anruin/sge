#pragma once
#include "typedefs.h"

/** Initializes the time service. */
void TimeService_Initialize();

/** Updates the time service frame returning the delta time for other services to use. */
U32 TimeService_Tick();

/** Clears and frees memory. */
void TimeService_Shutdown();
