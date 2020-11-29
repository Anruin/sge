#pragma once
#include "typedefs.h"

/** Handles the application initialization and startup. */
Bool Application_Initialize();

/** Handles the application run and main loop. */
void Application_Run();

/** Called to stop the application. */
void Application_RequestShutdown();

/** Handles the application cleanup and shutdown. */
void Application_Shutdown();
