#include "Cursor.h"
#include <nch/sdl-utils/input.h>
#include "Main.h"
#include "Resources.h"
#include "Settings.h"

using namespace nch;

Cursor::Cursor(){}
Cursor::~Cursor(){}

void Cursor::draw(Screen& scr)
{
    if(lockTicks>0) lockTicks--;
    if(dead) return;

    Rect dst(pos.x, pos.y, 12, 18);
    dst.r.x *= scr.getScale();
    dst.r.y *= scr.getScale();
    dst.translate(scr.getBounds().c1.x, scr.getBounds().c1.y);
    MMTexture* crsr = Resources::getTextureByID(Resources::ID::TEX_cursor);
    if(crsr!=nullptr) crsr->renderCopy(scr.getRenderer(), NULL, &dst.r);
}

void Cursor::updatePos(Map& map)
{
    int mw = Main::getWidth();
    int mh = Main::getHeight();

    //Get mouse movement for fake cursor ('mouse' = real cursor, 'cursor' = fake cursor)
    Vec2<double> dpos;
    Vec2i mousePos(Input::getMouseX(), Input::getMouseY());
    if(mousePos.x!=mw/2 || mousePos.y!=mh/2) {
        SDL_WarpMouseInWindow(Main::getWindow(), mw/2, mh/2);

        Vec2i lastMousePos = mousePos;
        mousePos = Vec2i(mw/2, mh/2);
        dpos = (lastMousePos-mousePos).toDouble()*Settings::getMouseAccelerationFactor();
    }

    if(lockTicks>0 || dead) return;
    if(dpos.length()!=0) {
        started = true;
    }

    //Process movement and collision for cursor
    Box2i area = map.getTileAreaByWorldArea(Box2<double>(pos.x-64, pos.y-64, pos.x+64, pos.y+64));
    std::vector<std::pair<Vec2<int>, Tile>> collidingTiles; collidingTiles.reserve(25);
    for(int ix = area.c1.x; ix<=area.c2.x; ix++)
    for(int iy = area.c1.y; iy<=area.c2.y; iy++) {
        Tile t = map.getTile(ix, iy);
        if(!t.canCollideWithCursor()) continue; 
        collidingTiles.push_back(std::make_pair(Vec2i(ix, iy), t));
    }
    processMovement(collidingTiles, dpos);

    //Final clipping
    Box2<double> mb = map.getBounds();
    if(pos.x< mb.c1.x) pos.x = 0;
    if(pos.x>=mb.c2.x) pos.x = mb.c2.x-1;
    if(pos.y< mb.c1.y) pos.y = 0;
    if(pos.y>=mb.c2.y) pos.y = mb.c2.y-1;
}

Vec2<int> Cursor::getPos() { return pos; }
bool Cursor::isDead() {
    return dead;
}
bool Cursor::hasStartedMoving() {
    return started;
}

void Cursor::processMovement(std::vector<std::pair<Vec2<int>, Tile>>& collidingTiles, Vec2<double>& dpos)
{
    Vec2<double> posCopy = pos.toDouble();

    //Find unit velocity vector such that its length is less than 1
    for(int u = 0; u<(8*dpos.length()); u++) {
        posCopy += dpos/(8*dpos.length());
        
        for(int i = 0; i<collidingTiles.size(); i++) {
            Vec2i tilePos = collidingTiles[i].first;
            Tile tile = collidingTiles[i].second;
            Box2<double> tileArea = tile.getWorldBounds(tilePos);

            if(tileArea.contains(posCopy)) {
                if(tile.getKillsCursor()) kill();
                Collision::snapPoint(tilePos, tile, posCopy, dpos);
            }
        }
    }

    pos = posCopy.toInt();
}

void Cursor::setNotStarted() {
    started = false;
}

void Cursor::templock() {
    lockTicks = 20;
}
void Cursor::setPos(const Vec2<int>& pos) {
    templock();
    Cursor::pos = pos;
}

void Cursor::kill()
{
    if(Settings::isInvincible() || dead) return;
    dead = true;
    Resources::getSFXByID(Resources::SFX_lose)->playback();
}
