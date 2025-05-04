#include "World.h"
#include <fstream>
#include <nch/cpp-utils/filepath.h>
#include <nch/cpp-utils/fs-utils.h>
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/noah-alloc-table.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/input.h>
#include <nch/sdl-utils/rect.h>
#include <nlohmann/json.hpp>
#include "Main.h"
#include "Resources.h"

using namespace nch;

World::World()
{
    Log::log("World initialized.");
}
World::~World(){}

void World::loadLevel(std::string levelPath)
{
    World::levelPath = levelPath;

    //Load tile map data
    FilePath fpLvl(Main::getBasePath()+"data/"+levelPath+".json");
    std::ifstream ifs(fpLvl.get());
    nlohmann::json jLvl = nlohmann::json::parse(ifs);
    //map.loadFromJSON(jLvl);
    map.load(Main::getBasePath()+"data/"+levelPath+".png");
    
    //See if mapdata exists
    nlohmann::json jMapData;
    try         { jMapData = jLvl.at("mapData"); }
    catch(...)  {
        Log::warnv(__PRETTY_FUNCTION__, "falling back on default values", "Couldn't find valid \"mapData\" object within JSON");
        return;
    }

    //Place player and cursor
    try {
        std::vector<double> ps = jMapData.at("playerStart");
        std::vector<double> cs = jMapData.at("cursorStart");
        plyr.setPos(Vec2d(64*ps.at(0), 64*ps.at(1)));
        crsr.setPos(Vec2i(64*cs.at(0), 64*cs.at(1)));
    } catch(...) {
        Log::warn(__PRETTY_FUNCTION__, "Something went wrong while getting player and cursor positions from \"mapData\"");
        return;
    }

    //Get par time
    try {
        int pt = jMapData.at("par");
        parTicks = (uint64_t)pt*40;
    } catch(...) {
        Log::warn(__PRETTY_FUNCTION__, "Something went wrong while getting par from \"mapData\"");
    }

    /* Generate various save data */
    Timer t("User data saving", true);

    #ifndef NDEBUG
        //Save image of latest map played
        map.save(Main::getBasePath()+"latest");
    #endif

    //Get user save data
    NoahAllocTable nat(Main::getBasePath()+"data/userdata");
    //Get list of played levels as a json structure, then add the current 'levelPath' to it    
    nlohmann::json j;
    j["playedLevels"] = {};
    try { j = nlohmann::json::from_bson(nat.load("playedLevels")); } catch(...) {}
    if(!j["playedLevels"].contains(levelPath)) {
        j["playedLevels"].push_back(levelPath);
    }
    //Save json structure
    auto bson = nlohmann::json::to_bson(j);
    nat.save("playedLevels", bson);
}

void World::tick()
{
    /* Update map */
    map.tick(plyr.getPos(), crsr.getPos().toDouble());

    /* Interactions */
    Vec2<double> vel;
    if(crsr.hasStartedMoving()) {
        //Set player velocity from (cursor pos minus player pos)
        vel = (crsr.getPos().toDouble()-plyr.getPos())*0.05;
        double maxSpeed = 8;
        if(vel.length()>maxSpeed) {
            vel = vel.normalized()*maxSpeed;
        }
        //Cursor hurts player
        if(plyr.getZoneType()==0 && plyr.getBounds().contains(crsr.getPos().toDouble())) {
            plyr.damage(0.5);
        }
    }

    
    /* Update player */ {
        //Set velocity
        plyr.setVel(vel);
        //Tick
        plyr.tick();
        //Process velocity AND collision for player
        auto pb = plyr.getBounds();
        Box2<int> area = map.getTileAreaByWorldArea(Box2<double>(pb.c1.x-64, pb.c1.y-64, pb.c2.x+64, pb.c2.y+64));
        std::vector<std::pair<Vec2<int>, Tile>> collidingTiles; collidingTiles.reserve(25);
        for(int ix = area.c1.x; ix<=area.c2.x; ix++)
        for(int iy = area.c1.y; iy<=area.c2.y; iy++) {
            Tile tile = map.getTile(ix, iy);
            collidingTiles.push_back(std::make_pair(Vec2i(ix, iy), tile));
        }
        plyr.processVelocity(collidingTiles);
    }


    /* Lose/win logic */
    if(state==DEFAULT && (plyr.isDead() || crsr.isDead())) {
        plyr.damage(10000);
        crsr.kill();
        state = LOST;
    }
    if(state<WON && plyr.getZoneType()==2) {
        win();
        state = WON;
    }

    if(crsr.hasStartedMoving()) {
        if(numTicksActive==0) {
            //Play "start" sound
            Resources::getSFXByID(Resources::SFX_spawn)->playback();
        }
        numTicksActive++;
    }

}

