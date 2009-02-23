#ifndef VOICEFORMAT_H
#define VOICEFORMAT_H

enum VoiceFormat
{
    VF_PCM8  = (1<<0),  //  8-bit uncompressed sound
    VF_PCM16 = (1<<1),  //  16-bit uncompressed sound
    VF_PCM32 = (1<<2),  //  32-bit uncompressed sound
    VF_SPEEX = (1<<3),  //  Compressed using speex
    VF_USER  = (1<<4)
};  //  enum VoiceFormat

unsigned short sampleSizeFromVoiceFormat(VoiceFormat format);

#endif  //  VOICEFORMAT_H
