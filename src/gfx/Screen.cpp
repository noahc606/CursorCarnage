#include "Screen.h"
#include "Main.h"

using namespace nch;

void Screen::init(SDL_Renderer* rend)
{
    Screen::rend = rend;
    initialized = true;
}

void Screen::update(Box2<double> newScrBounds)
{
    int mw = Main::getWidth();
    int mh = Main::getHeight();

    double factorW = mw/newScrBounds.getWidth();
    double factorH = mh/newScrBounds.getHeight();
    if(factorW>=factorH)    { scale = factorH; }
    else                    { scale = factorW; }

    for(double i = 32; i>=1; i--) {
        if(scale>=i/32.0f) {
            scale = i/32.0f;
            break;
        } else continue;
    }

    bounds = Box2<double>::createFromXYWH(
        (mw-(scale*newScrBounds.getWidth()))/2,
        (mh-(scale*newScrBounds.getHeight()))/2,
        scale*newScrBounds.getWidth(),
        scale*newScrBounds.getHeight()
    );
}

void Screen::drawBorders()
{
    SDL_Renderer* rend = Main::getRenderer();
    int tx = bounds.c1.x;
    int ty = bounds.c1.y;
    int bw = bounds.getWidth();
    int bh = bounds.getHeight();
    int mw = Main::getWidth();
    int mh = Main::getHeight();

    if(Main::isDebugging()) { SDL_SetRenderDrawColor(rend, 0, 128, 0, 255); }
    else                    { SDL_SetRenderDrawColor(rend, 0, 0, 0, 255); }
    
    Rect dst;
    dst = Rect(0, 0, mw, ty);             SDL_RenderFillRect(rend, &dst.r);
    dst = Rect(0, 0, tx, mh);             SDL_RenderFillRect(rend, &dst.r);
    dst = Rect(tx, ty+bh, bw+tx+5, ty+5); SDL_RenderFillRect(rend, &dst.r);
    dst = Rect(tx+bw, ty, tx+5, bh+ty+5); SDL_RenderFillRect(rend, &dst.r);
}

SDL_Renderer* Screen::getRenderer() {
    return rend;
}
bool Screen::isFocused() {
    return focused;
}
double Screen::getScale() {
    return scale;
}
Box2<double> Screen::getBounds() {
    return bounds;
}
double Screen::getUnscaledWidth() {
    return bounds.getWidth()/scale;
}
double Screen::getUnscaledHeight() {
    return bounds.getHeight()/scale;
}

bool Screen::wasInitialized() {
    return initialized;
}

void Screen::setFocused(bool focused) {
    Screen::focused = focused;
}

void Screen::transformRect(Rect& rect)
{
    rect.scale(scale);
    rect.translate(bounds.c1.x, bounds.c1.y);
}