#ifndef VOICECODECSPEEX_H
#define VOICECODECSPEEX_H

#include "voicecodec.h"
#include "voiceuuid.h"
#include <speex/speex.h>
#include <map>

/**
 *  Speex audio codec for voice data
 */
class VoiceCodecSpeex : public VoiceCodec
{
public:

    VoiceCodecSpeex(int sampleRate);
    ~VoiceCodecSpeex();

    unsigned short encode(const char* input, size_t inSize, char* output, size_t outSize);
    unsigned short decode(const VoiceUUID& id, const char* input, size_t inSize, char* output, size_t outSize);

    VoiceFormat getDecodeFormat() const;
    int getEncodeFormat() const;

    bool isEncodedSizeConstant() const;
    unsigned short getFrameSizePCM() const;

    int getSampleRate() const;

    void allocDecoder(const VoiceUUID& id);
    bool hasDecoder(const VoiceUUID& id);
    void releaseDecoder(const VoiceUUID& id);

private:
    SpeexBits mEncBits, mDecBits;
    void* mEncState;
    int mSampleRate;

    typedef std::map<VoiceUUID, void*> DecoderMap;
    DecoderMap mDecoders;

};  //  class VoiceCodecSpeex


#endif  //  VOICECODECSPEEX_H
