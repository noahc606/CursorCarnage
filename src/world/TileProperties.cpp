#include "TileProperties.h"
#include "Tile.h"

using namespace nch;
typedef Tile::TileID t;

std::unordered_map<uint32_t, int> TileProperties::mapColorToID;
std::unordered_map<int, uint32_t> TileProperties::mapIDToColor;

void TileProperties::create()
{
    //Create (Color <-> ID) bimap
    mapColorToID = {
        std::make_pair(Color(255, 255, 255).getRGBA(), t::AIR),
        std::make_pair(Color( 90, 100,  25).getRGBA(), t::PLAYER_WALL),
        std::make_pair(Color(200, 215, 100).getRGBA(), t::PLAYER_WALL_MINI),
        std::make_pair(Color(199,  82,  82).getRGBA(), t::PLAYER_KILLER),
        std::make_pair(Color(218, 138, 138).getRGBA(), t::PLAYER_KILLER_MINI),
        std::make_pair(Color( 25, 176, 118).getRGBA(), t::CURSOR_WALL),
        std::make_pair(Color(  6, 255, 167).getRGBA(), t::CURSOR_WALL_MINI),
        std::make_pair(Color(118,  11,  25).getRGBA(), t::CURSOR_KILLER),
        std::make_pair(Color(176,  17,  28).getRGBA(), t::CURSOR_KILLER_MINI),
        std::make_pair(Color(  0,  30,  15).getRGBA(), t::OP_WALL),
        std::make_pair(Color( 25,  62,  48).getRGBA(), t::OP_WALL_MINI),
        std::make_pair(Color( 26,   0,   0).getRGBA(), t::OP_KILLER),
        std::make_pair(Color( 50,   0,   0).getRGBA(), t::OP_KILLER_MINI),
        std::make_pair(Color(  0, 115, 150).getRGBA(), t::AND_GATE),
        std::make_pair(Color(0,  0,  0,  1).getRGBA(), t::AND_GATE_OPEN),
        std::make_pair(Color(0,  0,  0,  1).getRGBA(), t::AND_GATE_PLAYER),
        std::make_pair(Color(0,  0,  0,  1).getRGBA(), t::AND_GATE_CURSOR),
        std::make_pair(Color(255, 255,   0).getRGBA(), t::SLOW_ZONE),


        std::make_pair(Color(255,   0, 255).getRGBA(), t::UNKNOWN),
        std::make_pair(Color(  0, 255,   0).getRGBA(), t::MAP_START),
        std::make_pair(Color(  0,   0, 255).getRGBA(), t::MAP_END),
        std::make_pair(Color(  0,   0,   0).getRGBA(), t::MAP_WALL),
    };
    for(auto elem : mapColorToID) {
        mapIDToColor.insert(std::make_pair(elem.second, elem.first));
    }
}

Rect TileProperties::getSrcByID(int id)
{
    switch(id) {
        case t::PLAYER_WALL:        return Rect(  0,   0, 64, 64);
        case t::PLAYER_WALL_MINI:   return Rect( 64,   0, 64, 64);
        case t::PLAYER_KILLER:      return Rect(192,   0, 64, 64);
        case t::PLAYER_KILLER_MINI: return Rect(128,   0, 64, 64);
        case t::CURSOR_WALL:        return Rect(  0,  64, 64, 64);
        case t::CURSOR_WALL_MINI:   return Rect( 64,  64, 64, 64);
        case t::CURSOR_KILLER:      return Rect(192,  64, 64, 64);
        case t::CURSOR_KILLER_MINI: return Rect(128,  64, 64, 64);
        case t::OP_WALL:            return Rect(  0, 128, 64, 64);
        case t::OP_WALL_MINI:       return Rect( 64, 128, 64, 64);
        case t::OP_KILLER:          return Rect(192, 128, 64, 64);
        case t::OP_KILLER_MINI:     return Rect(128, 128, 64, 64);
        case t::AND_GATE:           return Rect(  0, 192, 64, 64);
        case t::AND_GATE_OPEN:      return Rect(192, 192, 64, 64);
        case t::AND_GATE_PLAYER:    return Rect(128, 192, 64, 64);
        case t::AND_GATE_CURSOR:    return Rect( 64, 192, 64, 64);
    }
    return Rect(448, 0, 64, 64);
}

Rect TileProperties::getBoundsByID(int id)
{
    switch(id) {
        case t::PLAYER_WALL_MINI:
        case t::PLAYER_KILLER_MINI:
        case t::CURSOR_WALL_MINI:
        case t::CURSOR_KILLER_MINI:
        case t::OP_WALL_MINI:
        case t::OP_KILLER_MINI:
            return Rect(2, 2, 60, 60);
    }
    return Rect(0, 0, 64, 64);
}

bool TileProperties::canCollidePlayerByID(int id)
{
    //player collision
    switch(id) {
        case t::AIR:
        case t::CURSOR_WALL:
        case t::CURSOR_WALL_MINI:
        case t::CURSOR_KILLER:
        case t::CURSOR_KILLER_MINI:
        case t::MAP_START:
        case t::MAP_END:
        case t::AND_GATE_OPEN:
        case t::SLOW_ZONE:
            return false;
    }
    return true;
}

bool TileProperties::canCollideCursorByID(int id)
{
    switch(id) {
        case t::CURSOR_WALL:
        case t::CURSOR_WALL_MINI:
        case t::CURSOR_KILLER:
        case t::CURSOR_KILLER_MINI:
        case t::OP_WALL:
        case t::OP_WALL_MINI:
        case t::OP_KILLER:
        case t::OP_KILLER_MINI:
        case t::MAP_WALL:
        case t::AND_GATE:
        case t::AND_GATE_PLAYER:
        case t::AND_GATE_CURSOR:
            return true;
    }
    return false;
}

bool TileProperties::getKillsPlayerByID(int id)
{
    switch(id) {
        case t::PLAYER_KILLER:
        case t::PLAYER_KILLER_MINI:
        case t::OP_KILLER:
        case t::OP_KILLER_MINI:
            return true;
    }
    return false;
}
bool TileProperties::getKillsCursorByID(int id)
{
    switch(id) {
        case t::CURSOR_KILLER:
        case t::CURSOR_KILLER_MINI:
        case t::OP_KILLER:
        case t::OP_KILLER_MINI:
            return true;
    }
    return false;
}

nch::Color TileProperties::getColorByID(int id)
{
    auto mitr = mapIDToColor.find(id);
    if(mitr!=mapIDToColor.end()) {
        //AND_GATE states treated as air
        if(mitr->second==Color(0, 0, 0, 1).getRGBA()) { return getColorByID(t::AIR); }
        //All other IDs
        return nch::Color(mitr->second);
    }
    return getColorByID(t::UNKNOWN);
}
int TileProperties::getIDByColor(nch::Color col)
{
    auto mitr = mapColorToID.find(col.getRGBA());
    if(mitr!=mapColorToID.end()) {
        //AND_GATE states treated as air
        if(col==Color(0, 0, 0, 1).getRGBA()) { return t::AIR; }
        //All other colors
        return mitr->second;
    }
    return t::UNKNOWN;
}