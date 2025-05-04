#pragma once
#include <nch/math-utils/vec2.h>
#include <SDL2/SDL.h>
#include "Cursor.h"
#include "Map.h"
#include "Player.h"

class World {
public:
    enum WorldState {
        DEFAULT = 0,
        LOST,
        WON,
    };

    World();
    ~World();
    void loadLevel(std::string levelPath);

    void tick();
    void draw(Screen& scr);
    void drawCursor(Screen& scr);
    
    int getState();
    nch::Box2<double> getMapBounds();
    uint64_t getNumTicksActive();
    uint64_t getParTicks();
    double getCurrentStarScore();

    void templockCursor();
    void setThemeBySection(std::string lvlSection);

private:
    void win();

    //World objects
    Player plyr;
    Map map;
    Cursor crsr;
    int state = DEFAULT;
    uint64_t numTicksActive = 0;
    uint64_t parTicks = -1;
    std::string levelPath = "???null???";
};