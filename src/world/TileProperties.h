#pragma once
#include <nch/cpp-utils/color.h>
#include <nch/sdl-utils/rect.h>
#include <map>
#include <unordered_map>

class TileProperties {
public:
    static void create();

    static nch::Rect getBoundsByID(int id);
    static nch::Rect getSrcByID(int id);
    static bool canCollidePlayerByID(int id);
    static bool canCollideCursorByID(int id);
    static bool getKillsPlayerByID(int id);
    static bool getKillsCursorByID(int id);
    static nch::Color getColorByID(int id);
    static int getIDByColor(nch::Color col);
    

private:
    static std::unordered_map<uint32_t, int> mapColorToID;
    static std::unordered_map<int, uint32_t> mapIDToColor;
};