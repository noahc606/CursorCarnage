#include "Button.h"
#include <nch/sdl-utils/input.h>
#include "Main.h"
#include "Rect.h"
#include "Resources.h"

using namespace nch;

Button::Button(int id)
{
    Button::id = id;
    rebuildTexes();
}

Button::~Button()
{
    if(btnTex0!=nullptr) SDL_DestroyTexture(btnTex0);
    if(btnTex1!=nullptr) SDL_DestroyTexture(btnTex1);
}

void Button::rebuildTexes()
{
    btnTxt.init(Main::getRenderer(), Resources::ttfOswald48);
    btnTxt.setTextColor(Color(0, 0, 0));
    btnTxt.setShadowCustomColor(Color(80, 80, 80));
    btnTxt.updateTextTexture();

    SDL_Renderer* rend = Main::getRenderer();
    
    //Build btnTex0 and btnTex1
    if(btnTex0!=nullptr) SDL_DestroyTexture(btnTex0);
    if(btnTex1!=nullptr) SDL_DestroyTexture(btnTex1);
    btnTex0 = SDL_CreateTexture(rend, Main::getPixelFormat()->format, SDL_TEXTUREACCESS_TARGET, (int)(width)+4, 32);
    btnTex1 = SDL_CreateTexture(rend, Main::getPixelFormat()->format, SDL_TEXTUREACCESS_TARGET, (int)(width)+4, 32); {
        MMTexture* guis = Resources::getTextureByID(Resources::TEX_guis);
        Rect src, dst;

        //Left part
        src = Rect(87, 0, 2, 32);
        dst = Rect(0, 0, 2, 32);
        SDL_SetRenderTarget(rend, btnTex0); guis->renderCopy(rend, &src.r, &dst.r);
        src.r.x += 15;
        SDL_SetRenderTarget(rend, btnTex1); guis->renderCopy(rend, &src.r, &dst.r);
        //Middle parts
        for(int i = 0; i<width/8; i++) {
            src = Rect(90, 0, 8, 32);
            dst = Rect(2+i*8, 0, 8, 32);
            SDL_SetRenderTarget(rend, btnTex0); guis->renderCopy(rend, &src.r, &dst.r);
            src.r.x += 15;
            SDL_SetRenderTarget(rend, btnTex1); guis->renderCopy(rend, &src.r, &dst.r);
        }
        //Right part
        src = Rect(99, 0, 2, 32);
        dst = Rect(2+(int)(width), 0, 2, 32);
        SDL_SetRenderTarget(rend, btnTex0); guis->renderCopy(rend, &src.r, &dst.r);
        src.r.x += 15;
        SDL_SetRenderTarget(rend, btnTex1); guis->renderCopy(rend, &src.r, &dst.r);
        SDL_SetRenderTarget(rend, NULL);
    }
}

void Button::tick(Screen& scr)
{
    int mx = Input::getMouseX();
    int my = Input::getMouseY();

    Rect dst(x, y, (width+4)*rs, 32*rs);
    scr.transformRect(dst);

    if(mx>=dst.x1() && mx<=dst.x2() && my>=dst.y1() && my<=dst.y2()) {
        hovering = true;
    } else {
        hovering = false;
    }

    if(clicked) hovering = true;

    if(hovering && Input::mouseDownTime(1)==1) {
        clicked = true;
    }
}

void Button::draw(Screen& scr)
{
    SDL_Renderer* rend = Main::getRenderer();

    Rect dst(x, y, (width+4)*rs, 32*rs);
    scr.transformRect(dst);
    if(!hovering) { SDL_RenderCopy(rend, btnTex0, NULL, &dst.r); }
    else          { SDL_RenderCopy(rend, btnTex1, NULL, &dst.r); }

    
    if(hovering) {
        btnTxt.setScale(scr.getScale()*1/2*rs);
        btnTxt.setTextColor(Color(64, 64, 128));
    } else {
        btnTxt.setScale(scr.getScale()*2/5*rs);
        btnTxt.setTextColor(Color(0, 0, 0));
    }
    btnTxt.draw(dst.r.x+dst.r.w/2-btnTxt.getWidth()/2, dst.r.y+dst.r.h/2-btnTxt.getHeight()/2-2*rs*scr.getScale());
}

int Button::getUnscaledWidth() {
    return width*rs;
}
int Button::getUnscaledHeight() {
    return height*rs;
}
int Button::getID() {
    return id;
}

void Button::setPos(Vec2<int> pos)
{
    x = pos.x;
    y = pos.y;
}

void Button::setText(std::string s) {
    btnTxt.setText(s);
}

void Button::setWidth(int width) {
    int prevWidth = Button::width;
    Button::width = width/8*8;
    if(width!=prevWidth) {
        rebuildTexes();
    }
}

bool Button::popClicked()
{
    if(!clicked) {
        return false;
    }

    AudioChunk* btnSound = Resources::getSFXByID(Resources::SFX_button);
    if(btnSound!=nullptr) btnSound->playback();
    clicked = false;
    return true;
}