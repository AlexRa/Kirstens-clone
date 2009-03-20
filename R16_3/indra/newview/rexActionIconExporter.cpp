// Copyright (c) 2007-2008 adminotech
// For conditions of distribution and use, see copyright notice in license.txt

#include "llviewerprecompiledheaders.h"
#include "CommonHeaders.h"
#include "rexActionIconExporter.h"

#include "llpanelactionicons.h"
#include <llpumpio.h>
#include <llcurl.h>
#include <llsdserialize.h>
#include <sha1.h>
#include <llbase64.h>
#include <llhttpclient.h>
#include <xmlrpc-epi/xmlrpc.h>
#include "llagent.h"
#include "llfloaterrexlogin.h"
#include "llavatar.h" 




REXActionIconExporter::REXActionIconExporter() : 
     Transaction(0)
   , Status(ALL_IDLE)
   , ErrorOccurred(false)
   , mExportingActionIconCalled(FALSE)
   , mDeletingActionIconCalled(FALSE)
   , Request(0)
   , Loop_Run(FALSE)
{
}

REXActionIconExporter::~REXActionIconExporter()
{
   cleanUp();
}

void REXActionIconExporter::cleanUp()
{
   SAFE_DELETE(Transaction);
   if (Request)
   {
      XMLRPC_RequestFree(Request, 1);
      Request = 0;
   }
   Status = ALL_IDLE; 
}

void REXActionIconExporter::setLoginCredentials(const std::string& account, const std::string& password)
{
   LoginAccount = account;
   LoginPassword = password;
}



BOOL REXActionIconExporter::login()
{

   //// If no credentials, store directly using default address
   if (LoginAccount.empty() || LoginPassword.empty())
   {
	  setError("Empty account or password.");
	  return FALSE;
   }
   else
   {
	  
	  if (Login.initiate(LoginAccount, LoginPassword))
	  {
		 llinfos << "Logging in with account: " << LoginAccount << llendl;
		 // RETURN TRUE
	  }
	  else
	  {
		 // Login failed to initiate, previous login still in progress
		 setError(Login.getErrorMessage());
		 return FALSE;
	  }
   }


return TRUE;
}




void REXActionIconExporter::loginAndDeleteFromASS( REXActionIconBtn *pIcon)
{
	Loop_Run = TRUE;

	if( Status == DELETING_NEWLOGIN)
	{
	   if(login())
	   {
			// SessionHash = Login.getSessionHash();
			 Status = DELETING_LOGIN;
			 CurrentIcon = pIcon;
	   }
	   else
	   {
		   Status = DELETING_NEWLOGIN;
		   LoginFloater();
	   }


	}

}


void REXActionIconExporter::loginAndStoreToASS( REXActionIconBtn *pIcon)
{
	Loop_Run = TRUE;

	if( Status == EXPORTING_NEWLOGIN)
	{
	   if(login())
	   {
		     //SessionHash = Login.getSessionHash();
			 Status = EXPORTING_LOGIN;
			 CurrentIcon = pIcon;
	   }
	   else
	   {
		    Status = EXPORTING_NEWLOGIN;
			LoginFloater();
	   }
	}

}


void REXActionIconExporter::StoreToASS(REXActionIconBtn *pIcon)
{

	if( Status == ALL_IDLE)
	{
		llinfos << "Exporting action icon..." << llendl;
		Loop_Run = TRUE;
		CurrentIcon =pIcon;
		Status =  EXPORTING_LOGIN;
	}
	else
	{
		if(mExportingActionIconCalled)
		{
		llinfos << "Can't export while old exporting still in progress! (storeToASS)" << llendl;
		return;
		}

		llinfos << "Exporting action icon...(WAITING)" << llendl;
		mExportingActionIconCalled = TRUE;
		mExportIconTemp = pIcon;
	}


}

void REXActionIconExporter::DeleteFromASS(REXActionIconBtn *pIcon)
{

	if( Status == ALL_IDLE)
	{
		llinfos << "Deleting action icon..." << llendl;
	    Loop_Run = TRUE;
		CurrentIcon =pIcon;
		Status =  DELETING_LOGIN;
	}
	else
	{
		if(mDeletingActionIconCalled)
		{
			llinfos << "Can't delete while old deleting still in progress! (DeleteFromASS)" << llendl;
			return;
		}
		llinfos << "Deleting action icon...(WAITING)" << llendl;
		mDeletingActionIconCalled = TRUE;
		mDeleteIconTemp = pIcon;
	}


}

