#ifndef VOICEBUFFER_H
#define VOICEBUFFER_H

#ifdef LL_FMOD

namespace FMOD
{
    class Channel;
    class Sound;
    class System;
}

class VoiceBuffer
{
public:
    VoiceBuffer(FMOD::Sound* sound);
	~VoiceBuffer();

    FMOD::Sound* getSound() const;
    FMOD::Channel* getChannel();

    bool isPlaying() const;
    void play(FMOD::System* system);
    void stop();

    void setLastPosition(unsigned int pos);
    unsigned int getLastPosition() const;

private:
    FMOD::Sound* mSound;
    unsigned int mLastPosition;
    FMOD::Channel* mChannel;

};	//	class VoiceBuffer

#endif

#endif	//	VOICEBUFFER_H
