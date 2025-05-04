#pragma once
#include <nch/cpp-utils/color.h>
#include <nch/math-utils/box2.h>
#include <nch/math-utils/vec2.h>
#include <nlohmann/json.hpp>
#include <SDL2/SDL.h>
#include "Screen.h"
#include "Tile.h"

class Map {
public:
    enum BackgroundTheme
    {
        BLUE_RED,
        YELLOW_ORANGE,
        YELLOW_GREEN,
        BLUE_PURPLE,
        RED_ORANGE,
        WHITE_GRAY,
        GOLD_SILVER,
        BLACK_DARKRED,

        BLACK_PINK,    //unknown
    };

    Map();
    ~Map();
    bool loadFromJSON(nlohmann::json j);
    bool load(std::string path);
    void save(std::string path);

    void tick(nch::Vec2<double> playerPos, nch::Vec2<double> cursorPos);
    void drawByLayer(Screen& scr, int layer);

    Tile getTile(int x, int y);
    int getWidth(); int getHeight();
    int getWidthPx(); int getHeightPx();
    nch::Box2<double> getBounds();
    static nch::Box2<int> getTileAreaByWorldArea(nch::Box2<double> warea);
    static int getTilePosByWorldPos(double wpos);
    static nch::Vec2<int> getTilePosByWorldPos(nch::Vec2<double> wpos);

    void setTile(int x, int y, const Tile& t);
    void setBackgroundTheme(int bgTheme);

private:
    void alloc(int width, int height);
    void free();

    double bgInterpolVal = 0.5;
    nch::Color bgBaseColor1, bgBaseColor2;
    int width = 8;
    int height = 8;
    Tile** tiles = nullptr;
    bool loaded = false;
};