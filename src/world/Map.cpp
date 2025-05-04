#include "Map.h"
#include <nch/cpp-utils/log.h>
#include <nch/sdl-utils/rect.h>
#include <nch/sdl-utils/texture-utils.h>
#include <SDL2/SDL_image.h>
#include "Main.h"
#include "Resources.h"
#include "TileProperties.h"

#include <nch/cpp-utils/timer.h>

using namespace nch;

Map::Map()
{
    setBackgroundTheme(0);

    alloc(32, 32);
    for(int x = 0; x<32 && x<width; x++) {
        setTile(x, 5, 2);
        setTile(x, 9, 2);
        setTile(x, 10, 1);

        setTile(x, 0, 3);
        setTile(x, 1, 4);
        setTile(x, 30, 4);
        setTile(x, 31, 3);
    }
    for(int y = 0; y<32 && y<height; y++) {
        setTile(23, 5, 1);
        setTile(y, y, 1);
    }
    for(int y = 0; y<32 && y<height; y++) {
        setTile(10, y, 0);
        setTile(11, y, 0);
    }
    setTile(9, 10, 0);
}

Map::~Map() {
    free();
}

bool Map::loadFromJSON(nlohmann::json j)
{
    //To be populated
    std::vector<std::string> tileMap;
    std::map<std::string, int> tilePalette;
    //Get data from JSON 'j' to populate 'tileMap' and 'tilePalette'
    try {
        tileMap = j.at("tileMap");
        
        nlohmann::json jtp = j.at("tilePalette");
        for(auto jitr = jtp.begin(); jitr!=jtp.end(); jitr++) {
            auto k = jitr.key();
            auto v = jitr.value();
            tilePalette.insert(std::make_pair(k, v));
        }
    } catch(...) {
        Log::error(__PRETTY_FUNCTION__, "Map loading error", "Bad JSON provided");
        return false;
    }

    //Set map dimensions
    int tmHeight = tileMap.size();
    int max = 0;
    for(int i = 0; i<tileMap.size(); i++) {
        if(tileMap[i].size()>max)
            max = tileMap[i].size();
    }
    int tmWidth = max;

    //Allocate map
    free();
    alloc(tmWidth, tmHeight);

    //Set map tiles
    for(int iy = 0; iy<tileMap.size(); iy++) {
        std::string tileRowStr = tileMap[iy];
        for(int ix = 0; ix<tileRowStr.size(); ix++) {
            auto tpItr = tilePalette.find(tileRowStr.substr(ix, 1));
            if(tpItr!=tilePalette.end()) {
                setTile(ix, iy, tpItr->second);
            } else {
                Log::warn(__PRETTY_FUNCTION__, "using ID 0", "No such palette element '%s' exists @ (%d, %d)", tileRowStr.substr(ix, 1).c_str(), ix, iy);
                setTile(ix, iy, 0);
            }
        }
    }

    return true;
}

bool Map::load(std::string path)
{
    SDL_Surface* lvlImg = IMG_Load(path.c_str());
    if(lvlImg==NULL) {
        return false;
    }

    free();
    alloc(lvlImg->w, lvlImg->h);

    for(int ix = 0; ix<width; ix++)
    for(int iy = 0; iy<height; iy++) {
        setTile(ix, iy, TileProperties::getIDByColor(TexUtils::getPixelColor(lvlImg, ix, iy)));
    }
    return true;
}

void Map::save(std::string path)
{
    SDL_Surface* lvlImg = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, Main::getPixelFormat()->format);
    for(int ix = 0; ix<width; ix++)
    for(int iy = 0; iy<height; iy++) {
        Tile t = getTile(ix, iy);
        TexUtils::setPixelColor(lvlImg, ix, iy, t.getColor().getRGBA());
    }
    
    IMG_SavePNG(lvlImg, (path+".png").c_str());
}