void REXActionIconExporter::Store(REXActionIconBtn *pIcon)
{
   if (ASSUri.empty())
   {
      setError("No uri to export to.");
      Ogre::LogManager::getSingleton().logMessage("Export failed");
      return;
   }

   if (Status == EXPORTING_EXPORTAGAIN)
      Status = EXPORTING_STOPPED;

   if (Transaction || Status != EXPORTING_STOPPED)
   {
      setError("Previous export still in progress.");
      llinfos << "Can't export while old exporting still in progress! (Store)" << llendl;
      return;
   }
   setError("No error.");
   ErrorOccurred = false;

   static REXActionIconBtn *CurrentIcon = 0;
   if (pIcon == 0)
      pIcon = CurrentIcon;

   CurrentIcon = pIcon;

   if (!pIcon)
      return;

   // create the request
//	XMLRPC_REQUEST 
    Request = XMLRPC_RequestNew();
	XMLRPC_RequestSetMethodName(Request, "StoreActionIcon");
	XMLRPC_RequestSetRequestType(Request, xmlrpc_request_call);


	// USER ID
      std::string id = gAgent.getID().getString();
      size_t n = id.find("-", 0);
      for ( ; n != std::string::npos ; n=id.find("-", 0))
         id.erase(n, 1);

	  LLImageRaw * rawima = pIcon->rawImagep;
	

	  S32 ImageDatasize = rawima->getDataSize();
      char * ImageData = (char *)rawima->getData();


	  S8  C = rawima->getComponents();
	  U16 H = rawima->getHeight();
	  U16 W = rawima->getWidth();
    
	  char h[10];
	  char w[10];
	  char Components[10];

		for (U32 i=0; i < 10; i++)
		{
		  h[i] = '\0';
		  w[i] = '\0';
		  Components[i] = '\0';
		}

	  sprintf(h, "%d", H); // itoa is not a standard function in C, it is Windows-only
	  sprintf(w, "%d", W); // TODO: Is "%d" correct in all three of these?
	  sprintf(Components, "%d", C);
		
//	  itoa(H, h, 10); 
//	  itoa(W, w, 10); 
//	  itoa(C, Components, 10);

	  S32 DataSize = ImageDatasize + (10*3);
	  char *Data = new char[DataSize];

		for (S32 i=0; i < ImageDatasize; i++)
			{
			Data[i] =  ImageData[i];
			}

			int a=0;
			for (U32 i=0; i < 10; i++)
			{
				if(w[i] >= '0' && w[i] <= '9')
				{
				Data[ImageDatasize+a] =  w[i];
				a += 1;
				}
			}
			Data[ImageDatasize+a] =  'X';
			a += 1;

			int b=0;
			for (U32 i=0; i < 10; i++)
			{
				if(h[i] >= '0' && h[i] <= '9')
				{
				Data[ImageDatasize+a+b] =  h[i];
				b += 1;
				}
			}
			Data[ImageDatasize+a+b] =  'X';
			b += 1;


			int c = 0;
			for (U32 i=0; i < 10; i++)
			{
				if(h[i] >= '0' && h[i] <= '9')
				{
				Data[ImageDatasize+a+b+c] =  Components[i];
				c += 1;
				}
			}
			Data[ImageDatasize+a+b+c] =  'X';
			c += 1;



	// stuff the parameters
	XMLRPC_VALUE params = XMLRPC_CreateVector(NULL, xmlrpc_vector_struct);
	XMLRPC_VectorAppendString(params, "account", Login.getAccount().c_str(), 0);
   // XMLRPC_VectorAppendString(params, "sessionhash", SessionHash.c_str(), 0);
	XMLRPC_VectorAppendString(params, "sessionhash",  Login.getSessionHash().c_str(), 0);
    XMLRPC_VectorAppendString(params, "UserServer", Login.getAuthenticationAddress().c_str(), 0);

    XMLRPC_VectorAppendString(params, "userID",id.c_str(), 0);
	XMLRPC_VectorAppendString(params, "ID", pIcon->getIconID().getString().c_str(), 0);
	XMLRPC_VectorAppendInt(params, "x", (int) pIcon->GetPosX());
	XMLRPC_VectorAppendInt(params, "y", (int) pIcon->GetPosY());
	XMLRPC_VectorAppendBase64(params, "image", Data, DataSize);


   // Send request
   XMLRPC_RequestSetData(Request, params);

   llinfos << "Sending request to uri: " << ASSUri << llendl;
   Transaction = new LLXMLRPCTransaction(ASSUri, Request, false);

   Status = EXPORTING_GENERIC;
}

