#include "LevelSelect.h"
#include <nch/cpp-utils/fs-utils.h>
#include <nch/cpp-utils/filepath.h>
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/noah-alloc-table.h>
#include <nch/cpp-utils/string-utils.h>
#include <SDL2/SDL_image.h>
#include "Input.h"
#include "Main.h"
#include "Resources.h"


using namespace nch;

LevelSelect::LevelSelect()
{
    reloadHeaderTxt();
    std::string bp = Main::getBasePath();
    
    FsUtils::ListSettings ls;
    ls.excludeSymlinkDirs = true;
    ls.includeFiles = false;

    sections = FsUtils::lsx(bp+"data", ls);
    std::sort(sections.begin(), sections.end(), [](std::string a, std::string b) {return a<b;});
    
    Log::log("%d sections found.", sections.size());
    setSectionByName("Volume 1");
}
LevelSelect::~LevelSelect(){}

void LevelSelect::tick(Screen& scr)
{
    int mx = Input::getMouseX(), my = Input::getMouseY();

    int maxIdx = *levelIdxes.rbegin();
    int numRows = std::ceil((double)maxIdx/rowSize);
    int boxesTX = scr.getUnscaledWidth()/2-(boxW*rowSize+boxSpacing*(rowSize-1))/2;
    int boxesTY = scr.getUnscaledHeight()/2-(boxH*numRows+boxSpacing*(numRows-1))/2;

    hoveredLvlIdx = -1;
    for(int i = 0; i<=maxIdx; i++) {
        int boxX = i%rowSize, boxY = i/rowSize;
        
        bool hoverable = false;
        if(levelIdxes.find(i)!=levelIdxes.end()) {
            hoverable = true;
        }

        Rect dst = Rect(
            boxesTX+boxX*(boxW+boxSpacing),
            boxesTY+boxY*(boxH+boxSpacing),
            boxW,
            boxH
        );
        scr.transformRect(dst);

        if(hoverable && mx>=dst.x1() && mx<=dst.x2() && my>=dst.y1() && my<=dst.y2()) {
            hoveredLvlIdx = i;
        }
    }

    clickedLvlIdx = -1;
    if(hoveredLvlIdx!=-1 && Input::mouseDownTime(1)==1) {
        clickedLvlIdx = hoveredLvlIdx;
    }
}

void LevelSelect::draw(Screen& scr)
{
    SDL_Renderer* rend = Main::getRenderer();
    
    //Header
    headerTxt.setScale(scr.getScale());
    Rect dst(scr.getUnscaledWidth()/2-headerTxt.getWidth()/scr.getScale()/2, 16, 0, 0);     scr.transformRect(dst);
    headerTxt.draw(dst.r.x, dst.r.y);
    
    //Levels
    bool alwaysVisibleMaps = false;
    #ifdef NDEBUG
        alwaysVisibleMaps = false;
    #endif

    MMTexture* guis = Resources::getTextureByID(Resources::ID::TEX_guis);
    if(guis!=nullptr) {
        int maxIdx = *levelIdxes.rbegin();
        int numRows = std::ceil((double)maxIdx/rowSize);
        int boxesTX = scr.getUnscaledWidth()/2-(boxW*rowSize+boxSpacing*(rowSize-1))/2;
        int boxesTY = scr.getUnscaledHeight()/2-(boxH*numRows+boxSpacing*(numRows-1))/2;
        for(int i = 0; i<=maxIdx; i++) {
            int boxX = i%rowSize, boxY = i/rowSize;
            
            bool questionMark = false;
            if(levelIdxesPlayed.find(i)==levelIdxesPlayed.end()) questionMark = true;
            if(alwaysVisibleMaps) questionMark = false;

            //Main button background
            Rect src0;
            if(levelIdxes.find(i)!=levelIdxes.end()) {
                src0 = Rect(0, 0, 43, 32);
                if(questionMark) src0 = Rect(0, 32, 43, 32);
                if(i==hoveredLvlIdx) src0.r.x += 43;
            } else {
                src0 = Rect(43, 64, 43, 32);
            }
            Rect dst0 = Rect(
                boxesTX+boxX*(boxW+boxSpacing),
                boxesTY+boxY*(boxH+boxSpacing),
                boxW,
                boxH
            );
            scr.transformRect(dst0);
            guis->renderCopy(rend, &src0.r, &dst0.r);

            //If this level was played before
            if(!questionMark)
            try {
                if(levelImages.at(i)==nullptr) {
                    throw std::exception();
                }

                //Map icon
                int lvlImgW, lvlImgH;
                SDL_QueryTexture(levelImages[i], NULL, NULL, &lvlImgW, &lvlImgH);
                dst0 = Rect(
                    boxesTX+boxX*(boxW+boxSpacing)+3*4,
                    boxesTY+boxY*(boxH+boxSpacing)+3*4,
                    boxW-17*4,
                    boxH-6*4
                );
                scr.transformRect(dst0);
                SDL_RenderCopy(rend, levelImages[i], NULL, &dst0.r);

                //Stars
                int starScore = levelStarsEarned[i];
                if(starScore>-1)
                for(int j = 0; j<3; j++) {
                    Rect src1 = Rect(117, 1, 9, 9);

                    if(j+1>starScore) src1.r.y += 10;    //Black star
                    if(i==hoveredLvlIdx) src1.r.x += 10; //Mouse hovering on top of GUI
                    Rect dst1;
                    dst1 = Rect(
                        boxesTX+boxX*(boxW+boxSpacing)+32*4,
                        boxesTY+boxY*(boxH+boxSpacing)+4+9.5*4*j,
                        9*4,
                        9*4
                    );
                    scr.transformRect(dst1);
                    
                    guis->renderCopy(rend, &src1.r, &dst1.r);
                }
            } catch(...) {}
        }
    }
}

