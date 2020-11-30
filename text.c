#include "text.h"
#include <SDL_ttf.h>

TTF_Font* Font = NULL;

const pStr DefaultFont = "assets/fonts/ttf/DejaVuLGCSansMono.ttf";
const I32 DefaultFontSize = 16;

#pragma region Private Function Declarations
TTF_Font* TextService_LoadFont(pStr FontPath, I32 FontSize);
void TextService_UnloadFont();
#pragma endregion

#pragma region Public Function Definitions
Bool TextService_Initialize() {
    TTF_Init();

    TextService_LoadFont(DefaultFont, DefaultFontSize);

    return Font != NULL;
}

TTF_Font* TextService_GetFont() {
    return Font;
}

void TextService_Shutdown() {
    TextService_UnloadFont();

    TTF_Quit();
}
#pragma endregion

#pragma region Private Function Definitions
TTF_Font* TextService_LoadFont(const pStr FontPath, const I32 FontSize) {
    Font = TTF_OpenFont(FontPath, FontSize);
    if (Font == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL failed to open font %s, %d: %s", FontPath, FontSize, SDL_GetError());
    }

    return Font;
}

void TextService_UnloadFont() {
    TTF_CloseFont(Font);
}
#pragma endregion
