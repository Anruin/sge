#include "font.h"
#include <SDL_ttf.h>

TTF_Font* Font = NULL;

const pStr DefaultFont = "assets/fonts/ttf/DejaVuLGCSansMono.ttf";
const I32 DefaultFontSize = 16;

#pragma region Private Function Declarations
TTF_Font* Font_LoadFont(pStr FontPath, I32 FontSize);
void Font_UnloadFont();
#pragma endregion

#pragma region Public Function Definitions
Bool Font_Initialize() {
    TTF_Init();

    Font_LoadFont(DefaultFont, DefaultFontSize);

    return Font != NULL;
}

TTF_Font* Font_GetFont() {
    return Font;
}

void Font_Shutdown() {
    Font_UnloadFont();

    TTF_Quit();
}
#pragma endregion

#pragma region Private Function Definitions
TTF_Font* Font_LoadFont(const pStr FontPath, const I32 FontSize) {
    Font = TTF_OpenFont(FontPath, FontSize);
    if (Font == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL failed to open font %s, %d: %s", FontPath, FontSize, SDL_GetError());
    }

    return Font;
}

void Font_UnloadFont() {
    TTF_CloseFont(Font);
}
#pragma endregion
