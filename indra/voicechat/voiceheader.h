#ifndef VOICEHEADER_H
#define VOICEHEADER_H

// ---------------------------------------------------------------------------
// Packet types

enum VoicePacketType
{
    VP_LOGIN = 0,
    VP_AUDIO_TO_SERVER = 1,
    VP_AUDIO_TO_CLIENT = 2,
    VP_ENABLE_VOIP = 3
};

// ---------------------------------------------------------------------------
// Audio data headers

struct VoicePacketHeader
{
    int format;
    unsigned short audioSize;
    unsigned short decodedAudioSize;
    unsigned short frames;
};  //  struct VoicePacketHeader

struct VoiceFrameHeader
{
    unsigned short frameSize;
};  //  struct VoiceFrameHeader

#endif  //  VOICEHEADER_H