void World::draw(Screen& scr)
{
    //Update cursor movement (in draw() since it should update faster than 60 per second)
    if(scr.isFocused()) {
        crsr.updatePos(map);
    }

    //World objects
    map.drawByLayer(scr, 1);
    plyr.draw(scr);
    map.drawByLayer(scr, 2);
}

void World::drawCursor(Screen& scr) {
    crsr.draw(scr);
}

int World::getState() {
    return state;
}
Box2<double> World::getMapBounds() {
    return map.getBounds();
}
uint64_t World::getNumTicksActive() {
    return numTicksActive;
}
uint64_t World::getParTicks() {
    return parTicks;
}
double World::getCurrentStarScore()
{
    uint64_t tx = getNumTicksActive();
    uint64_t parTx = getParTicks();
    if(parTx>999999) return 0;

    //Number of stars earned should = ceil(numStars)
    double numStars = 3;
    if(tx>parTx*1.0-40*2) numStars = 2.5;
    if(tx>parTx*1.0)      numStars = 2.0;
    if(tx>parTx*1.5-40*2) numStars = 1.5;
    if(tx>parTx*1.5)      numStars = 1.0;
    if(tx>parTx*2.0-40*2) numStars = 0.5;
    if(tx>parTx*2.0)      numStars = 0.0;
    return numStars;
}

void World::templockCursor() {
    crsr.templock();
}

void World::setThemeBySection(std::string lvlSection)
{
    std::unordered_map<std::string, int> themeMap = {
        std::make_pair("Custom", Map::BLUE_RED),
        std::make_pair("Volume 1", Map::BLUE_RED),
        std::make_pair("Volume 2", Map::YELLOW_ORANGE),
        std::make_pair("Volume 3", Map::YELLOW_GREEN),
        std::make_pair("Volume 4", Map::BLUE_PURPLE),
        std::make_pair("Volume 5", Map::RED_ORANGE),
        std::make_pair("Volume 6", Map::WHITE_GRAY),
        std::make_pair("Volume 7", Map::GOLD_SILVER),
        std::make_pair("Volume 8", Map::BLACK_DARKRED),
        std::make_pair("Volume 9", Map::BLACK_DARKRED),
    };
    
    auto itr = themeMap.find(lvlSection);
    if(itr!=themeMap.end()) {
        map.setBackgroundTheme(itr->second);
        return;
    }
    map.setBackgroundTheme(Map::BLACK_PINK);
}

void World::win()
{
    int numStarsEarned = (int)std::ceil(getCurrentStarScore());
    Resources::getSFXByID(Resources::SFX_win)->playback();
    if(levelPath=="???null???") {
        Log::warnv(__PRETTY_FUNCTION__, "stopping save operation", "levelPath is not set");
        return;
    }


    //Get user save data
    NoahAllocTable nat(Main::getBasePath()+"data/userdata");
    //Get list of played levels as a json structure, then add the current 'levelPath' to it    
    nlohmann::json j;
    j["stars"] = {};
    try { j = nlohmann::json::from_bson(nat.load("stars")); } catch(...) {}
    try {
        int oldStarsEarned = j.at("stars").at(levelPath);
        if(numStarsEarned>oldStarsEarned) {
            j.at("stars").at(levelPath) = numStarsEarned;
        }
    } catch(...) {
        j["stars"][levelPath] = numStarsEarned;
    }


    Log::log("Earned %d stars on level \"%s\"", numStarsEarned, levelPath.c_str());
    
    Log::log("Current json: \"%s\"", j.dump().c_str());

    //Save json structure
    auto bson = nlohmann::json::to_bson(j);
    nat.save("stars", bson);
}
