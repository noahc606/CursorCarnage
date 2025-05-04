#pragma once
#include <nch/cpp-utils/color.h>
#include <nch/sdl-utils/rect.h>
#include <nch/math-utils/box2.h>
#include <nch/math-utils/vec2.h>
#include <SDL2/SDL.h>

class Tile {
public:
    enum TileID {
        AIR,
        PLAYER_WALL,    PLAYER_WALL_MINI,
        PLAYER_KILLER,  PLAYER_KILLER_MINI,
        CURSOR_WALL,    CURSOR_WALL_MINI,
        CURSOR_KILLER,  CURSOR_KILLER_MINI,
        OP_WALL,        OP_WALL_MINI,
        OP_KILLER,      OP_KILLER_MINI,
        AND_GATE,       AND_GATE_OPEN,
        AND_GATE_PLAYER,AND_GATE_CURSOR,
        SLOW_ZONE,

        UNKNOWN = 96,
        MAP_START = 97,
        MAP_END = 98,
        MAP_WALL = 99,
    };

    Tile();
    Tile(int id);
    ~Tile();

    void draw(SDL_Renderer* rend, nch::Rect& dst);
    int getID();
    nch::Box2<double> getBounds() const;
    nch::Box2<double> getWorldBounds(nch::Vec2<int> tilePos) const;
    bool canCollideWithPlayer();
    bool canCollideWithCursor();
    bool getKillsPlayer();
    bool getKillsCursor();
    nch::Color getColor();

private:
    int id = 99;
};