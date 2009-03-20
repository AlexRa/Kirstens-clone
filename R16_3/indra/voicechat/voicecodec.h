#ifndef VOICECODEC_H
#define VOICECODEC_H

#include "voiceformat.h"

class VoiceUUID;

/**
 *  Voice codec interface
 */
class VoiceCodec
{
public:
    virtual ~VoiceCodec() {}
    /**
     *  Encodes audio from an input data buffer to a given output buffer
     *  @param input Input data buffer
     *  @parma output A pointer to output data buffer
     *  @param outSize Maximum number of bytes that can be written to output buffer
     */
    virtual unsigned short encode(const char* input, size_t inSize, char* output, size_t outSize) = 0;

    /**
     *  Decodes audio from an input data buffer to a given output buffer
     *  @param id Decoder identifier
     *  @param input Input data buffer
     *  @parma output A pointer to output data buffer
     *  @param outSize Maximum number of bytes that can be written to output buffer
     */
    virtual unsigned short decode(const VoiceUUID& id, const char* input, size_t inSize, char* output, size_t outSize) = 0;

    /**
     *	Allocates a new decoder that can be used to decode audio
     *  @param id Decoder identifier
     */
    virtual void allocDecoder(const VoiceUUID& id) = 0;

    /**
     *  Looks up if the codec has a decoder with the given id
     *	@return true if the decoder was found or false otherwise
     */
    virtual bool hasDecoder(const VoiceUUID& id) = 0;

    /**
     *	Frees a decoder
     *  @param Decoder identifier
     */
    virtual void releaseDecoder(const VoiceUUID& id) = 0;

    /**
     *  @return The format in which the audio will be after it's encoded
     */
    virtual int getEncodeFormat() const = 0;

    /**
     *  @return The format the audio should be in before encoding
     */
    virtual VoiceFormat getDecodeFormat() const = 0;

    /**
     *  @return Desired sample rate in Hz
     */
    virtual int getSampleRate() const = 0;

    /**
     *  @return True if the size of an encoded frame is same for all
     *          frames or false if the encoded frame size varies
     *          depending on the input
     */
    virtual bool isEncodedSizeConstant() const = 0;

    /**
     *  @return Frame size in PCM samples
     */
    virtual unsigned short getFrameSizePCM() const = 0;


};  //  class VoiceCodec

#endif  //  VOICECODEC_H
