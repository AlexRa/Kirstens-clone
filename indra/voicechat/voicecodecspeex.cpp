#include "voicecodecspeex.h"

VoiceCodecSpeex::VoiceCodecSpeex(int sampleRate)
    : mEncState(0)
    , mSampleRate(sampleRate)
{
    // Initialize speex
    speex_bits_init(&mEncBits);
    speex_bits_init(&mDecBits);

    mEncState = speex_encoder_init(&speex_nb_mode);
    speex_encoder_ctl(mEncState, SPEEX_SET_SAMPLING_RATE, &mSampleRate);

//	int vbr = 1;// Variable bit rate to ON
//	speex_encoder_ctl(mEncState, SPEEX_SET_VBR, &vbr); 

     int quality = 8; // Need good headphones to tell the difference
     speex_encoder_ctl(mEncState, SPEEX_SET_QUALITY, &quality);
}

VoiceCodecSpeex::~VoiceCodecSpeex()
{
    speex_bits_destroy(&mEncBits);
    speex_bits_destroy(&mDecBits);

    speex_encoder_destroy(mEncState);
    mEncState = 0;

    for (DecoderMap::iterator i = mDecoders.begin(); i != mDecoders.end(); ++i)
    {
        speex_decoder_destroy(i->second);
    }
}

unsigned short VoiceCodecSpeex::encode(const char* input, size_t inSize, char* output, size_t outSize)
{
    speex_bits_reset(&mEncBits);
    if (speex_encode_int(mEncState, (spx_int16_t*)input, &mEncBits) == 0)
        return 0;

    return (unsigned short)speex_bits_write(&mEncBits, output, (int)outSize);
}

unsigned short VoiceCodecSpeex::decode(const VoiceUUID& id, const char* input, size_t inSize, char* output, size_t outSize)
{
    DecoderMap::const_iterator i = mDecoders.find(id);
    if (i == mDecoders.end())
        return 0;

    speex_bits_read_from(&mDecBits, (char*)input, (int)inSize); //  XXX Why isn't read buffer const?
    speex_decode_int(i->second, &mDecBits, (spx_int16_t*)output);

    return (unsigned short)outSize;
}

VoiceFormat VoiceCodecSpeex::getDecodeFormat() const
{
    return VF_PCM16;
}

int VoiceCodecSpeex::getEncodeFormat() const
{
    return VF_SPEEX;
}

bool VoiceCodecSpeex::isEncodedSizeConstant() const
{
    return true;
}

unsigned short VoiceCodecSpeex::getFrameSizePCM() const
{
    int frameSize;
    speex_encoder_ctl(mEncState, SPEEX_GET_FRAME_SIZE, &frameSize);
    return (unsigned short)frameSize;
}

int VoiceCodecSpeex::getSampleRate() const
{
    return mSampleRate;
}

void VoiceCodecSpeex::allocDecoder(const VoiceUUID& id)
{
    void* decState = speex_decoder_init(&speex_nb_mode);
    speex_decoder_ctl(decState, SPEEX_SET_SAMPLING_RATE, &mSampleRate);
    mDecoders.insert(std::make_pair(id, decState));
}

bool VoiceCodecSpeex::hasDecoder(const VoiceUUID& id)
{
    return (mDecoders.find(id) != mDecoders.end());
}

void VoiceCodecSpeex::releaseDecoder(const VoiceUUID& id)
{
    DecoderMap::iterator i = mDecoders.find(id);
    if (i != mDecoders.end())
    {
        speex_decoder_destroy(i->second);
        mDecoders.erase(i);
    }
}
