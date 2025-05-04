#include "Tile.h"
#include <nch/sdl-utils/rect.h>
#include "Resources.h"
#include "TileProperties.h"

using namespace nch;

Tile::Tile(){}
Tile::Tile(int id) {
    Tile::id = id;
}
Tile::~Tile(){}

void Tile::draw(SDL_Renderer* rend, Rect& dst)
{
    if(id>=99 || id<=0) return;
    SDL_Rect src = TileProperties::getSrcByID(id).r;

    MMTexture* tileset = Resources::getTextureByID(Resources::ID::TEX_tileset);
    if(tileset!=nullptr) tileset->renderCopy(rend, &src, &dst.r);
}

int Tile::getID() {
    return id;
}
Box2<double> Tile::getBounds() const {
    Rect bounds = TileProperties::getBoundsByID(id);
    return Box2<double>(bounds.r.x, bounds.r.y, bounds.x2(), bounds.y2());
}
Box2<double> Tile::getWorldBounds(Vec2<int> tilePos) const {
    Box2<double> b = getBounds();
    return Box2<double>(
        tilePos.x*64+b.c1.x,
        tilePos.y*64+b.c1.y,
        tilePos.x*64+b.c2.x,
        tilePos.y*64+b.c2.y
    );
}

bool Tile::canCollideWithPlayer() {
    return TileProperties::canCollidePlayerByID(id);
}
bool Tile::canCollideWithCursor() {
    return TileProperties::canCollideCursorByID(id);
}
bool Tile::getKillsPlayer() {
    return TileProperties::getKillsPlayerByID(id);
}
bool Tile::getKillsCursor() {
    return TileProperties::getKillsCursorByID(id);
}
Color Tile::getColor() {
    return TileProperties::getColorByID(id);
}