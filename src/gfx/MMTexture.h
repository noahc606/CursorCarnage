#pragma once
#include <nch/cpp-utils/color.h>
#include <SDL2/SDL.h>
#include <string>

/// @brief Mipmapped Texture.
class MMTexture {
public:
    MMTexture(SDL_Renderer* rend, SDL_PixelFormat* pxFmt, std::string resourcePath);
    ~MMTexture();

    void drawMipmap(SDL_Renderer* rend, int winWidth, int winHeight);
    void renderCopy(SDL_Renderer* rend, SDL_Rect* src, SDL_Rect* dst);
    void setBlendMode(SDL_BlendMode bm);
    void setColorMod(const nch::Color& col);
    void setAlphaMod(uint8_t val);

    int getScaleIndexBySrcDst(SDL_Rect& src, SDL_Rect& dst);
    SDL_Rect getAreaByScaleIndex(int scaleIdx);
    int getMaxScaleIndex();

    int getWidth();
    int getHeight();
private:
    int width = -1, height = -1;
    int mmWidth = -1;
    SDL_Texture* mipmap = nullptr;
};