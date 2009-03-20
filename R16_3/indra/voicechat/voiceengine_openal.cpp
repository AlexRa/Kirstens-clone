// ----------------------------------------------------------------------------
//
// TODO:
//  - Add support for multiple servers at the same time (and find out to which
//    one the local avatar belongs)
//  - Maybe VoiceBuffer and VoiceSource should be merged?
//  - Simplify avatar position updates (get rid of per avatar mutex locks etc.)
//  - Add VAD & DTX support
//
// ----------------------------------------------------------------------------

#ifndef LL_FMOD

#include "linden_common.h"
#include "audioengine_openal.h"
#include "voiceengine_openal.h"
#include "voicecodec.h"
#include "voiceheader.h"
#include "voicebuffer_openal.h"
#include "voicecodecspeex.h"
#include <boost/pool/singleton_pool.hpp>

// ----------------------------------------------------------------------------
// Helper functions

int voiceFormatToOpenAL(VoiceFormat format)
{
    switch(format)
    {
        case VF_PCM8: return AL_FORMAT_MONO8;
        case VF_PCM16: return AL_FORMAT_MONO16;
    }

    return 0;
}

// ----------------------------------------------------------------------------
// Voice engine implementation

VoiceEngine* VoiceEngine::mInstance = 0;

typedef boost::singleton_pool<VoiceBuffer, sizeof(VoiceBuffer)> VoiceBufferPool;
typedef boost::singleton_pool<VoiceFrameHeader, sizeof(VoiceFrameHeader)> FrameHeaderPool;


VoiceEngine* VoiceEngine::getInstance()
{
    return mInstance;
}

VoiceEngine::VoiceEngine(LLAudioEngine_OpenAL* audioEngine, unsigned int networkChunkSizePCM,
                             unsigned int bufferFrameCount, unsigned int frequency) :
      mAudioEngine(audioEngine)
    , mCaptureDevice(0)
    , mCaptureBuffer(0)
    , mRecordSampleRate(frequency)
    , mBufferFrameCount(bufferFrameCount)
    , mRecordSampleSize(0)
    , mRecording(false)
    , mNetworkChunkSize(networkChunkSizePCM)
    , mRun(true)
    , mSock(VE_INVALID_SOCKET)
    , mReceiveThread(0)
    , mAprPool(0)
    , mSupportedFormats(VF_PCM8 | VF_PCM16 | VF_PCM32)
    , mReceivePool(1, 40960)
    , mSendPool(1, 40960)
    , mEnabled(true)
    , mInited(false)
    , mAvatarMutex(0)
	, silenceSampleRateThreshold(0.95)
	, silencePowerThreshold(0.05)
	, sequentialSilentTimeMs(0) 
	, silentVoicePacketFilterTriggerTimeMs(3000)
	, silentVoicePacketFilterActived(false)
	, defaultServerPort(12000)
{
    if (!mAudioEngine)
    {
        llwarns << "No OpenAL audioengine, unable to initialize voicechat" << llendl;
        return;
    }
    if (!mAudioEngine->isInited())
    {
        llwarns << "OpenAL audioengine not initialized, unable to initialize voicechat" << llendl;
        return;
    }

    CodecPtr speexCodec(new VoiceCodecSpeex(frequency));
    if (!setRecordCodec(speexCodec))
        return;

    apr_pool_create(&mAprPool, NULL);
    apr_thread_mutex_create(&mAvatarMutex, APR_THREAD_MUTEX_DEFAULT, mAprPool);

    mInited = true;
    mInstance = this;
}

VoiceEngine::~VoiceEngine()
{
    disconnect();

    if (mCaptureDevice)
    {
        alcCaptureCloseDevice(mCaptureDevice);
        mCaptureDevice = 0;
    }

    if (mCaptureBuffer)
    {
        free(mCaptureBuffer);
        mCaptureBuffer = 0;
    }

    for (BufferMap::iterator i = mBuffers.begin(); i != mBuffers.end(); ++i)
    {
        if (i->second)
        {
            i->second->~VoiceBuffer();
            VoiceBufferPool::free(i->second);
        }
    }

    if (mAvatarMutex) apr_thread_mutex_destroy(mAvatarMutex);
    mAvatarMutex = 0;

    if (mAprPool) apr_pool_destroy(mAprPool);
    mAprPool = 0;

    mInstance = 0;
}