void Map::tick(Vec2<double> playerPos, Vec2<double> cursorPos)
{
    bool playSfx0 = false;
    bool playSfx1 = false;
    for(int ix = 0; ix<width; ix++)
    for(int iy = 0; iy<height; iy++) {
        int id = getTile(ix, iy).getID();

        bool andGate = false;
        switch(id) {
            case Tile::AND_GATE:
            case Tile::AND_GATE_OPEN:
            case Tile::AND_GATE_PLAYER:
            case Tile::AND_GATE_CURSOR: {
                andGate = true;
            } break;
        }

        if(andGate) {
            Vec2d tileCenter = Vec2d(ix*64+32, iy*64+32);
            bool playerNear = (tileCenter.distanceTo(playerPos)<64+48);
            bool cursorNear = (tileCenter.distanceTo(cursorPos)<64);

            if(playerNear) {
                if(cursorNear) { setTile(ix, iy, Tile::AND_GATE_OPEN);   }
                else           { setTile(ix, iy, Tile::AND_GATE_PLAYER); }
            } else {
                if(cursorNear) { setTile(ix, iy, Tile::AND_GATE_CURSOR); }
                else           { setTile(ix, iy, Tile::AND_GATE);        }
            }

            //Upon tiles changing, play sound effect
            int newID = getTile(ix, iy).getID();
            if(newID!=id) {
                if(
                    (id==Tile::AND_GATE         && newID==Tile::AND_GATE_CURSOR) ||
                    (id==Tile::AND_GATE_CURSOR  && newID==Tile::AND_GATE)        ||
                    (id==Tile::AND_GATE_OPEN    && newID==Tile::AND_GATE_CURSOR) ||
                    (id==Tile::AND_GATE_PLAYER  && newID==Tile::AND_GATE_OPEN)
                ) {
                    playSfx0 = true;
                }

                if(
                    (id==Tile::AND_GATE         && newID==Tile::AND_GATE_PLAYER) ||
                    (id==Tile::AND_GATE_PLAYER  && newID==Tile::AND_GATE)        ||
                    (id==Tile::AND_GATE_OPEN    && newID==Tile::AND_GATE_PLAYER) ||
                    (id==Tile::AND_GATE_CURSOR  && newID==Tile::AND_GATE_OPEN)
                ) {
                    playSfx1 = true;
                }
            }
        }
    }

    if(playSfx0) Resources::getSFXByID(Resources::SFX_andgate_open_cursor)->playback();
    if(playSfx1) Resources::getSFXByID(Resources::SFX_andgate_open_player)->playback();
}

void Map::drawByLayer(Screen& scr, int layer)
{
    Color bg1;
    Color bg2;

    //[T]ile[s]cale
    double ts = 64*scr.getScale();

    //Iterate thru tile map
    for(int ix = 0; ix<width; ix++)
    for(int iy = 0; iy<height; iy++) {
        Tile tile = getTile(ix, iy);
        Rect tileArea = Rect(ix*ts+scr.getBounds().c1.x, iy*ts+scr.getBounds().c1.y, ts, ts);        

        MMTexture* tileset = Resources::getTextureByID(Resources::ID::TEX_tileset);
        if(tileset!=nullptr) tileset->setBlendMode(SDL_BLENDMODE_BLEND);
        switch(layer) {
            case 1: {
                bg1 = bgBaseColor1;
                bg2 = bgBaseColor2;
                bg1.brighten( -8+4*std::sin(Timer::getTicks()/250.0) );
                bg2.brighten( -8+4*std::sin(Timer::getTicks()/250.0) );

                switch(tile.getID()) {
                    case Tile::SLOW_ZONE: {
                        bg1 = bg1.getInterpolColor(Color(255, 255, 0), bgInterpolVal);
                        bg2 = bg2.getInterpolColor(Color(255, 255, 0), bgInterpolVal);
                    } break;
                    case Tile::MAP_START: {
                        bg1 = bg1.getInterpolColor(Color(0, 192, 0), bgInterpolVal);
                        bg2 = bg2.getInterpolColor(Color(0, 192, 0), bgInterpolVal);
                    } break;
                    case Tile::MAP_END: {
                        bg1 = bg1.getInterpolColor(Color(0, 0, 255), bgInterpolVal);
                        bg2 = bg2.getInterpolColor(Color(0, 0, 255), bgInterpolVal);
                    } break;
                }

                if((ix+iy)%2==0) { SDL_SetRenderDrawColor(scr.getRenderer(), bg1.r, bg1.g, bg1.b, 255); }
                else             { SDL_SetRenderDrawColor(scr.getRenderer(), bg2.r, bg2.g, bg2.b, 255); }
                SDL_RenderFillRect(scr.getRenderer(), &tileArea.r);

                //Tiles that should be on layer 1
                if(tile.canCollideWithPlayer()) {
                    tile.draw(scr.getRenderer(), tileArea);
                }
            } break;
            case 2: {
                //Tiles that should be on layer 2
                if(tile.canCollideWithCursor() && !tile.canCollideWithPlayer()) {
                    tile.draw(scr.getRenderer(), tileArea);
                }
            } break;
        }
    }
}