void REXActionIconExporter::Delete(REXActionIconBtn *pIcon)
{
   if (ASSUri.empty())
   {
      setError("No uri to delete icon");
      llinfos << "Delete failed" << llendl;
      return;
   }

  // if (Status == EXPORTING_EXPORTAGAIN)
  //    Status = EXPORTING_IDLE;

   if (Transaction || Status != DELETING_STOPPED)
   {
      setError("Previous delete still in progress.");
      llinfos << "Can't export while old deleting still in progress!" << llendl;
      return;
   }
   setError("No error.");
   ErrorOccurred = false;

   static REXActionIconBtn *CurrentIcon = 0;
   if (pIcon == 0)
      pIcon = CurrentIcon;

   CurrentIcon = pIcon;

   if (!pIcon)
      return;

   // create the request
//	XMLRPC_REQUEST 
    Request = XMLRPC_RequestNew();
	XMLRPC_RequestSetMethodName(Request, "DeleteActionIcon");
	XMLRPC_RequestSetRequestType(Request, xmlrpc_request_call);


	// USER ID
      std::string id = gAgent.getID().getString();
      size_t n = id.find("-", 0);
      for ( ; n != std::string::npos ; n=id.find("-", 0))
         id.erase(n, 1);


	// stuff the parameters
	XMLRPC_VALUE params = XMLRPC_CreateVector(NULL, xmlrpc_vector_struct);
	XMLRPC_VectorAppendString(params, "account", Login.getAccount().c_str(), 0);
    //XMLRPC_VectorAppendString(params, "sessionhash", SessionHash.c_str(), 0);
	XMLRPC_VectorAppendString(params, "sessionhash", Login.getSessionHash().c_str(), 0);
    XMLRPC_VectorAppendString(params, "UserServer", Login.getAuthenticationAddress().c_str(), 0);

    XMLRPC_VectorAppendString(params, "userID",id.c_str(), 0);
	XMLRPC_VectorAppendString(params, "ID", pIcon->getIconID().getString().c_str(), 0);

	
   // Send request
   XMLRPC_RequestSetData(Request, params);

   Ogre::LogManager::getSingleton().logMessage("Sending request to uri: " + ASSUri);
   Transaction = new LLXMLRPCTransaction(ASSUri, Request, false);

   Status = DELETING_GENERIC;
}


void REXActionIconExporter::LoginFloater()			
{
	if (!mFloaterRexLogin)
	{
	BOOL  UserLock = TRUE;
	LLString WhyText = "If you want save action icon, please login:";
	LLFloaterRexLogin::create(WhyText, UserLock);

	if(Status ==  EXPORTING_STOPPED || Status == EXPORTING_LOGIN )
		Status =  EXPORTING_NEWLOGIN;
	else 
	if(Status ==   DELETING_STOPPED || Status == DELETING_LOGIN)
		Status =  DELETING_NEWLOGIN;
	}
	//else
		// previous login still in progress
}




