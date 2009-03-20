#include "llviewerprecompiledheaders.h"

#ifdef LL_WINDOWS

#include "rexskypeplugin.h"
#include "llviewergenericmessage.h"
#include "llviewercontrol.h"

RexSkypePlugin* RexSkypePlugin::pInstance = NULL;

//! Thread for launching / attaching to Skype client
void RexSkypePluginAttach::run(void)
{
   if (!mSkypePlugin)
   {
      llwarns << "Threat started but no Skype plugin set" << llendl;
      return;
   }
   launchSkype();
}

void RexSkypePluginAttach::launchSkype(bool retry)
{
   bool startSkype = false;
   try
   {
      VARIANT_BOOL minimized = VARIANT_FALSE;
      VARIANT_BOOL noSplash = VARIANT_TRUE;

		if (!mSkypePlugin->skype->Client->IsRunning)
      {
         llinfos << "Starting Skype client..." << llendl;
			mSkypePlugin->skype->Client->Start(minimized, noSplash);
         startSkype = true;
      }

      HRESULT result = mSkypePlugin->skype->Attach(mSkypePlugin->protocolVersion, VARIANT_TRUE);

		if ( FAILED(result) )
		{
         llinfos << "Attaching to Skype client FAIL" << llendl;
			mSkypePlugin->initOk = false;
			return;
		}
	} catch (_com_error & err)
	{
		mSkypePlugin->initOk = false;
      if (startSkype && !retry && !LLThread::isQuitting())
      {
         launchSkype(true);
      } else
      {
         if (err.Description().length())
         {
            llinfos << "Error while starting Skype client: " << err.Description() << llendl;
         }
         else
         {
            llinfos << "Error while starting Skype client" << llendl;
         }
      }

 	   return;
	}

   if (LLThread::isQuitting() == false)
   {
      mSkypePlugin->initOk = true;
      mSkypePlugin->sendSkypeAddressToServer();
   }
}

// ****************************

RexSkypePlugin::RexSkypePlugin(void):
skypePresent(false),
initOk(false),
exposeSkypeAddress(false),
mySkypeAddress(""),
protocolVersion(6)
{
   this->skypePresent = createSkype();
}

RexSkypePlugin::~RexSkypePlugin(void)
{
   SkypeAttachThread.shutdown();
   if (SkypeAttachThread.isQuitting() == false)
   {
      skype.Detach(); // Don't detach if thread is still running, otherwise ->crash
   }
}

// Start the Skype call 
bool RexSkypePlugin::StartSkypeCall(const std::string &skypeAddress)
{
	if (!initOk || !skypePresent)
		return false;
	
	try
	{
      if (skype->CurrentUserStatus == SKYPE4COMLib::cusOffline)
         skype->ChangeUserStatus(SKYPE4COMLib::cusOnline);
		callPointer = skype->PlaceCall(skypeAddress.c_str(), "", "", "");
	}
	catch(...)
	{
		return false;
	}
//	TCallStatus status = callPointer->GetStatus();

	return true;
}

// return current Skype account on running Skype client
std::string RexSkypePlugin::GetLocalSkypeAccount() const
{
	if (!this->initOk || !this->exposeSkypeAddress || !skypePresent)
      return "";

	std::string account = "";
	if (mySkypeAddress.length() == 0 )
		account = skype->CurrentUser->Handle;
	else
		account = this->mySkypeAddress;
	return account;
}

bool RexSkypePlugin::createSkype()
{
	HRESULT result = skype.CreateInstance(__uuidof(SKYPE4COMLib::Skype));
	if ( FAILED(result) )
	{
		return false;
	}
   return true;
}

void RexSkypePlugin::Initialize()
{
   LoadSettings();
   if (!skypePresent || initOk || !exposeSkypeAddress)
      return;

   llinfos << "Initializing skype..." << llendl;

   SkypeAttachThread.setSkypePlugin(this);
   SkypeAttachThread.start();
   llinfos << "Attaching to Skype client..." << llendl;

}

void RexSkypePlugin::sendSkypeAddressToServer()
{
   LoadSettings();

   if (initOk && skypePresent)
   {
      llinfos << "Sending skype address to server..." << llendl;

	   // parameters: [0] the skype address
	   //             [1] "True" if the skype address is public otherwise "False"
	   //                 (currently server implementation doesn't use this)
	   std::vector<std::string> attributes;

	   std::string address = exposeSkypeAddress == TRUE ?  GetLocalSkypeAccount() : "";
	   attributes.push_back( address );
	   std::string publicAddress = exposeSkypeAddress == TRUE ?  "True" : "False";
	   attributes.push_back( publicAddress );
	   send_generic_message("RexSkypeStore", attributes); // maybe should rename to: RexStoreSkypeAddress
   }
}

bool RexSkypePlugin::skypeInstalled() const
{
   return skypePresent;
}

void RexSkypePlugin::CreateInstance()
{
   if (pInstance == NULL)
   	pInstance = new RexSkypePlugin();
}

RexSkypePlugin* RexSkypePlugin::GetInstance()
{
	return  RexSkypePlugin::pInstance;
}

void RexSkypePlugin::RemoveInstance(bool notifyServer)
{
	if (pInstance != NULL)
	{
      if (notifyServer)
      {
         pInstance->sendSkypeAddressToServer();
      }
	   delete pInstance;
	   pInstance = NULL;
	}
}

void RexSkypePlugin::LoadSettings()
{
   this->exposeSkypeAddress = gSavedPerAccountSettings.getBOOL("SkypeExposeAddress");
   this->mySkypeAddress = gSavedPerAccountSettings.getString("SkypeAddress");
   this->protocolVersion = gSavedPerAccountSettings.getS32("SkypeProtocolVersion");
}

bool RexSkypePlugin::InitOk() const
{
	return this->initOk;
}

#endif

