#include "voiceformat.h"

unsigned short sampleSizeFromVoiceFormat(VoiceFormat format)
{
    switch(format)
    {
        case VF_PCM8: return sizeof(char);
        case VF_PCM16: return sizeof(short);
        case VF_PCM32: return sizeof(int);
    }

    return 0;
}