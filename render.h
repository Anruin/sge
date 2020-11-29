#pragma once
#include <SDL_video.h>
#include "typedefs.h"

/** Initializes the render service. Loads and compiles shaders, loads textures, initializes camera and matrices, creates vertex arrays. */
void RenderService_Initialize();

/** Updates the render service frame. */
void RenderService_Tick(U32 DeltaTime);

/** Clears the initialized buffers, shaders, textures and frees memory. */
void RenderService_Shutdown();

/** Returns the SDL window. */
SDL_Window* RenderService_GetSDLWindow();
