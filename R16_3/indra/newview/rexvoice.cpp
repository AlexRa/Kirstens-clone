// ----------------------------------------------------------------------------
//  TODO:
//  - Fix "Talk button lock"
//  - Support "Start viewer in push to talk mode"
// ----------------------------------------------------------------------------

#include "llviewerprecompiledheaders.h"
#include "rexvoice.h"
#include "voiceengine.h"
#include "audioengine.h"
#ifndef LL_FMOD
#include "audioengine_openal.h"
#endif

#include "llkeyboard.h"
#include "llcontrol.h"
#include "llrexhud.h"

extern LLControlGroup gSavedSettings;

RexVoice* RexVoice::sInstance = 0;

RexVoice::RexVoice()
{
    // Listen to changes in voice settings
    // static listener now registered in llviewercontrol.cpp

    // Get initial settings

    if (!LLKeyboard::keyFromString(gSavedSettings.getString("PushToTalkButton"), &mPTTKey))
        mPTTKey = KEY_NONE;
}

void RexVoice::create()
{
    sInstance = new RexVoice;
    #ifdef LL_FMOD
        VoiceEngine* ve = new VoiceEngine(0);
    #else
        VoiceEngine* ve = new VoiceEngine((LLAudioEngine_OpenAL*)gAudiop);
    #endif
    BOOL enabled = gSavedSettings.getBOOL("EnableVoiceChat");
    ve->setEnabled((enabled == TRUE));
    sInstance->mEnabled = enabled;
}

void RexVoice::destroy()
{
    delete sInstance;
    sInstance = 0;

    delete VoiceEngine::getInstance();
}

void RexVoice::keyDown(KEY key, MASK mask)
{
    if (!sInstance)
        return;           

    if (gKeyboard->getKeyRepeated(key))
        return;

    if (gSavedSettings.getBOOL("PTTCurrentlyEnabled"))
    {
        if (gSavedSettings.getBOOL("PushToTalkToggle"))
        {
            if (key == sInstance->mPTTKey)
            {
                if (VoiceEngine::getInstance()->isRecording())
                    VoiceEngine::getInstance()->stopRecording();
                else if(!LLRexHud::mScriptMute)
                    VoiceEngine::getInstance()->startRecording();
            }
        }
        else if (sInstance->mPTTKey != KEY_NONE)
        {
            if (!LLRexHud::mScriptMute && gKeyboard->getKeyDown(sInstance->mPTTKey) && !VoiceEngine::getInstance()->isRecording())
                VoiceEngine::getInstance()->startRecording();
        }
    }
}

void RexVoice::keyUp(KEY key, MASK mask)
{
    if (!sInstance)
        return;

    //if (!sInstance->mPTTIsMiddleMouse)
    {
        if(!gSavedSettings.getBOOL("PushToTalkToggle") && (sInstance->mPTTKey != KEY_NONE))
        {
            if (!gKeyboard->getKeyDown(sInstance->mPTTKey) && VoiceEngine::getInstance()->isRecording())
                VoiceEngine::getInstance()->stopRecording();
        }
    }
}

// static
bool RexVoice::handleVoiceClientPrefsChanged(const LLSD& newvalue)
{
    if (!sInstance)
        return true;

    return sInstance->voiceClientPrefsChanged();
}

bool RexVoice::voiceClientPrefsChanged()
{
    BOOL enabled = gSavedSettings.getBOOL("EnableVoiceChat");
    if (enabled != mEnabled)
    {
        mEnabled = enabled;
        VoiceEngine::getInstance()->setEnabled((enabled == TRUE));
    }

    LLString key = gSavedSettings.getString("PushToTalkButton");
    if (key == "MiddleMouse")
    {
        //mPTTIsMiddleMouse = true;
    }
    else
    {
        //mPTTIsMiddleMouse = false;
        if (!LLKeyboard::keyFromString(key, &mPTTKey))
            mPTTKey = KEY_NONE;
    }

    return true;
}
