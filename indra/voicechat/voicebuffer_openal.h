#ifndef VOICEBUFFER_OPENAL_H
#define VOICEBUFFER_OPENAL_H

#ifndef LL_FMOD

#include "AL/al.h"
#include "voicesource.h"
#include <vector>

class VoiceBuffer
{
public:
    VoiceBuffer(size_t mMaxBuffers);
	~VoiceBuffer();

    bool isPlaying() const;
    void play();
    void stop();
    void bufferData(ALuint format, unsigned samplerate, unsigned datasize, void* data);
    void update();
    void update3D(const VoiceSource& source);

private:
    ALuint mSource;
    std::vector<ALuint> mQueuedBuffers;
    size_t mMaxBuffers; 
	bool dropThisVoicePacket;

};	//	class VoiceBuffer

#endif

#endif	//	VOICEBUFFER_OPENAL_H
