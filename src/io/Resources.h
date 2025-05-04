#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <nch/cpp-utils/arraylist.h>
#include "AudioChunk.h"
#include "MMTexture.h"

class Resources {
public:
    enum ID {
    BEGIN_TEX,
        TEX_cursor,
        TEX_guis,
        TEX_logo,
        TEX_tileset,
        TEX_ootws,
    END_TEX,
    BEGIN_SFX,
        SFX_andgate_open_cursor,
        SFX_andgate_open_player,
        SFX_button,
        SFX_impact,
        SFX_lose,
        SFX_spawn,
        SFX_win,
    END_SFX,
    };

    Resources();
    ~Resources();

    static void reloadAll();
    static MMTexture* getTextureByID(int id);
    static AudioChunk* getSFXByID(int id);

    static TTF_Font* ttfBte100;
    static TTF_Font* ttfOswald16;
    static TTF_Font* ttfOswald48;
    static TTF_Font* ttfOswaldBold96;
private:
    static void unloadAll();

    static nch::ArrayList<MMTexture> textures;
    static nch::ArrayList<AudioChunk> sounds;
    static bool initialized;
};
