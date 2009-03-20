#pragma once
// ---------------------------------------------------------------------------
// ** Rex skype plugin **
// 
// Users skype address is sent to rex server if "exposeSkypeAddress=true" is 
// found from skype.ini file. Otherwise the skype plugin will not be
// initialized.
//
// Rex server will do follow operations after skype address is received from
// rex client:
// 1) Store received address to database for later usage (authentication server)
// 2) Send received skype address to all users in sim
// 3) Send all users (stored) skype addresses to user who sent the skype address
// ---------------------------------------------------------------------------

#if LL_WINDOWS

#include "llthread.h"

#import "skype4com.dll" rename("CreateEvent","CreatePluginEvent"), rename("SendMessage","SendChatMessage")

using namespace SKYPE4COMLib;

class RexSkypePlugin;

//! Thread for launching / attaching to Skype client
class RexSkypePluginAttach : public LLThread
{
public:
   RexSkypePluginAttach() : LLThread("SkypeAttachThread"), mSkypePlugin(0) {}
   virtual ~RexSkypePluginAttach() {}
   virtual void run(void);
   void launchSkype(bool retry = false);
   void setSkypePlugin(RexSkypePlugin *plugin) { mSkypePlugin = plugin; }

private:
   RexSkypePlugin *mSkypePlugin;
};

// Wrapper for SKYPE4COM activeX component
class RexSkypePlugin 
{
   friend class RexSkypePluginAttach;
private:
	// interface
	RexSkypePlugin(void);
public:
	~RexSkypePlugin(void);

	bool StartSkypeCall(const std::string &skypeAddress);
	
	bool InitOk() const;

   // initialize skype. Call when enabling skype
   void Initialize();
   
   bool skypeInstalled() const;

	// static instanche handlers
	static void CreateInstance();
	static RexSkypePlugin* GetInstance();
	static void RemoveInstance(bool notifyServer = false);

private:
   std::string GetLocalSkypeAccount() const;
   bool createSkype();

   // Call after initialize and after connected to server, does nothing if skype is not initialized
   void sendSkypeAddressToServer();

	static RexSkypePlugin* pInstance;

   bool skypePresent;
	volatile bool initOk; 
	int protocolVersion;
	bool exposeSkypeAddress;
	std::string mySkypeAddress;
	ISkypePtr skype;
	ICallPtr callPointer;

   RexSkypePluginAttach SkypeAttachThread;

	
	void LoadSettings();
};

#endif