// -----------------------------------------------------------------------------
// Codec methods

void VoiceEngine::addCodec(CodecPtr& codec)
{
    assert(codec.px != 0);
    int format = codec->getEncodeFormat();
    mCodecs.insert(std::make_pair(format, codec));
    mSupportedFormats |= format;
}

bool VoiceEngine::setRecordCodec(CodecPtr& codec)
{
    CodecMap::const_iterator i = mCodecs.find(codec->getEncodeFormat());
    if (i == mCodecs.end())
    {
        addCodec(codec);
        i = mCodecs.find(codec->getEncodeFormat());
        if (i == mCodecs.end())
        {
            llwarns << "Error adding new voice codec!" << llendl;
            return false;
        }
    }

    mEncodeFormat = i->second->getEncodeFormat();
    VoiceFormat format = i->second->getDecodeFormat();

    mRecordSampleSize = sampleSizeFromVoiceFormat(format);
    mRecordSampleRate = i->second->getSampleRate();

    int openALFormat = voiceFormatToOpenAL(format);
    if (!openALFormat)
    {
        llwarns << "Unsupported record sample size" << llendl;
        return false;
    }

    mCaptureBuffer = (char *)malloc(mNetworkChunkSize * mBufferFrameCount * mRecordSampleSize);
    if (!mCaptureBuffer)
    {
        llwarns << "Failed to allocate recording buffer" << llendl;
        return false;
    }

    const char* captureDeviceName = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
    if (!captureDeviceName)
    {
        llwarns << "No OpenAL default recording device" << llendl;
        return false;
    }

    mCaptureDevice = alcCaptureOpenDevice(captureDeviceName, mRecordSampleRate, openALFormat, mNetworkChunkSize * mBufferFrameCount);    
    if (!mCaptureDevice)
    {
        llwarns << "Failed to open sound recording device" << llendl;
        return false;  
    }
    
    return true;
}

