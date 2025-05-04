#include "Resources.h"
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/timer.h>
#include <SDL2/SDL_image.h>
#include "Main.h"

using namespace nch;

bool Resources::initialized = false;
TTF_Font* Resources::ttfBte100;
TTF_Font* Resources::ttfOswald16;
TTF_Font* Resources::ttfOswald48;
TTF_Font* Resources::ttfOswaldBold96;
ArrayList<MMTexture> Resources::textures;
ArrayList<AudioChunk> Resources::sounds;

Resources::Resources() { reloadAll(); }
Resources::~Resources() { unloadAll(); }

void Resources::reloadAll()
{
    /* Prep */
    if(initialized) {
        unloadAll();
    }
    initialized = true;

    Timer t("Resource loading", true);
    SDL_Renderer* rend = Main::getRenderer();
    SDL_PixelFormat* pxFmt = Main::getPixelFormat();
    std::string bp = Main::getBasePath();

    /* Set window icon */
    SDL_Surface* ico = IMG_Load((bp+"res/icon.png").c_str());
    SDL_SetWindowIcon(Main::getWindow(), ico);
    SDL_FreeSurface(ico);

    /* Load resource files */
    //Fonts...
    ttfBte100 = TTF_OpenFont((bp+"res/fonts/BackToEarth.ttf").c_str(), 100);
    ttfOswald16 = TTF_OpenFont((bp+"res/fonts/Oswald.ttf").c_str(), 16);
    ttfOswald48 = TTF_OpenFont((bp+"res/fonts/Oswald.ttf").c_str(), 48);
    ttfOswaldBold96 = TTF_OpenFont((bp+"res/fonts/Oswald-Bold.ttf").c_str(), 96);
    //Textures...
    textures.pushBack(new MMTexture(rend, pxFmt, bp+"res/cursor.png"));
    textures.pushBack(new MMTexture(rend, pxFmt, bp+"res/guis.png"));
    textures.pushBack(new MMTexture(rend, pxFmt, bp+"res/logo.png"));
    textures.pushBack(new MMTexture(rend, pxFmt, bp+"res/tileset.png"));
    textures.pushBack(new MMTexture(rend, pxFmt, bp+"res/ootws.png"));
    //SFX...
    sounds.pushBack(new AudioChunk(bp+"res/audio/andgate_open_cursor.mp3"));
    sounds.pushBack(new AudioChunk(bp+"res/audio/andgate_open_player.mp3"));
    sounds.pushBack(new AudioChunk(bp+"res/audio/button.mp3"));
    sounds.pushBack(new AudioChunk(bp+"res/audio/impact.wav"));
    sounds.pushBack(new AudioChunk(bp+"res/audio/lose.mp3"));
    sounds.pushBack(new AudioChunk(bp+"res/audio/spawn.mp3"));
    sounds.pushBack(new AudioChunk(bp+"res/audio/win.mp3"));

    //Modify blend mode
    getTextureByID(TEX_cursor)->setBlendMode(SDL_BLENDMODE_BLEND);
}

MMTexture* Resources::getTextureByID(int id)
{
    id = id-BEGIN_TEX-1;
    if(id<0 || id>=textures.size()) {
        Log::warnv(__PRETTY_FUNCTION__, "returning nullptr", "No such texture exists for out of range 'id' of %d", id);
        return nullptr;
    }
    return &textures[id];
}

AudioChunk* Resources::getSFXByID(int id)
{
    id = id-BEGIN_SFX-1;
    if(id<0 || id>=sounds.size()) {
        Log::warnv(__PRETTY_FUNCTION__, "returning nullptr", "No such sound chunk exists for out of range 'id' of %d", id);
        return nullptr;
    }
    return &sounds[id];
}

void Resources::unloadAll()
{
    initialized = false;

    Timer t("Resource cleanup", true);

    /* Cleanup all resources */
    //Sounds...
    sounds.clear();
    //Fonts...
    TTF_CloseFont(ttfBte100);
    TTF_CloseFont(ttfOswald16);
    TTF_CloseFont(ttfOswald48);
    TTF_CloseFont(ttfOswaldBold96);
    //Images...
    textures.clear();
}
