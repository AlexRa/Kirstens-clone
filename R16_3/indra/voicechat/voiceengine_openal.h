#ifndef VOICEENGINE_OPENAL_H
#define VOICEENGINE_OPENAL_H

#ifndef LL_FMOD

#include "AL/al.h"
#include "AL/alc.h"

#include "voiceformat.h"
#include "voicesource.h"
#include "voicenet.h"
#include "voiceuuid.h"

#include <apr-1/apr_thread_proc.h>
#include <apr-1/apr_thread_mutex.h>

#include <boost/pool/pool.hpp>
#include <boost/shared_ptr.hpp>

#include <map>

class VoiceBuffer;
class VoiceCodec;
struct VoicePacketHeader;

class LLAudioEngine_OpenAL;

/**
 *  A singleton class for handling voip communication
 */
class VoiceEngine
{
public:
    typedef boost::shared_ptr<VoiceCodec> CodecPtr;
	const int defaultServerPort;

    /**
     *  @return VoiceEngine singleton instance
     */
    static VoiceEngine* getInstance();

    /**
     *  @param audioEngine LLAudioEngine_OpenAL instance (must exist & be initialized for voicechat to work)
     *  @param networkChunkSize Size of a network chunk, i.e. the amount of data that will be buffered before
     *                          it's sent to the server. The size is specified in PCM samples.
     *  @param bufferFrameCount The number of frames to hold in record buffer (buffer sample count = frequency * frame count)
     *  @param frequency Frequency of audio to record (affects only the default SPEEX codec)
     */
    VoiceEngine(LLAudioEngine_OpenAL* audioEngine, unsigned int networkChunkSizePCM = 3000, unsigned int bufferFrameCount = 4,
                  unsigned int frequency = 8000); 

    ~VoiceEngine();

    /**
     *  Connects to a voice chat server
     *  @param host Voice server hostname
     *  @param port Voice server port
     *  @param id Avatar id
     *  @return TRUE if successfully connected, FALSE otherwise
     */
    bool connect(const char* host, int port, const VoiceUUID& id);

    /**
     *  Closes the connection to a voice server if connected
     */
    void disconnect();

    /**
     *  Updates the internal sound system. This should be called once
     *  per frame even if an external sound system is supplied.
     */
    void update();


    /**
     *  Starts recording audio through an input device. The recorded
     *  data will be automatically sent to the voice server.
     */
    void startRecording();

    /**
     *  Stops recording audio and sending data to the voice server.
     */
    void stopRecording();

    /**
     *  @return TRUE if the voice engine is currently recording audio or
     *          FALSE otherwise
     */
    bool isRecording() const;


    /**
     *  Adds a new voice codec to the engine that can be used to
     *  encode/decode incoming audio. If you want to encode outgoing
     *  audio using this codec you can use setRecordCodec.
     */
    void addCodec(CodecPtr& codec);

    /**
     *	Sets the codec used to encode outgoing audio data. A new
     *  recording buffer will be created with the preferences of
     *  the codec.
     *  @return TRUE on success, FALSE on failure
     */
    bool setRecordCodec(CodecPtr& codec);


    void updateListener(float* pos, float* dir, float* vel);
    void updateAvatar(const VoiceUUID& id, float* pos, float* dir, float* vel);

    void setEnabled(bool enabled);

private:
    int sendPacketHeader(char type, unsigned int size);
    int sendLogin(const VoiceUUID& id);
    int sendAudioFrames(unsigned int samples);
    int sendEnableVOIP(bool enabled);

    unsigned short encodeAudioFrame(unsigned int from, unsigned int to, char* buffer);

    int recvPacketHeader(char* type, unsigned int* size);
    int recvAudioFrames(unsigned int expectedSize);
    int recvVoiceHeader(VoicePacketHeader* header);
    int recvUUID(VoiceUUID* id);

    static void* APR_THREAD_FUNC receiveThread(apr_thread_t* thread, void* param);

    VoiceBuffer* newAvatar(const VoiceUUID& id, VoiceCodec* codec);
    void removeAvatar(const VoiceUUID& id);

    VoiceBuffer* getAvatarVoiceBuffer(const VoiceUUID& id) const;

private:
    static VoiceEngine* mInstance;

    LLAudioEngine_OpenAL* mAudioEngine;
    ALCdevice* mCaptureDevice;
    char* mCaptureBuffer;
    
    // Encoded audio format (i.e. codec id)
    int mEncodeFormat;

    // Audio frequency
    unsigned int mRecordSampleRate;

    // Audio sample size (in bytes)
    unsigned short mRecordSampleSize;

    // The number of frames to hold in record buffer
    unsigned int mBufferFrameCount;

    // The number of samples to record before sending data to the network
    unsigned int mNetworkChunkSize;

    // The position in the record buffer from where we last sent data (in PCM samples)
    unsigned int mLastRecordPos;

    bool mRecording;
    bool mRun;

    VESocketHandle mSock;

    apr_thread_t* mReceiveThread;
    apr_pool_t* mAprPool;

    typedef std::map<int, CodecPtr> CodecMap;
    CodecMap mCodecs;
    int mSupportedFormats;

    VoiceSource mSelfListener;

    apr_thread_mutex_t* mAvatarMutex;

    typedef std::map<VoiceUUID, VoiceBuffer*> BufferMap;
    BufferMap mBuffers;

    typedef std::map<VoiceUUID, VoiceSource> SourceMap;
    SourceMap mSources;

    // We need two pools because send/receive can happen at the same time
    boost::pool<> mReceivePool;
    boost::pool<> mSendPool;

    bool mEnabled;
    bool mInited;

	// silent voice packet filter
	double silenceSampleRateThreshold;  // (percentage from total sample count) if silence sample rate is at least this then voice packet is considered to be silence
	double silencePowerThreshold;  // (percentage from max value)
	int silentVoicePacketFilterTriggerTimeMs;
	int sequentialSilentTimeMs;
	bool silentVoicePacketFilterActived;

};  //  class VoiceEngine

#endif

#endif  //  VOICEENGINE_H
