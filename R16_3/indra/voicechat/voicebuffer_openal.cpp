#ifndef LL_FMOD

#include "linden_common.h"
#include "v3math.h"
#include "voicebuffer_openal.h"

VoiceBuffer::VoiceBuffer(size_t maxBuffers) :
    mSource(AL_NONE),
    mMaxBuffers(maxBuffers),
	dropThisVoicePacket(true)
{
    alGenSources(1, &mSource);     
}

VoiceBuffer::~VoiceBuffer()
{
    if (mSource != AL_NONE)
    {
        stop();

        // Forcibly unqueue & delete active buffers still remaining      
        for (unsigned i = 0; i < mQueuedBuffers.size(); i++)
        {
		    alSourceUnqueueBuffers(mSource, 1, &mQueuedBuffers[i]);
            alDeleteBuffers(1, &mQueuedBuffers[i]);
        }
        mQueuedBuffers.clear();            

        alDeleteSources(1, &mSource);
        mSource = AL_NONE;
    }
}

void VoiceBuffer::stop()
{
    if (mSource != AL_NONE)
    {
        alSourceStop(mSource);
    }
}

void VoiceBuffer::play()
{
	ALenum error;  

    if (mSource != AL_NONE)
    {
	    error = alGetError(); /* Clear error */
        alSourcePlay(mSource);
	    error = alGetError();                                                                                                           
 		if (error != AL_NO_ERROR)
        {   
            llwarns << "error starting playback" << llendl;
        }
    }
}

bool VoiceBuffer::isPlaying() const
{
    bool playing = false;

    if (mSource != AL_NONE)
    {
        ALint state;                                                                                                                                                            
	    alGetSourcei(mSource, AL_SOURCE_STATE, &state);                                                                                                                        
	    if(state == AL_PLAYING) playing = true;
    }

    return playing;    
}

void VoiceBuffer::update()
{
	ALenum error;  

    if (mSource == AL_NONE) return;

    // Unqueue & delete buffers that have already been played
    int processed = 0;                                                                                                                          
	error = alGetError(); /* Clear error */
    alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);                                                                        
		                                                                                                                                        
	while(processed--)                                                                                                                      
    {                                                                                                                                      
        ALuint buffer = AL_NONE;     

		alSourceUnqueueBuffers(mSource, 1, &buffer);
	    error = alGetError();                                                                                                           
 		if (error != AL_NO_ERROR)  
        {
            llwarns << "error unqueuing buffer" << llendl;
        }
        else
        {
            std::vector<ALuint>::iterator i = mQueuedBuffers.begin();
            while (i != mQueuedBuffers.end())
            {
                if ((*i) == buffer)
                {
                    mQueuedBuffers.erase(i);
                    break;
                }
                i++;
            }
            alDeleteBuffers(1, &buffer);            
        }        
    }

    // If buffers remain & not already playing, play now
    if ((mQueuedBuffers.size()) && (!isPlaying()))
        play();
}

void VoiceBuffer::bufferData(ALuint format, unsigned samplerate, unsigned datasize, void* data)
{
	ALenum error;  

    if (mSource == AL_NONE) return;
    if (!format) return;
    // If too much data queued for playback, drop this one
    if (mQueuedBuffers.size() >= mMaxBuffers)
    {
        llwarns << "dropping voice buffer, " << mQueuedBuffers.size() << " already in playback queue" << llendl;
        return;
    }
    
	error = alGetError(); /* Clear error */
    ALuint buffer = AL_NONE;
    alGenBuffers(1, &buffer);
    if (buffer != AL_NONE)
    {
        alBufferData(buffer, format, data, datasize, samplerate);
	    error = alGetError();                                                                                                           
 		if (error != AL_NO_ERROR)        
        {
            llwarns << "error setting bufferdata" << llendl;
            alDeleteBuffers(1, &buffer);
            return;
        }    

        alSourceQueueBuffers(mSource, 1, &buffer);      
	    error = alGetError();                                                                                                           
 		if (error != AL_NO_ERROR)   
        {
            llwarns << "error queuing buffer" << llendl;
            alDeleteBuffers(1, &buffer);
        }
        else
        {
            mQueuedBuffers.push_back(buffer); 
        }      
    }
    else
    {
        llwarns << "error creating voice buffer" << llendl;
    }
}

void VoiceBuffer::update3D(const VoiceSource& source)
{
    if (mSource == AL_NONE) return;

    LLVector3 pos;
    LLVector3 vel;

    source.getPosition(&pos.mV[0], &pos.mV[1], &pos.mV[2]);
    source.getVelocity(&vel.mV[0], &vel.mV[1], &vel.mV[2]);

	alSourcefv(mSource, AL_POSITION, pos.mV);                                                                                                                                                           
	alSourcefv(mSource, AL_VELOCITY, vel.mV);                                                                                           
	alSourcef (mSource, AL_ROLLOFF_FACTOR, 1.0);                                                                                                                   
	alSourcei (mSource, AL_SOURCE_RELATIVE, AL_FALSE);        
    // Boost voice a "bit"
	alSourcef(mSource, AL_GAIN, 2.0);   
}


#endif