unsigned short VoiceEngine::encodeAudioFrame(unsigned int from, unsigned int to, char* buffer)
{
    if (to > from)
    {
        CodecMap::const_iterator i = mCodecs.find(mEncodeFormat);
        if (i != mCodecs.end())
        {
            unsigned int len = (to-from)*mRecordSampleSize;
            return i->second->encode((const char*)(mCaptureBuffer + from*mRecordSampleSize), len, buffer, len);
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------
// Recording methods

void VoiceEngine::startRecording()
{
    if (!mEnabled) return;

    if (mCaptureDevice)
    {
        alcCaptureStart(mCaptureDevice);     
        mRecording = true;
    }
}

void VoiceEngine::stopRecording()
{
    if (!mEnabled) return;

    if (mCaptureDevice)
    {
        alcCaptureStop(mCaptureDevice);     
        mRecording = false;
    }
}

bool VoiceEngine::isRecording() const
{
    return mRecording;
}

// -----------------------------------------------------------------------------
// Update methods

void VoiceEngine::update()
{
    if (!mInited) return;

    // Update avatar voices
//     if (apr_thread_mutex_trylock(mAvatarMutex) != APR_EBUSY)
    {
        apr_thread_mutex_lock(mAvatarMutex);
        for (BufferMap::const_iterator i = mBuffers.begin(); i != mBuffers.end(); ++i)
        {
            // Update buffer's 3D position
            bool playing = i->second->isPlaying();
            if (playing)
            {
                SourceMap::const_iterator j = mSources.find(i->first);
                if (j != mSources.end())
                {
                    const VoiceSource& source = j->second;    
                    i->second->update3D(source);
                }
            }
            // Unqueue played buffers, start playback of new buffers if necessary
            i->second->update(); 
        }

        apr_thread_mutex_unlock(mAvatarMutex);
    }

    if (!isRecording() && !mEnabled)
        return;

    if (mCaptureDevice)
    {
        // Send/encode samples if we have enough
	    ALCint samples;
	    alcGetIntegerv(mCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &samples);
        if (samples >= (int)mNetworkChunkSize)
        {
            sendAudioFrames(samples);   
        }
    }
}

void VoiceEngine::updateListener(float* pos, float* dir, float* vel)
{
    mSelfListener.setPosition(pos[0], pos[1], pos[2]);
    mSelfListener.setDirection(dir[0], dir[1], dir[2]);
    mSelfListener.setVelocity(vel[0], vel[1], vel[2]);
}

// -----------------------------------------------------------------------------
// Avatar methods

void VoiceEngine::updateAvatar(const VoiceUUID& id, float* pos, float* dir, float* vel)
{
    apr_thread_mutex_lock(mAvatarMutex);

    SourceMap::iterator i = mSources.find(id);
    if (i == mSources.end())
    {
        apr_thread_mutex_unlock(mAvatarMutex);
        return;
    }

    i->second.setPosition(pos[0], pos[1], pos[2]);
    i->second.setDirection(dir[0], dir[1], dir[2]);
    i->second.setVelocity(vel[0], vel[1], vel[2]);

    apr_thread_mutex_unlock(mAvatarMutex);
}

VoiceBuffer* VoiceEngine::newAvatar(const VoiceUUID& id, VoiceCodec* codec)
{
    // Create voice buffer and source
    VoiceBuffer* buffer = new(VoiceBufferPool::malloc()) VoiceBuffer(mBufferFrameCount);
    apr_thread_mutex_lock(mAvatarMutex);
    mBuffers.insert(std::make_pair(id, buffer));
    mSources.insert(std::make_pair(id, VoiceSource()));
    apr_thread_mutex_unlock(mAvatarMutex);

    // Create voice decoders

    codec->allocDecoder(id);
//     for (CodecMap::iterator i = mCodecs.begin(); i != mCodecs.end(); ++i)
//     {
//         i->second->allocDecoder(id);
//     }

    return buffer;
}

void VoiceEngine::removeAvatar(const VoiceUUID &id)
{
    // Release voice buffer and source

    apr_thread_mutex_lock(mAvatarMutex);

    BufferMap::iterator b = mBuffers.find(id);
    if (b != mBuffers.end())
    {
        if (b->second)
        {
            b->second->~VoiceBuffer();
            VoiceBufferPool::free(b->second);
            b->second = 0;
        }
        mBuffers.erase(b);
    }

    SourceMap::iterator s = mSources.find(id);
    if (s != mSources.end())
    {
        mSources.erase(s);
    }

    apr_thread_mutex_unlock(mAvatarMutex);

    // Release voice decoders

    for (CodecMap::iterator i = mCodecs.begin(); i != mCodecs.end(); ++i)
    {
        if (i->second->hasDecoder(id))
            i->second->releaseDecoder(id);
    }
}

VoiceBuffer* VoiceEngine::getAvatarVoiceBuffer(const VoiceUUID& id) const
{
    BufferMap::const_iterator i = mBuffers.find(id);
    if (i == mBuffers.end())
        return 0;

    return i->second;
}

// -----------------------------------------------------------------------------
// Network methods

bool VoiceEngine::connect(const char* host, int port, const VoiceUUID& id)
{
    // Check for possibility of evil misconfiguration
    if (strstr(host, "http://") == host)
        host += 7;

    if (mSock != VE_INVALID_SOCKET)
        disconnect();

    if (ve_create_socket_tcp(mSock))
    {
        if (ve_connect_socket_tcp(mSock, host, port))
        {
            sendLogin(id);
            sendEnableVOIP(mEnabled);
            mRun = true;

            apr_thread_create(&mReceiveThread, NULL, receiveThread, (void*)this, mAprPool);
            return true;
        }

        mRun = false;
        ve_destroy_socket_tcp(mSock);
    }

    return false;
}

void VoiceEngine::disconnect()
{
    ve_destroy_socket_tcp(mSock);

    mRun = false;
    if (mReceiveThread)
    {
        apr_status_t ret = 0;
        apr_thread_join(&ret, mReceiveThread);
        if (ret != 0)
        {
            llwarns << "Warning: Unable to shut down receive thread" << llendl;
        }
    }
    mReceiveThread = 0;
}

int VoiceEngine::sendPacketHeader(char type, unsigned int size)
{
    char packet[sizeof(char)+sizeof(unsigned int)];
    memcpy(packet, &type, sizeof(char));
    memcpy(&packet[sizeof(char)], &size, sizeof(unsigned int));
    return ve_send_packet_tcp(mSock, packet, sizeof(packet));
}

int VoiceEngine::recvPacketHeader(char* type, unsigned int* size)
{
    int received = 0;
    received = ve_receive_packet_tcp(mSock, type, sizeof(char));
    if (received <= 0) return received;
    received = 0;
    while (received < sizeof(unsigned int))
        received += ve_receive_packet_tcp(mSock, &((char*)size)[received], sizeof(unsigned int)-received);
    return received + sizeof(char);
}

int VoiceEngine::recvVoiceHeader(VoicePacketHeader* header)
{
    int received = 0;
    int headerSize = sizeof(VoicePacketHeader);
    while (received < headerSize)
    {
        int bytesReceived = ve_receive_packet_tcp(mSock, &((char*)header)[received], headerSize-received);
        received += bytesReceived;
        if (bytesReceived <= 0) return bytesReceived;
    }

    return received;
}

int VoiceEngine::recvUUID(VoiceUUID* id)
{
    int received = 0;
    while (received < sizeof(VoiceUUID))
        received += ve_receive_packet_tcp(mSock, &((char*)id)[received], sizeof(VoiceUUID)-received);

    return received;
}

int VoiceEngine::sendLogin(const VoiceUUID& id)
{
    int sent = 0;
    sent += sendPacketHeader(VP_LOGIN, sizeof(VoiceUUID) + sizeof(int));
    sent += ve_send_packet_tcp(mSock, (const char*)&id, sizeof(VoiceUUID));
    sent += ve_send_packet_tcp(mSock, (const char*)&mSupportedFormats, sizeof(mSupportedFormats));
    return sent;
}

int VoiceEngine::sendEnableVOIP(bool enabled)
{
    int sent = 0;
    sent += sendPacketHeader(VP_ENABLE_VOIP, sizeof(char));
    char val = enabled ? 1 : 0;
    sent += ve_send_packet_tcp(mSock, &val, sizeof(char));
    return sent;
}

int VoiceEngine::sendAudioFrames(unsigned int samples)
{
    if ((!mCaptureDevice) || (!mCaptureBuffer))
        return 0;

    if (samples > mNetworkChunkSize * mBufferFrameCount)
        samples = mNetworkChunkSize * mBufferFrameCount;
  
    VoicePacketHeader header;
    header.format = mEncodeFormat;

    CodecMap::const_iterator i = mCodecs.find(mEncodeFormat);
    if (i != mCodecs.end())
    {
        // Encoded audio, encode in frames equal to codec frame size and send all available frames
        // as a chunk as big as possible
        unsigned short frameSizePCM = i->second->getFrameSizePCM();
        unsigned int chunkSize = samples;
        if (chunkSize >= frameSizePCM)
        {
            // Calculate frame and sample count
            unsigned short frames = chunkSize/frameSizePCM;
            unsigned int sampleCount = frames*frameSizePCM;

            // Capture only as many samples as we are going to use
            alcCaptureSamples(mCaptureDevice, mCaptureBuffer, sampleCount);

            bool constantFrameSize = i->second->isEncodedSizeConstant();

            // Allocate only one frame header if frame size is constant
            size_t headerCount = constantFrameSize ? 1 : frames;
            VoiceFrameHeader* frameHeaders = (VoiceFrameHeader*)FrameHeaderPool::ordered_malloc(headerCount);

            // Encode audio frames to a temporary buffer

            size_t bufferSize = frames * frameSizePCM * mRecordSampleSize;
            char* encBuffer = (char*)mSendPool.ordered_malloc(bufferSize);

            unsigned short size = 0;
            unsigned int framesSkipped = 0;
            for (int i = 0; i<frames; ++i)
            {
                unsigned short encodedSize = encodeAudioFrame(i*frameSizePCM, (i+1)*frameSizePCM, &encBuffer[size]);

                if (encodedSize == 0)
                {
                    ++framesSkipped;
                    continue;
                }

                if (constantFrameSize)
                    frameHeaders->frameSize = encodedSize;
                else if (frameHeaders->frameSize == 0)
                    frameHeaders[i-framesSkipped].frameSize = encodedSize;

                size += encodedSize;
            }

            frames -= framesSkipped;

            header.frames = frames;
            header.decodedAudioSize = frames*frameSizePCM*mRecordSampleSize;
            header.audioSize = size;

            unsigned int frameHeadersSize = (unsigned int)sizeof(VoiceFrameHeader)*headerCount;

            // filter: drop silent voice packets

			int silenceSampleCount = 0;
			for (unsigned int i=0; i<sampleCount; i++)
			{
				if (mRecordSampleSize == 1)
				{
					int sampleUnsigned8bit = mCaptureBuffer[i];
					
					if ( abs(sampleUnsigned8bit) < 128*silencePowerThreshold )
						silenceSampleCount++;
				}
				if (mRecordSampleSize == 2)
				{
					int sampleUnsigned16bit = mCaptureBuffer[i*2+1]*256+mCaptureBuffer[i*2]; // little endian

					if (abs(sampleUnsigned16bit) < 32768*silencePowerThreshold)
						silenceSampleCount++;
				}
			}

			bool emptyVoicePacket = false;
			if ( ((double)silenceSampleCount)/sampleCount > silenceSampleRateThreshold )
				emptyVoicePacket = true;

			int voicePacketLengthMs = 1000 * sampleCount / mRecordSampleRate;

			if (emptyVoicePacket)
				sequentialSilentTimeMs += voicePacketLengthMs;
			else
				sequentialSilentTimeMs = 0;

			if (sequentialSilentTimeMs >= silentVoicePacketFilterTriggerTimeMs)
				silentVoicePacketFilterActived = true;
			else
				silentVoicePacketFilterActived = false;

			// Send headers and audio data to server

            if (header.audioSize != 0 && !silentVoicePacketFilterActived)
            {
                sendPacketHeader(VP_AUDIO_TO_SERVER, sizeof(header) + frameHeadersSize + header.audioSize);
                ve_send_packet_tcp(mSock, (const char*)&header, sizeof(header));
                ve_send_packet_tcp(mSock, (const char*)frameHeaders, frameHeadersSize);
                ve_send_packet_tcp(mSock, encBuffer, header.audioSize);
            }

            // Free temporary buffers

            FrameHeaderPool::ordered_free(frameHeaders, headerCount);
            frameHeaders = 0;

            mSendPool.ordered_free(encBuffer, bufferSize);
            encBuffer = 0;
//             mSendPool.ordered_free(audioBuffer, bufferSize);
//             audioBuffer = 0;

            return sampleCount;
        }
    }

    return 0;
}

int VoiceEngine::recvAudioFrames(unsigned int expectedSize)
{
    int size = 0;
    int received = 0;

    VoiceUUID avatarId;
    received = recvUUID(&avatarId);
    size += received;
    if (received <= 0) return size;

    // Read audio header

    VoicePacketHeader header;
    received = recvVoiceHeader(&header);
    size += received;
    if (received <= 0) return size;

    // Check for codec format and do a dummy receive if the codec is unknown

    received = 0;
    CodecMap::const_iterator codec = mCodecs.find(header.format);
    if (codec == mCodecs.end())
    {
        llwarns << "Warning: Avatar has an unknown voice codec (id:" << header.format << ")" << llendl;
        char* data = (char*)mReceivePool.ordered_malloc(expectedSize - size);
        size += ve_receive_packet_tcp(mSock, data, expectedSize - size);
        mReceivePool.ordered_free(data, expectedSize - size);
        return size;
    }

    // Create new avatar if not found

    VoiceBuffer* voice = getAvatarVoiceBuffer(avatarId);
    if (voice == 0)
    {
        voice = newAvatar(avatarId, codec->second.get());
        if (!voice)
            return size;
    }

    // Read frame headers

    bool constantFrameSize = codec->second->isEncodedSizeConstant();
    int headerCount = constantFrameSize ? 1 : header.frames;
    VoiceFrameHeader* frameHeaders = (VoiceFrameHeader*)FrameHeaderPool::ordered_malloc(headerCount);

    for (int h = 0; h < headerCount; ++h)
    {
        received = 0;
        while (received < sizeof(VoiceFrameHeader))
        {
            int packetBytes = ve_receive_packet_tcp(mSock, &((char*)&frameHeaders[h])[received],
                                                 sizeof(VoiceFrameHeader)-received);
            size += packetBytes;
            received += packetBytes;
            if (packetBytes <= 0) return size;
        }
    }

    // Read audio data

    received = 0;
    char* data = (char*)mReceivePool.ordered_malloc(header.audioSize);
    while (received < header.audioSize)
    {
        int bytesReceived = ve_receive_packet_tcp(mSock, &data[received], header.audioSize-received);
        received += bytesReceived;
        size += bytesReceived;
        if (bytesReceived <= 0) return size;
    }


    // Decode audio data

    char* audioData = 0;
    unsigned short decodedSize = 0;

//     // Test sound to detect jittering
//     unsigned int lastPos = voice->getLastPosition();
//     decodedSize = header.decodedAudioSize;
//     audioData = (char*)mReceivePool.ordered_malloc(header.decodedAudioSize);
//     short* ad = (short*)audioData;
//     for (int i = 0; i < decodedSize/2; ++i)
//     {
//         ad[i] = (short)(sinf((i+lastPos)/10.0f)*32767);
//     }

    unsigned short sampleSize = sampleSizeFromVoiceFormat(codec->second->getDecodeFormat());
    audioData = (char*)mReceivePool.ordered_malloc(header.decodedAudioSize);
    unsigned short frameSizeDecoded = codec->second->getFrameSizePCM() * sampleSize;
    size_t offset = 0;
    unsigned decodedTotal = 0;
    for (unsigned short f = 0; f < header.frames; ++f)
    {
        // Get encoded frame size
        unsigned short frameSizeEncoded = constantFrameSize ? frameHeaders->frameSize : frameHeaders[f].frameSize;

        // Decode encoded frame to audio buffer
        decodedSize += codec->second->decode(avatarId, &data[offset], frameSizeEncoded,
                                             &audioData[f*frameSizeDecoded], frameSizeDecoded);
        offset += frameSizeEncoded;
        decodedTotal += frameSizeDecoded;
    }

    // Queue audio for playback
    if (decodedTotal)
    {
        apr_thread_mutex_lock(mAvatarMutex);

        unsigned sampleRate = codec->second->getSampleRate();
        ALuint sampleFormat = voiceFormatToOpenAL(codec->second->getDecodeFormat());
        voice->bufferData(sampleFormat, sampleRate, decodedTotal, audioData);

        apr_thread_mutex_unlock(mAvatarMutex);
    }

    // Free audio headers and data
    mReceivePool.ordered_free(data, header.audioSize);
    mReceivePool.ordered_free(audioData, header.decodedAudioSize);
    FrameHeaderPool::ordered_free(frameHeaders, constantFrameSize ? 1 : header.frames);

    return size;
}

// -----------------------------------------------------------------------------
// Network data receiver thread

void* APR_THREAD_FUNC VoiceEngine::receiveThread(apr_thread_t* thread, void* param)
{
    VoiceEngine* ve = (VoiceEngine*)param;

    while (ve->mRun)
    {
        char packetType;
        unsigned int packetSize;
        if (ve->recvPacketHeader(&packetType, &packetSize) <= 0)
            return 0;

        switch (packetType)
        {
            case VP_AUDIO_TO_CLIENT:
                {
                    int receivedSize = ve->recvAudioFrames(packetSize);
                    if (receivedSize <= 0)
                        return 0;

                    if (receivedSize != packetSize)
                    {
                        llwarns << "Warning: receivedSize != packetSize ("
                                << receivedSize << "/" << packetSize << ")" << llendl;
                    }
                    break;
                }

            default:
                llwarns << "Unknown packet header received: type " << packetType << ", size " << packetSize
                        << ". Shutting down receive thread..." << llendl;
                return 0;
        }
    }

    return 0;
}

// ----------------------------------------------------------------------------

void VoiceEngine::setEnabled(bool enabled)
{
    mEnabled = enabled;
    if (mSock != VE_INVALID_SOCKET)
        sendEnableVOIP(mEnabled);
}

#endif