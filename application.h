﻿#pragma once

#include "SDL.h"

typedef struct {
    const char* Title;
    uint32_t Width;
    uint32_t Height;
} FApplicationWindow;

/** The application struct. */
typedef struct {
    FApplicationWindow Window;
    SDL_Window* pSDL_Window;
    SDL_GLContext pSDL_GlContext;
    uint32_t LastTime;
    int bShutdownRequested;
    // std::vector<void*> World;
} FApplication;

/** Handles the application initialization and startup. */
void Application_Initialize(FApplication* pApplication);

/** Handles the application run and main loop. */
void Application_Run(FApplication* pApplication);

/** Handles the application cleanup and shutdown. */
void Application_Shutdown(FApplication* pApplication);