Tile Map::getTile(int x, int y)
{
    if(x<0||x>=width||y<0||y>=height) {
        return 99;
    }
    return tiles[x][y];
}
int Map::getWidth() {
    return width;
}
int Map::getHeight() {
    return height;
}
int Map::getWidthPx() {
    return width*64;
}
int Map::getHeightPx() {
    return height*64;
}
Box2<double> Map::getBounds() {
    return Box2<double>(0, 0, getWidthPx(), getHeightPx());
}

Box2<int> Map::getTileAreaByWorldArea(Box2<double> warea)
{
    return Box2<int>(
        getTilePosByWorldPos(warea.c1),
        getTilePosByWorldPos(warea.c2)
    );
}
Vec2<int> Map::getTilePosByWorldPos(Vec2<double> pos) {
    return Vec2<int>(floor(pos.x/64.0), floor(pos.y/64.0));
}
int Map::getTilePosByWorldPos(double pos) {
    return floor(pos/64.0);
}

void Map::setTile(int x, int y, const Tile& t)
{
    if(x<0||x>=width||y<0||y>=height) {
        Log::warnv(__PRETTY_FUNCTION__, "returning -1", "Access out of bounds");
        return;
    }
    tiles[x][y] = t;
}

void Map::setBackgroundTheme(int bgTheme) {
    switch(bgTheme) {
        case BLUE_RED: {
            bgBaseColor1 = Color(255, 192, 192); //Red-ish
            bgBaseColor2 = Color(192, 192, 255); //Blue-ish
        } break;
        case YELLOW_ORANGE: {
            bgBaseColor1 = Color(215, 168,  56); //Yellow-ish
            bgBaseColor2 = Color(215, 125,  56); //Orange-red-ish
        } break;
        case YELLOW_GREEN: {
            bgBaseColor1 = Color(215, 168,  56); //Yellow-ish
            bgBaseColor2 = Color(127, 255,   0); //Yellow-green
        }
        case BLUE_PURPLE: {
            bgBaseColor1 = Color(100, 120, 255); //Blue
            bgBaseColor2 = Color(140, 120, 255); //Purple
        } break;
        case RED_ORANGE: {
            bgBaseColor1 = Color(255, 192, 192); //Red-ish
            bgBaseColor2 = Color(215, 125,  56); //Orange-red-ish
        } break;
        case WHITE_GRAY: {
            bgBaseColor1 = Color(220, 220, 220); //Light-gray
            bgBaseColor2 = Color(150, 150, 150); //gray
        } break;
        case GOLD_SILVER: {
            bgBaseColor1 = Color(200, 150,   0); //gold
            bgBaseColor2 = Color(180, 180, 180); //silver
            bgInterpolVal = 0.25;
        } break;
        case BLACK_DARKRED: {
            bgBaseColor1 = Color(  0,   0,   0); //Black
            bgBaseColor2 = Color( 50,   0,   0); //Dark red
            bgInterpolVal = 0.25;
        } break;

        default: {
            bgBaseColor1 = Color(  0,   0,   0);
            bgBaseColor2 = Color(255,   0, 255);
        } break;
    }
}

void Map::alloc(int width, int height)
{
    if(loaded) { Log::warn(__PRETTY_FUNCTION__, "doing nothing", "Map data already allocated"); return; }
    loaded = true;

    Map::width = width;
    Map::height = height;

    //Create Map
    tiles = new Tile*[width];
    for(int ix = 0; ix<width; ix++) {
        tiles[ix] = new Tile[height];
        for(int iy = 0; iy<height; iy++) {
            tiles[ix][iy] = 0;
        }
    }
}

void Map::free()
{
    if(!loaded) { Log::warn(__PRETTY_FUNCTION__, "doing nothing", "Map data already empty"); return; }
    loaded = false;

    //Destroy Map
    for(int ix = 0; ix<width; ix++)
        delete[] tiles[ix];
    delete[] tiles;

    //Reset width/height
    width = 8;
    height = 8;
}
