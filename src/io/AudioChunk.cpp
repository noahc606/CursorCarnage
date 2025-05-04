#include "AudioChunk.h"
#include <nch/cpp-utils/log.h>
#include "Settings.h"

using namespace nch;

AudioChunk::AudioChunk(std::string resourcePath)
{
    mc = Mix_LoadWAV(resourcePath.c_str());
    if(mc==NULL) {
        Log::errorv(__PRETTY_FUNCTION__, Mix_GetError(), "Failed to Mix_LoadWAV resource @ \"%s\"", resourcePath.c_str());
    }
}
AudioChunk::~AudioChunk()
{
    if(mc!=nullptr) {
        Mix_FreeChunk(mc);
        mc = nullptr;
    }
}

void AudioChunk::playback()
{
    if(mc==nullptr) return;
	Mix_VolumeChunk(mc, Settings::getSoundVolume()*(double)MIX_MAX_VOLUME);
	Mix_PlayChannelTimed(-1, mc, 0, -1);
}