#include "Player.h"
#include <nch/cpp-utils/color.h>
#include <nch/sdl-utils/main-loop-driver.h>
#include "Main.h"
#include "Map.h"
#include "Resources.h"
#include "Settings.h"

using namespace nch;

Player::Player(){
    vel = Vec2<double>(0, 0.1);
    updateBounds();
}
Player::~Player(){}

void Player::tick()
{
    updateBounds();



    if(!isDead()) {
        if(health<100) {
            numTicksHealthy = 0;
            numTicksUnhealthy++;
    
            if(MainLoopDriver::getNumTicksPassedTotal()-lastDamageTick>10)
                heal(0.1);
        } else {
            numTicksHealthy++;
            numTicksUnhealthy = 0;
            health = 100;
        }
    } else {
        health = -1000;
    }
}

void Player::draw(Screen& scr)
{
    SDL_Renderer* rend = scr.getRenderer();

    //Rectangle
    Rect dst(pos.x-48, pos.y-48, size, size);
    scr.transformRect(dst);

    Rect src(256, 0, 96, 96);
    if(MainLoopDriver::getNumTicksPassedTotal()-lastDamageTick<2) { src = Rect(352, 0, 96, 96); }
    if(isDead()) { src = Rect(256, 96, 96, 96); }
    MMTexture* tileset = Resources::getTextureByID(Resources::ID::TEX_tileset);
    if(tileset!=nullptr) tileset->renderCopy(scr.getRenderer(), &src.r, &dst.r);

    //Bounding boxes
    if(Main::isDebugging()) {
        Color c;
        for(int i = Collision::NORTH; i<=Collision::WEST; i++) {
            Rect r = Rect::createFromTwoPts(bounds[i].c1.x, bounds[i].c1.y, bounds[i].c2.x, bounds[i].c2.y);
            r.scale(scr.getScale());
            r.translate(scr.getBounds().c1.x, scr.getBounds().c1.y);

            c.setFromHSV(i*72, 100, 100);
            SDL_SetRenderDrawBlendMode(scr.getRenderer(), SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(scr.getRenderer(), c.r, c.g, c.b, 150);
            SDL_RenderFillRect(scr.getRenderer(), &r.r);
        }
    }

    /* Health bar */
    //Alpha value
    int hbAlpha = 0;
    if(numTicksUnhealthy>0) hbAlpha = 255;
    if(numTicksHealthy>0)   hbAlpha = 255;
    if(numTicksHealthy>60)  hbAlpha = 255-(numTicksHealthy-60)*8;
    if(hbAlpha<0) hbAlpha = 0;
    if(hbAlpha>255) hbAlpha = 255;
    //Background rectangle
    Rect hb0Dst(pos.x-size/2+8, pos.y-size/2-16, size-16, 10);
    //Foreground (red) rectangle
    Rect hb1Dst = hb0Dst;
    hb1Dst.r.x += 2; hb1Dst.r.y += 2;
    hb1Dst.r.w -= 4; hb1Dst.r.h -= 4;
    //Foreground (green) rectangle
    Rect hb2Dst = hb1Dst;
    hb2Dst.r.w = health*hb1Dst.r.w/100.0;
    if(hb2Dst.r.w<0) hb2Dst.r.w = 0;
    //Transform + Draw rectangles
    scr.transformRect(hb0Dst);
    scr.transformRect(hb1Dst);
    scr.transformRect(hb2Dst);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, hbAlpha);
    SDL_RenderFillRect(rend, &hb0Dst.r);
    SDL_SetRenderDrawColor(rend, 255, 0, 0, hbAlpha);
    SDL_RenderFillRect(rend, &hb1Dst.r);
    SDL_SetRenderDrawColor(rend, 0, 255, 0, hbAlpha);
    SDL_RenderFillRect(rend, &hb2Dst.r);
}

Vec2<double> Player::getPos() {
    return pos;
}
Box2<double> Player::getBounds(Collision::Direction dir) {
    return bounds[dir];
}
Box2<double> Player::getBounds() {
    return getBounds(Collision::Direction::NONE);
}
bool Player::isDead() {
    return health<=0;
}
int Player::getZoneType() {
    return zoneType;
}

void Player::setPos(const Vec2<double>& pos) {
    Player::pos = pos;
}
void Player::setVel(const Vec2<double>& vel) {
    Player::vel = vel;
}

void Player::processVelocity(std::vector<std::pair<nch::Vec2<int>, Tile>>& collidingTiles)
{
    zoneType = 0;
    bool slowdown = false;

    //Find unit velocity vector such that its length is less than 1
    for(int u = 0; u<(std::ceil(vel.length())); u++) {
        pos += vel/(std::ceil(vel.length()));
        
        for(int i = 0; i<collidingTiles.size(); i++) {
            Vec2i tilePos = collidingTiles[i].first;
            Tile tile = collidingTiles[i].second;
            Box2<double> tileArea = tile.getWorldBounds(tilePos);

            if(tileArea.intersects(getBounds())) {
                if(tile.getKillsPlayer()) damage(1000);
                switch(tile.getID()) {
                    case Tile::MAP_START:       zoneType = 1; break;
                    case Tile::MAP_END:         zoneType = 2; break;
                    case Tile::SLOW_ZONE:       slowdown = true; break;
                }
            }

            if(slowdown) {
                if(vel.x>=2)  vel.x = 2;
                if(vel.x<=-2) vel.x = -2;
                if(vel.y>=2)  vel.y = 2;
                if(vel.y<=-2) vel.y = -2;
            }

            if(tile.canCollideWithPlayer())
            for(int j = 0; j<Collision::dirs.size(); j++) {
                if(tileArea.intersects(getBounds(Collision::dirs[j]))) {
                    Collision::snapRectByDir(Collision::dirs[j], tilePos, tile, size, pos, vel);
                    updateBounds();
                }
            }
        }

        updateBounds();
    }
}

void Player::damage(double val)
{
    if(!Settings::isInvincible())
        health -= val;
    lastDamageTick = MainLoopDriver::getNumTicksPassedTotal();
}
void Player::heal(double val)
{
    health += val;
}

void Player::updateBounds()
{
    Vec2d tl(pos.x-size/2, pos.y-size/2);    //top left
    bounds[0]                = Box2<double>::createFromXYWH(tl.x, tl.y,                size,   size);   //Main bounding box
    bounds[Collision::NORTH] = Box2<double>::createFromXYWH(tl.x+2, tl.y-1,            size-4, size/4); //Northern bounding box
    bounds[Collision::EAST]  = Box2<double>::createFromXYWH(tl.x-1, tl.y+2,            size/4, size-4); //...
    bounds[Collision::SOUTH] = Box2<double>::createFromXYWH(tl.x+2, tl.y+3*size/4+1,   size-4, size/4); //...
    bounds[Collision::WEST]  = Box2<double>::createFromXYWH(tl.x+3*size/4+1, tl.y+2,   size/4, size-4); //...
}

void Player::updateZoneType(int newZone)
{
    if(newZone>zoneType) {
        newZone = zoneType;
    }
}
