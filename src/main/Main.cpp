#include <nch/cpp-utils/fs-utils.h>
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/input.h>
#include <nch/sdl-utils/main-loop-driver.h>
#include <SDL2/SDL_mixer.h>
#include "CC.h"
#include "Main.h"
#include "Resources.h"
#include "Settings.h"

using namespace nch;

SDL_Window* Main::win = nullptr;
SDL_Renderer* Main::rend = nullptr;
SDL_PixelFormat* Main::pxFmt = nullptr;
std::string Main::basePath = "???null???";
bool Main::db = false;
bool Main::fullscreen = false;
Text Main::debugText;
CC* Main::cc;
bool Main::needToReload = false;
uint64_t Main::lastReloadTimeMS = 0;

Main::Main()
{
    /* Initialization */
    //SDL, SDL_ttf, SDL_mixer
    SDL_Init(0);
    TTF_Init();
    Mix_Init(MIX_INIT_MP3);
	int frequency = 48000;
	int channels = 2;
	if( Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, channels, 1024)==-1 ) {
		Log::warn(__PRETTY_FUNCTION__, "SDL_mixer failed to open.");
	}
    Mix_AllocateChannels(32);

    //Window, renderer, base path, pixel format
    win = SDL_CreateWindow("CursorCarnage", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 640, SDL_WINDOW_RESIZABLE);
    rend = SDL_CreateRenderer(win, -1, 0);
    #ifdef EMSCRIPTEN
        basePath = "/bin/";
    #else
        basePath = SDL_GetBasePath();
    #endif
    pxFmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    Mix_Music* mm = nullptr;
    /**/
    
    /* Game */ {
    //Resources
    Settings s;
    Resources res;

    FsUtils::ListSettings lise;
    FsUtils::RecursionSettings rese;
    rese.recursiveSearch = true;
    auto gdc = FsUtils::getDirContents(basePath, lise, rese);
    for(std::string s : gdc) {
        printf("%s\n", s.c_str());
    }
    
    //Mix_Music
    mm = Mix_LoadMUS((basePath+"res/music/Astropilot - Distant Worlds.mp3").c_str());
    if(mm==NULL) {
        Log::errorv(__PRETTY_FUNCTION__, Mix_GetError(), "Failed to load music");
    } else {
        Mix_PlayMusic(mm, -1);

        int vol = Settings::getMusicVolume()*(double)MIX_MAX_VOLUME;
        Mix_VolumeMusic(vol);
    }
    //Stuff
    debugText.init(rend, Resources::ttfOswald16);
    debugText.setShadowRelPos(1, 1);

    //World and main loop
    cc = new CC();
    MainLoopDriver mld(rend, tick, 40, draw, 1000, events);
    /**/ }

    /* Cleanup then quit */
    delete cc;
    SDL_FreeFormat(pxFmt);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    Mix_FreeMusic(mm);
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
    /**/
}
Main::~Main(){}

int main(int argc, char** args)
{ Main m; return 0; }
#if ( defined(_WIN32) || defined(WIN32) )
int WINAPI WinMain()
{ Main m; return 0; }
#endif

void Main::tick()
{
    if(needToReload && Timer::getTicks()>lastReloadTimeMS+250) {
        needToReload = false;
        lastReloadTimeMS = Timer::getTicks();
        cc->reloadResources();
    }

    if(Input::keyDownTime(SDLK_F12)==1) {
        db = !db;
    }
    if(Input::keyDownTime(SDLK_F11)==1) {
        fullscreen = !fullscreen;
        if(fullscreen) {
            SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else {
            SDL_SetWindowFullscreen(win, 0);
        }
    }

    cc->tick();

    if(cc->getGamestate()==CC::GameState::QUIT) {
        MainLoopDriver::quit();
    }
}

void Main::draw(SDL_Renderer* rend)
{
    /* Fill screen with black */
    SDL_RenderClear(rend);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderFillRect(rend, NULL);
    /**/

    /* Game content */
    cc->draw();
    if(db) {
        Resources::getTextureByID(Resources::ID::TEX_tileset)->drawMipmap(rend, getWidth(), getHeight());
        
        {
            std::stringstream ss;
            ss << MainLoopDriver::getPerformanceInfo();
            
            Color c; c.setFromHSV((Timer::getTicks()/10)%360, 100, 30);
            debugText.setShadowCustomColor(c);
            debugText.setText(ss.str());
            debugText.draw(0, 0);
        }

    }
    cc->drawCursor();
    /**/

    /* Render present objects */
    SDL_RenderPresent(rend);
    /**/
}

void Main::events(SDL_Event& e)
{
    switch(e.type) {
        case SDL_RENDER_TARGETS_RESET:
            needToReload = true; 
        break;
    }
}

int Main::getWidth() {
    int res; SDL_GetWindowSize(win, &res, NULL); return res;
}
int Main::getHeight() {
    int res; SDL_GetWindowSize(win, NULL, &res); return res;
}
SDL_Window* Main::getWindow() {
    return win;
}
SDL_Renderer* Main::getRenderer() {
    return rend;
}
SDL_PixelFormat* Main::getPixelFormat() {
    return pxFmt;
}
std::string Main::getBasePath() {
    return basePath;
}
bool Main::isDebugging() {
    return db;
}
bool Main::isFullscreen() {
    return fullscreen;
}
