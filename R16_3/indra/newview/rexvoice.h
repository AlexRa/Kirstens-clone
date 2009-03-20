#ifndef REXVOICE_H
#define REXVOICE_H

class RexVoice
{
public:
    static void create();
    static void destroy();

    // Needed for push to talk events
    static void keyDown(KEY key, MASK mask);
    static void keyUp(KEY key, MASK mask);
    static bool handleVoiceClientPrefsChanged(const LLSD& newvalue);

protected:
    RexVoice();

    bool voiceClientPrefsChanged();

    static RexVoice* sInstance;

    BOOL mEnabled;
    KEY mPTTKey;
    bool mEnablePTT;

};  //  class RexVoice

#endif	//	REXVOICE_H
