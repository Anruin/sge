#pragma once
#include <SDL_ttf.h>
#include <SDL_video.h>
#include "typedefs.h"

/** Initializes the render service. Loads and compiles shaders, loads textures, initializes camera and matrices, creates vertex arrays. */
void RenderService_Initialize();

/** Updates the render service frame. */
void RenderService_Tick();

/** Clears the initialized buffers, shaders, textures and frees memory. */
void RenderService_Shutdown();

/** Returns the SDL window. */
SDL_Window* RenderService_GetSDLWindow();

void RenderService_DrawText(pStr Text);
SDL_Texture* RenderService_RenderTextToTexture(pStr Text, SDL_Color Color, I32 X, I32 Y, TTF_Font* Font);