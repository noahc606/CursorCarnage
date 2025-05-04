#pragma once
#include <SDL2/SDL_mixer.h>
#include <string>

class AudioChunk {
public:
    AudioChunk(std::string resourcePath);
    ~AudioChunk();

    void playback();

private:
    Mix_Chunk* mc = nullptr;
};