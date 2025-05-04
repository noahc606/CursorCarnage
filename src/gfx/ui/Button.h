#pragma once
#include <nch/math-utils/vec2.h>
#include <nch/sdl-utils/text.h>
#include <SDL2/SDL.h>
#include "Screen.h"

class Button {
public:
    enum ID {
        freeplay,
        quit,
        back_to_title,
        next, prev,
    };

    Button(int id);
    ~Button();
    void rebuildTexes();

    void tick(Screen& scr);
    void draw(Screen& scr);

    int getUnscaledWidth();
    int getUnscaledHeight();
    int getID();

    void setPos(nch::Vec2<int> pos);
    void setText(std::string s);
    void setWidth(int width);

    bool popClicked();

private:

    int x = 0; int y = 0;
    double rs = 2;  //relative scale
    int width = 32*8; int height = 32;
    bool hovering = false;
    bool clicked = false;

    nch::Text btnTxt;
    SDL_Texture* btnTex0 = nullptr, * btnTex1 = nullptr;
    int id = -1;
};