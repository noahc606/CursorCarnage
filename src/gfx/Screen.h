#pragma once
#include <nch/math-utils/box2.h>
#include <nch/math-utils/vec2.h>
#include <nch/sdl-utils/rect.h>
#include <SDL2/SDL.h>

class Screen {
public:
    void init(SDL_Renderer* rend);
    void update(nch::Box2<double> newScrBounds);
    void drawBorders();

    SDL_Renderer* getRenderer();
    bool isFocused();
    double getScale();
    nch::Box2<double> getBounds();
    double getUnscaledWidth();
    double getUnscaledHeight();
    bool wasInitialized();

    void setFocused(bool focused);
    void transformRect(nch::Rect& rect);
private:
    SDL_Renderer* rend = nullptr;
    bool focused = true;
    double scale = 0.25;
    nch::Box2<double> bounds;

    bool initialized = false;
};