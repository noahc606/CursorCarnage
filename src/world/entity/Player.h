#pragma once
#include <nch/math-utils/box2.h>
#include <nch/math-utils/vec2.h>
#include <vector>
#include "Collision.h"
#include "Screen.h"
#include "Tile.h"

class Player {
public:

    Player();
    ~Player();

    void tick();
    void draw(Screen& scr);

    nch::Vec2<double> getPos();
    bool rectCollides(nch::Box2<double>& r);
    nch::Box2<double> getBounds(Collision::Direction dir);
    nch::Box2<double> getBounds();
    bool isDead();
    int getZoneType();

    void setPos(const nch::Vec2<double>& pos);
    void setVel(const nch::Vec2<double>& vel);
    void processVelocity(std::vector<std::pair<nch::Vec2<int>, Tile>>& collidingTiles);
    void damage(double val);
    void heal(double val);
private:
    void updateBounds();
    void updateZoneType(int newZone);

    const int size = 96;
    nch::Box2<double> bounds[5] = { nch::Box2<double>() };
    nch::Vec2<double> pos = nch::Vec2<double>(1*64, 1*64);
    nch::Vec2<double> vel;

    int zoneType = 0;

    uint64_t lastDamageTick = 0;
    uint64_t numTicksHealthy = 0;   //Full health
    uint64_t numTicksUnhealthy = 0; //NOT at full health
    double health = 100;
};