#pragma once
#include <SDL_ttf.h>
#include <SDL_video.h>
#include "typedefs.h"

/** Initializes the render service. Loads and compiles shaders, loads textures, initializes camera and matrices, creates vertex arrays. */
void Render_Initialize();

/** Updates the render service frame. */
void Render_Tick();

/** Clears the initialized buffers, shaders, textures and frees memory. */
void Render_Shutdown();

/** SDL window getter. */
SDL_Window* Render_GetSDLWindow();

void Render_DrawText(pStr Text);

SDL_Texture* Render_RenderTextToTexture(pStr Text, SDL_Color Color, I32 X, I32 Y, TTF_Font* Font);