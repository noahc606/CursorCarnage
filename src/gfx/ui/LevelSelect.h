#pragma once
#include <nch/sdl-utils/text.h>
#include <set>
#include <string>
#include <vector>
#include "Screen.h"

class LevelSelect {
public:
    LevelSelect();
    ~LevelSelect();

    void tick(Screen& scr);
    void draw(Screen& scr);

    int getSectionIdx();
    std::string getSectionName();

    void reloadHeaderTxt();
    void setSectionIdx(int sidx);
    void incSectionIdx(int changeIdx);
    void setSectionByName(std::string secName);
    int popClickedLvlIdx();

private:
    void updateLevelImages();
    void updateLevelStars();

    nch::Text headerTxt;
    std::vector<std::string> sections;
    std::set<int> levelIdxes;
    std::set<int> levelIdxesPlayed;
    std::vector<SDL_Texture*> levelImages;
    std::vector<int> levelStarsEarned;
    int sectionIdx = 0;
    int clickedLvlIdx = -1;

    //Level selection GUI
    int rowSize = 8;
    int boxW = 42*4, boxH = 31*4;
    int boxSpacing = 5*4;
    int hoveredLvlIdx = -1;
};