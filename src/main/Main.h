#pragma once
#include <cstdint>
#include <nch/sdl-utils/text.h>
#include <SDL2/SDL.h>
#include <string>
#include "CC.h"

class Main {
public:
    Main();
    ~Main();

    static void tick();
    static void draw(SDL_Renderer* rend);
    static void events(SDL_Event& e);

	static int getWidth();
	static int getHeight();
	static SDL_Window* getWindow();
    static SDL_Renderer* getRenderer();
    static SDL_PixelFormat* getPixelFormat();
    static std::string getBasePath();
    static bool isDebugging();
    static bool isFullscreen();
private:
    static SDL_Window* win;
    static SDL_Renderer* rend;
    static SDL_PixelFormat* pxFmt;
    static std::string basePath;

    static bool fullscreen;
    static bool db;
    static nch::Text debugText;
    static CC* cc;
    static bool needToReload;
    static uint64_t lastReloadTimeMS;
};