BOOL REXActionIconExporter::process()
{

   if(Loop_Run)
   {
	   switch (Status)
	   {
	   case EXPORTING_STOPPED:
		   {
			   exportCompleted();
			   break;
		   }

	   case DELETING_STOPPED:
		   {
			   deleteCompleted();
			   break;
		   }

	   case ALL_IDLE:
		   {
				if(mExportingActionIconCalled)
				{
					Ogre::LogManager::getSingleton().logMessage("Exporting action icon...(FROM WAIT PLACE)");
					mExportingActionIconCalled = FALSE;
					CurrentIcon = mExportIconTemp;
					StoreToASS( mExportIconTemp);
					mExportIconTemp = NULL;
					//break;
					return Loop_Run = TRUE;
				}
				else	
				if(mDeletingActionIconCalled)
				{
					Ogre::LogManager::getSingleton().logMessage("Deleting action icon...(FROM WAIT PLACE)");
					mDeletingActionIconCalled = FALSE;
					CurrentIcon = mDeleteIconTemp;
					DeleteFromASS( mDeleteIconTemp);
					mDeleteIconTemp = NULL;
					//break;
					return Loop_Run = TRUE;
				}

				return Loop_Run = FALSE;
		   break;
		   }


	   case EXPORTING_NEWLOGIN:
	   case DELETING_NEWLOGIN:
		   {
			if(mFloaterRexLogin)
			if(mFloaterRexLogin->Status == LLFloaterRexLogin::OK)
			{

				LoginAccount  = mFloaterRexLogin->GetAccountANDauthserver();
				LoginPassword = mFloaterRexLogin->GetPassword();
				mFloaterRexLogin->CloseLoginFloater();

				if(Status ==  EXPORTING_NEWLOGIN)
				{
					loginAndStoreToASS(CurrentIcon);
				}
				else 
				if(Status ==  DELETING_NEWLOGIN)
				{
					loginAndDeleteFromASS(CurrentIcon);
				}
			 }

			  else if(mFloaterRexLogin->Status == LLFloaterRexLogin::CANCEL)
			 {
				   llinfos << "Successful login, but empty avatar storage URL" << llendl;
				   setError("Authentication: Empty avatar storage URL received");
				   mFloaterRexLogin->CloseLoginFloater();
				   //Status = ALL_IDLE;
				   	if(Status ==  EXPORTING_NEWLOGIN)
					{
						Status = EXPORTING_STOPPED;
					}
					else 
					if(Status ==  DELETING_NEWLOGIN)
					{
						Status = DELETING_STOPPED;
					}

					//Cancel all 
					mExportingActionIconCalled = FALSE;
					mExportIconTemp = NULL;
					mDeletingActionIconCalled = FALSE;
					mDeleteIconTemp = NULL;
				   
			 }
		   }
			break;



	   case EXPORTING_LOGIN:
	   case DELETING_LOGIN:
		   {
 
			   if(Login.getStatus() == Login.LOGIN_NOT_INITIATED)
				{
					LoginFloater();
				}

			   if (Login.process())
			   {

				if(Status ==  EXPORTING_LOGIN)
				   Status =  EXPORTING_STOPPED;
				else 
				if(Status ==  DELETING_LOGIN)
				   Status =  DELETING_STOPPED;

				 if (Login.getSuccess())
				 {
					std::string url = gPanelIcons->getStorageAddress();

					if (!url.empty())
					{  


						if(Status ==  EXPORTING_STOPPED)
							url = (url + "/xmlrpc/"+ CurrentIcon->getIconID().getString().c_str());//StoreActionIcon/");
					    else 
					    if(Status ==  DELETING_STOPPED)
						  url = (url + "/xmlrpc/" + CurrentIcon->getIconID().getString().c_str());//DeleteActionIcon/");

					   llinfos << "Successful login, avatar storage URL is " << url << llendl;

					   setASSUri(url);

					   if(Status ==  EXPORTING_STOPPED)
							Store(CurrentIcon);
					   else 
					   if(Status ==  DELETING_STOPPED)
							Delete(CurrentIcon);
					}
					else
					{

		             llinfos << "Successful login, but empty avatar storage URL" << llendl;
					//LoginFloater();
					//return Loop_Run = TRUE;

		  
		             setError("Authentication: Empty avatar storage URL received");
					}
				 }
				 else
				 {
					llinfos << "Authentication login failed: " << Login.getErrorMessage() << llendl;
					setError(std::string("Login failed: ") + Login.getErrorMessage());
					LoginFloater();
					
				 }
			  }
		   }
		  break;


	   case EXPORTING_GENERIC:
		  {
  
			 if (Transaction)
			 {
				if (Transaction->process())
				{
				   if (Transaction->status(0) > LLXMLRPCTransaction::StatusComplete)
				   {
					  Status = EXPORTING_STOPPED;
					  llinfos << "Error1 while exporting icon: " << Transaction->statusMessage() << llendl;
					  setError(std::string("Avatar storage server: ") + Transaction->statusMessage());
				   } else
				   {
					  LLXMLRPCValue result = Transaction->responseValue();
						// If sessionhash is present, should be successful
						//SessionHash = '\0';
						//SessionHash = result["SessionHash"].asString();
					    std::string Err = result["Error"].asString();

						if (Err.empty())
						{

//						   ErrorMessage = "Login successful";
						}
						else
						{
							llinfos << "Error Transaction: action icon export: " << Err << llendl;
						   //ErrorMessage = result["message"].asString();
							setError(std::string("Error: ") + Transaction->statusMessage());
						}

					  Status = EXPORTING_STOPPED;
				   }
				   clearCurrentTransaction();
	               
				} else if (Transaction->status(0) >= LLXMLRPCTransaction::StatusComplete)
				{
				   // Error occurred, cancel export
				   Status = EXPORTING_STOPPED;
				   llinfos << "Error2 while exporting icon: " << Transaction->statusMessage() << llendl;
				   setError(std::string("Avatar storage server: ") + Transaction->statusMessage());
				}
			 }
			 break;
		  }

	   case DELETING_GENERIC:
		  {
			 if (Transaction)
			 {
				if (Transaction->process())
				{
				   if (Transaction->status(0) > LLXMLRPCTransaction::StatusComplete)
				   {
					  Status = DELETING_STOPPED;
					  llinfos << "Error while deleting icon: " << Transaction->statusMessage() << llendl;
					  setError(std::string("Avatar storage server: ") + Transaction->statusMessage());
				   } else
				   {
					  LLXMLRPCValue result = Transaction->responseValue();
					    
					  std::string Err = result["Error"].asString();
						if (Err.empty())
						{

//						   ErrorMessage = "Login successful";
						}
						else
						{
							llinfos << "Error Transaction: action icon delete: " << Err << llendl;
						   //ErrorMessage = result["message"].asString();
							setError(std::string("Error: ") + Transaction->statusMessage());
						}


					  Status = DELETING_STOPPED;
				   }
				   clearCurrentTransaction();
	               
				} else if (Transaction->status(0) >= LLXMLRPCTransaction::StatusComplete)
				{
				   // Error occurred, cancel export
				   Status = DELETING_STOPPED;
				   llinfos << "Error while deleting icon: " << Transaction->statusMessage() << llendl;
				   setError(std::string("Avatar storage server: ") + Transaction->statusMessage());
				}
			 }
			 break;
		  }


	   }
	     return Loop_Run = TRUE;
	}
   return Loop_Run = FALSE;
}