int LevelSelect::getSectionIdx() {
    return sectionIdx;
}

std::string LevelSelect::getSectionName()
{
    try { return sections.at(sectionIdx); }
    catch(...) { Log::warnv(__PRETTY_FUNCTION__, "returning \"???null???\"", "Invalid 'sections' or 'sectionIdx'"); }
    return "???null???";
}

void LevelSelect::reloadHeaderTxt()
{
    headerTxt.init(Main::getRenderer(), Resources::ttfOswald48);
    headerTxt.updateTextTexture();
}

void LevelSelect::setSectionIdx(int sidx)
{
    //Prelims
    std::string sec;
    try        { sec = sections.at(sidx); }
    catch(...) { Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "'sidx' out of range"); return; }
    sectionIdx = sidx;

    //Set header text
    headerTxt.setText(sec);
    
    //Build list of files ('levels') within the folder "[base]/data/['sec']".
    FsUtils::ListSettings ls;
    ls.excludeSymlinkDirs = true;
    ls.includeDirs = false;
    std::vector<std::string> levels = FsUtils::lsx(Main::getBasePath()+"data/"+sec, ls);
    
    //Build 'levelIdxes' from entries within 'levels'
    levelIdxes.clear();
    for(int i = 0; i<levels.size(); i++) {
        std::string lvlName = levels.at(i);
        if(StringUtils::aHasPrefixB(lvlName, "lvl_")) {
            lvlName = FilePath(lvlName).getWithoutExtension();
            try {
                levelIdxes.insert(std::atoi(lvlName.substr(4).c_str()));
                continue;
            } catch(...){}
        }
        Log::warnv(__PRETTY_FUNCTION__, "skipping", "Found invalid filename \"%s\"", lvlName.c_str());
    }

    //Build 'levelPlayedIdxes' from saved data
    NoahAllocTable nat(Main::getBasePath()+"data/userdata");
    
    try {
        nlohmann::json j = nlohmann::json::from_bson(nat.load("playedLevels"));
        std::vector<std::string> vec = j["playedLevels"];
        std::string sn = getSectionName()+"/";
        levelIdxesPlayed.clear();
        for(int i = 0; i<vec.size(); i++) {
            if(StringUtils::aHasPrefixB(vec[i], sn)) {
                try {
                    int idx = std::atoi(vec[i].substr(sn.size()+4).c_str());
                    levelIdxesPlayed.insert(idx);
                } catch(...) {
                    Log::warnv(__PRETTY_FUNCTION__, "skipping", "Couldn't get level index number from \"%s\"", vec[i].c_str());
                }
            }
        }
    } catch(...) {}
    
    nat.close();

    //Update GUI buttons' details
    updateLevelImages();    //Map icons
    updateLevelStars();     //Stars earned
}
void LevelSelect::incSectionIdx(int changeIdx)
{
    int newSecIdx = sectionIdx+changeIdx;

    while(newSecIdx<0) newSecIdx += sections.size();
    while(newSecIdx>=sections.size()) newSecIdx -= sections.size();
    setSectionIdx(newSecIdx);
}

void LevelSelect::setSectionByName(std::string secName)
{
    for(int i = 0; i<sections.size(); i++) {
        if(sections.at(i)==secName) {
            setSectionIdx(i);
            return;
        }
    }
    
    Log::warnv(__PRETTY_FUNCTION__, "skipping", "No such secName \"%s\" exists", secName.c_str());
}

int LevelSelect::popClickedLvlIdx()
{
    int res = clickedLvlIdx;
    clickedLvlIdx = -1;
    return res;
}

void LevelSelect::updateLevelImages()
{
    int lvlImgsSize = levelImages.size();
    for(int i = 0; i<lvlImgsSize; i++)  {
        SDL_DestroyTexture(levelImages[0]);
        levelImages.erase(levelImages.begin());
    }

    SDL_Surface* tempImg;
    for(int i = *levelIdxes.begin(); i<=*levelIdxes.rbegin(); i++) {
        std::stringstream ss;
        ss << Main::getBasePath() << "data/" << sections.at(sectionIdx) << "/lvl_" << i << ".png";
        tempImg = IMG_Load(ss.str().c_str());
        if(tempImg!=NULL) {
            levelImages.push_back(SDL_CreateTextureFromSurface(Main::getRenderer(), tempImg));
        } else {
            levelImages.push_back(nullptr);
        }
        
        SDL_FreeSurface(tempImg);
    }
}

void LevelSelect::updateLevelStars()
{
    levelStarsEarned.clear();

    NoahAllocTable nat(Main::getBasePath()+"data/userdata");

    nlohmann::json j;
    try { j = nlohmann::json::from_bson(nat.load("stars")); }
    catch(...) {}

    for(int i = *levelIdxes.begin(); i<=*levelIdxes.rbegin(); i++) {
        std::stringstream ss; ss << getSectionName() << "/lvl_" << i;
        try {
            levelStarsEarned.push_back(j.at("stars").at(ss.str()));
        } catch(...) {
            levelStarsEarned.push_back(-1);
        }
        
    }
}