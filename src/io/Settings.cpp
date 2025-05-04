#include "Settings.h"
#include <fstream>
#include <nch/cpp-utils/filepath.h>
#include <nch/cpp-utils/log.h>
#include <nlohmann/json.hpp>
#include "Main.h"

using namespace nch;

double Settings::mouseAccelerationFactor = 1.0;
double Settings::musicVolume = 1.0;
double Settings::soundVolume = 1.0;
bool Settings::invincible = false;

Settings::Settings()
{
    bool res = true;

    /* Settings file */
    FilePath fpSettings(Main::getBasePath()+"data/settings.json");
    nlohmann::json j;
    try {
        std::ifstream ifs(fpSettings.get());
        j = nlohmann::json::parse(ifs);
    } catch(...) {
        res = false;
    }

    /* Load individual settings */
    /* Mouse acceleration factor */ {
        try { mouseAccelerationFactor = j.at("mouseAccelerationFactor"); } catch(...) { res = false; }
    }
    /* Music volume */ {
        try { musicVolume = j.at("musicVolume"); } catch(...) { res = false; }
    }
    /* Sound volume */ {
        try { soundVolume = j.at("soundVolume"); } catch(...) { res = false; }
    }
    /* Invincible */ {
        try { invincible = j.at("invincible"); } catch(...) { res = false; }
    }

    /* Warnings */
    if(!res) {
        Log::warnv(__PRETTY_FUNCTION__, "falling back on default values", "Something went wrong while loading settings @ \"%s\"", fpSettings.get().c_str());
    }
}
Settings::~Settings(){}

double Settings::getMouseAccelerationFactor() {
    return mouseAccelerationFactor;
}
double Settings::getMusicVolume(){
    return musicVolume;
}
double Settings::getSoundVolume(){
    return soundVolume;
}
bool Settings::isInvincible(){
    return invincible;
}