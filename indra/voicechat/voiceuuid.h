#ifndef VOICEUUID_H
#define VOICEUUID_H

class VoiceUUID
{
public:
    VoiceUUID();
    VoiceUUID(const unsigned char id[16]);
	VoiceUUID(const VoiceUUID& other);

    void operator = (const VoiceUUID& other);

    bool operator == (const VoiceUUID& other) const;
    bool operator < (const VoiceUUID& other) const;

    unsigned char mID[16];

};	//	class VoiceUUID

#endif	//	VOICEUUID_H
