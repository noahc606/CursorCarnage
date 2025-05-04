#include "MMTexture.h"
#include <SDL2/SDL_image.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/rect.h>
#include <nch/sdl-utils/texture-utils.h>

using namespace nch;

MMTexture::MMTexture(SDL_Renderer* rend, SDL_PixelFormat* pxFmt, std::string resourcePath)
{
    SDL_Surface* surf, * surf2;
    SDL_Texture* rawTex;
    /* Create raw texture to build mipmap */ {
    surf = IMG_Load(resourcePath.c_str());
    surf2 = SDL_ConvertSurface(surf, pxFmt, 0);
    rawTex = SDL_CreateTextureFromSurface(rend, surf2);
    TexUtils::clearTexture(rend, rawTex);
    /**/ }

    /* Create mipmap */ {
    //Find dimensions of mipmap
    width = surf->w; height = surf->h;
    mmWidth = width+width/2;
    //Create empty mipmap texture
    mipmap = SDL_CreateTexture(rend, pxFmt->format, SDL_TEXTUREACCESS_TARGET, mmWidth, height);
    TexUtils::clearTexture(rend, mipmap);

    //Build final mipmap texture
    SDL_Texture* prevTgt = SDL_GetRenderTarget(rend); {
        SDL_SetRenderTarget(rend, mipmap);
        for(int i = 0; i<=getMaxScaleIndex(); i++) {
            SDL_Rect dst = getAreaByScaleIndex(i);
            
            SDL_RenderCopy(rend, rawTex, NULL, &dst);
        }
    } SDL_SetRenderTarget(rend, prevTgt);
    /**/ }

    /* Cleanup */ {
    SDL_FreeSurface(surf);
    SDL_FreeSurface(surf2);
    SDL_DestroyTexture(rawTex);
    /**/ }

    setBlendMode(SDL_BLENDMODE_BLEND);
}

MMTexture::~MMTexture()
{
    SDL_DestroyTexture(mipmap);
}

void MMTexture::drawMipmap(SDL_Renderer* rend, int winWidth, int winHeight)
{
    Color c;
    c.setFromHSV(std::floor((Timer::getTicks()/10)%360), 50, 50);

    int scale = 1;

    Rect dst(winWidth-mmWidth*scale, winHeight-height*scale, mmWidth*scale, height*scale);
    SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, 255);
    SDL_RenderFillRect(rend, &dst.r);

    dst.r.x += scale;   dst.r.w -= (2*scale);
    dst.r.y += scale;   dst.r.h -= (2*scale);
    SDL_SetTextureBlendMode(mipmap, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(rend, mipmap, NULL, &dst.r);
}

void MMTexture::renderCopy(SDL_Renderer* rend, SDL_Rect* src, SDL_Rect* dst)
{
    SDL_Rect s;
    if(src==NULL) { s = Rect(0, 0, width, height).r; }
    else { s = *src; }
    
    int sidx = 0;
    if(dst!=NULL) { sidx = getScaleIndexBySrcDst(s, *dst); }
    SDL_Rect ms = getAreaByScaleIndex(sidx);
    int div = std::pow(2, sidx);
    
    ms.x += s.x/div;
    ms.y += s.y/div;
    ms.w = s.w/div;
    ms.h = s.h/div;

    SDL_RenderCopy(rend, mipmap, &ms, dst);
}

void MMTexture::setColorMod(const Color& col) {
    SDL_SetTextureColorMod(mipmap, col.r, col.g, col.b);
}

void MMTexture::setAlphaMod(uint8_t val) {
    SDL_SetTextureAlphaMod(mipmap, val);
}

void MMTexture::setBlendMode(SDL_BlendMode bm) {
    SDL_SetTextureBlendMode(mipmap, bm);
}


int MMTexture::getScaleIndexBySrcDst(SDL_Rect& src, SDL_Rect& dst)
{
    int sidx = -1;
    for(int i = 0; i<=getMaxScaleIndex(); i++) {
        int div = std::pow(2, i+1);
        if(dst.w>src.w/div || dst.h>src.h/div) return i;
    }

    return 0;
}

SDL_Rect MMTexture::getAreaByScaleIndex(int scaleIdx)
{
    int w = width, h = height;

    Rect res;
    switch(scaleIdx) {
        case 0: res = Rect(0, 0, w, h); break;
        default: {
            int fact = std::pow(2, scaleIdx-1);
            res = Rect(w, (fact-1)*h/fact, w/(fact*2), h/(fact*2));
        } break;
    }

    return res.r;
}

int MMTexture::getMaxScaleIndex()
{
    int mw = mmWidth, mh = height;
    int res = -1;
    while(mw>0 || mh>0) {
        res++;
        mw /= 2;
        mh /= 2;
    }
    return res;
}

int MMTexture::getWidth() {
    return width;
}
int MMTexture::getHeight() {
    return height;
}