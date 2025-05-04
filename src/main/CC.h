#pragma once
#include <nch/sdl-utils/text.h>
#include "Button.h"
#include "LevelSelect.h"
#include "World.h"

class CC {
public:
    enum GameState {
        NOTHING = -1,
        SPLASH_SCREEN,
        TITLE,
        LEVEL_SELECT,
        WORLD,
        QUIT,
    };

    CC(); ~CC();

    void tick();
    void draw();
    void drawCursor();
    void drawLossScreen();
    void drawTitleBackground();
    void drawSplashScreen();
    void drawTimer();

    int getGamestate();

    void reloadResources();
private:
    void setGamestate(int gs);
    void unloadWorld();
    void enterLevel(bool advanceLevel);
    void enterLevel();
    void initUIs();

    int gamestate = NOTHING;

    //Game objects
    Button* btnFreeplay = nullptr, * btnQuit = nullptr;
    Button* btnBack = nullptr, * btnPrev = nullptr, * btnNext = nullptr;
    nch::Text splashTxt;
    nch::Text lossTxt;
    nch::Text continueTxt;
    nch::Text timerTxt;
    
    World* world = nullptr;
    Screen screen;
    LevelSelect lvlSel;

    //Game state tracking
    uint64_t ticksOnSplashScreen = 0;
    uint64_t ticksWhileLost = 0;
    bool lossMsgNeedsUpdate = false;
    std::string selectedLvlSection = "???null???";
    int selectedLvlIndex = 0;

    //Main initialization
    bool initialized = false;
};