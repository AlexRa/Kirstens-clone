#ifdef LL_FMOD

#include "voicebuffer.h"
#include <fmodex/fmod.hpp>


VoiceBuffer::VoiceBuffer(FMOD::Sound* sound)
    : mSound(sound)
    , mLastPosition(0)
    , mChannel(0)
{
}

VoiceBuffer::~VoiceBuffer()
{
    if (mSound)
    {
        mSound->release();
        mSound = 0;
    }

    mChannel = 0;
}

FMOD::Sound* VoiceBuffer::getSound() const
{
    return mSound;
}

FMOD::Channel* VoiceBuffer::getChannel()
{
    return mChannel;
}

void VoiceBuffer::setLastPosition(unsigned int pos)
{
    mLastPosition = pos;
}

unsigned int VoiceBuffer::getLastPosition() const
{
    return mLastPosition;
}

void VoiceBuffer::play(FMOD::System* system)
{
    system->playSound(FMOD_CHANNEL_FREE, mSound, false, &mChannel);
}

void VoiceBuffer::stop()
{
    mLastPosition = 0;

    if (mChannel)
    {
        mChannel->stop();
        mChannel = 0;
    }
}

bool VoiceBuffer::isPlaying() const
{
    bool playing = false;
    if (mChannel)
        mChannel->isPlaying(&playing);

    return playing;
}

#endif