void REXActionIconExporter::exportCompleted(void)
{
   Status = ALL_IDLE;
   
   if (hasError())
   {
      llinfos << "Actionicon not exported." << llendl;
   } else
   {
      llinfos << "ActionIcon exported." << llendl;
   }
}


void REXActionIconExporter::deleteCompleted(void)
{
   Status = ALL_IDLE;
   
   if (hasError())
   {
      llinfos << "Actionicon not deleted." << llendl;
   } else
   {
      llinfos << "ActionIcon deleted." << llendl;
   }
}

void REXActionIconExporter::clearCurrentTransaction()
{
   SAFE_DELETE(Transaction);
   if (Request)
   {
      XMLRPC_RequestFree(Request, 1);
      Request = 0;
   }
}

void REXActionIconExporter::setASSUri(const std::string &uri)
{
   ASSUri = uri;
}

const std::string &REXActionIconExporter::getUri() const
{
   return ASSUri;
}

REXActionIconExporter::ExportStatus REXActionIconExporter::getStatus() const
{
   return Status;
}



void REXActionIconExporter::setError(const std::string &error)
{
   Error = error;
   ErrorOccurred = true;
}

std::string REXActionIconExporter::getErrorDescription() const
{
   if (ErrorOccurred)
      return Error;

   return std::string();
}

bool REXActionIconExporter::hasError() const
{
   return ErrorOccurred;
}


void REXActionIconExporter::subStr(std::string &str, const std::string &replaceThis, const std::string &replaceWith)
{
   size_t location = str.find(replaceThis, 0);
   while (location != std::string::npos)
   {
      str.replace(location, replaceThis.length(), replaceWith);
      location = str.find(replaceThis, 0);      
   }
}


