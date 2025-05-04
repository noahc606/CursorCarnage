#pragma once
#include <nch/math-utils/vec2.h>
#include <nch/sdl-utils/rect.h>
#include <vector>
#include "Map.h"
#include "Screen.h"

class Cursor {
public:
    Cursor();
    ~Cursor();

    void draw(Screen& scr);
    void updatePos(Map& map);
    nch::Vec2<int> getPos();
    bool isDead();
    bool hasStartedMoving();

    void setNotStarted();
    void templock();
    void setPos(const nch::Vec2<int>& pos);
    void kill();
private:
    void processMovement(std::vector<std::pair<nch::Vec2<int>, Tile>>& collidingTiles, nch::Vec2<double>& dpos);

    nch::Vec2<int> pos;

    bool started = false;
    bool dead = false;
    uint64_t lockTicks = 0;
};