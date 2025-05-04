#pragma once

class Settings {
public:
    Settings();
    ~Settings();

    static double getMouseAccelerationFactor();
    static double getMusicVolume();
    static double getSoundVolume();
    static bool isInvincible();
private:
    static double mouseAccelerationFactor;
    static double musicVolume;
    static double soundVolume;
    static bool invincible;
};