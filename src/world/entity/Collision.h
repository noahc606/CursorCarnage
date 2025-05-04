#pragma once
#include <nch/math-utils/vec2.h>
#include <vector>
#include "Tile.h"

class Collision {
public:
    enum Direction {
        NONE, NORTH, EAST, SOUTH, WEST
    };

    template<typename T> static void snapRectByDir(Direction dir, const nch::Vec2<int>& tilePos, const Tile& tile, double objSize, nch::Vec2<T>& objPos, nch::Vec2<T>& objVel) {
        switch(dir) {
            case NORTH: {
                if(objVel.y<0) {
                    double ySnap = 64*tilePos.y+tile.getBounds().c2.y+objSize/2;
                    if(std::abs(objPos.y-ySnap)<=4) {
                        objVel.y = 0; objPos.y = ySnap;
                    }
                }
            } break;
            case EAST: {
                if(objVel.x<0) {
                    double xSnap = 64*tilePos.x+tile.getBounds().c2.x+objSize/2;
                    if(std::abs(objPos.x-xSnap)<=4) {
                        objVel.x = 0; objPos.x = xSnap;
                    }
                }
            } break;
            case SOUTH: {
                if(objVel.y>0) {
                    double ySnap = 64*tilePos.y+tile.getBounds().c1.y-objSize/2;
                    if(std::abs(objPos.y-ySnap)<=4) {
                        objVel.y = 0; objPos.y = ySnap;
                    }
                }
            } break;
            case WEST: {
                if(objVel.x>0) {
                    double xSnap = 64*tilePos.x+tile.getBounds().c1.x-objSize/2;
                    if(std::abs(objPos.x-xSnap)<=4) {
                        objVel.x = 0; objPos.x = xSnap;
                    }
                }
            } break;
            default: {} break;
        }
    }
    template<typename T> static void snapPoint(const nch::Vec2<int>& tilePos, const Tile& tile, nch::Vec2<T>& objPos, nch::Vec2<T>& objVel) {
        snapRectByDir(NORTH, tilePos, tile, 2, objPos, objVel);
        snapRectByDir(SOUTH, tilePos, tile, 2, objPos, objVel);
        snapRectByDir(EAST,  tilePos, tile, 2, objPos, objVel);
        snapRectByDir(WEST,  tilePos, tile, 2, objPos, objVel);
    }

    static const std::vector<Direction> dirs;

private:

};
