// Copyright (c) 2007-2008 adminotech
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __REXActionIconExporter_h__
#define __REXActionIconExporter_h__

//#include <DynamicAnimation.h>
//#include "llxmlrpctransaction.h"
#include "Login.h"

#include <list>
#include <vector>



   class REXActionIconBtn;


//! Stores an icon to avatarstorageserver using xmlrpc transaction.
/*! First stores basic info about the icon using one request, and then based
    on the received reply, stores missing icon assets with additional requests.
*/
class REXActionIconExporter
{
public:
   //! State of the exporter
   enum ExportStatus
   {
      //! Logging to Authentication to find out avatar storage address
      EXPORTING_LOGIN = 0,
	  EXPORTING_NEWLOGIN,
      //! Exporting generic icon data
      EXPORTING_GENERIC,
      //! Exporting icon assets
    //  EXPORTING_ASSETS,
      //! icon needs to be re-exported
      EXPORTING_EXPORTAGAIN,
      //! Exporter is idle
      EXPORTING_STOPPED,

	  DELETING_NEWLOGIN,
	  DELETING_LOGIN,
	  DELETING_GENERIC,
	  DELETING_STOPPED,
	
	  LOGIN,
	  ALL_IDLE
   };






private:
  


public:
   //! constructor
   REXActionIconExporter();

   //! destructor
   ~REXActionIconExporter();

   //! Cleans up the class.
   //    Can be called after export is complete, but should not be called during.
   //    Generally the class cleans after itself after export is complete.
   
   void cleanUp();

   //! Sets login credentials
   void setLoginCredentials(const std::string& account, const std::string& password);


   void StoreToASS(REXActionIconBtn *pIcon = NULL);
   void DeleteFromASS(REXActionIconBtn *pIcon = NULL);

   BOOL Loop_Run;

   //! Advances export by little. Should be called occasionally (or every frame) to complete the transfer.
   BOOL process();

   //! Sets avatarstorageserver uri to use for exporting
   void setASSUri(const std::string &uri);

   //! Returns avatarstorageserver uri
   const std::string &getUri() const;

	
   BOOL login();

   //! Returns status of the exporter
   ExportStatus getStatus() const;

   //! Returns error description, or empty string if no error has occurred during export.
   std::string getErrorDescription() const;

   //! Returns true if error occurred while exporting.
   bool hasError() const;


protected:

 void deleteCompleted(void);
 void exportCompleted(void);

private:

   //! Logins first to Authentication and then stores icon
   void loginAndStoreToASS( REXActionIconBtn *pIcon = NULL);
   void loginAndDeleteFromASS( REXActionIconBtn *pIcon= NULL);

   void LoginFloater();

   //! Stores icon to ASS. Resets error status.
   void Store(REXActionIconBtn *pIcon = NULL);
   void Delete(REXActionIconBtn *pIcon = NULL);

   //! Clears the current xml-rpc transaction
   void clearCurrentTransaction();

   //! Set error description
   void setError(const std::string &error);

   //! Helper function for replacing a string in a string with another string
   void subStr(std::string &str, const std::string &replaceThis, const std::string &replaceWith);

   //! Xml transaction used
   LLXMLRPCTransaction *Transaction;

   //! Xml rpc request
   XMLRPC_REQUEST Request;

   //! Status of the exporting
   ExportStatus Status;


   //! Uri to avatarstorageasset to use for export
   std::string ASSUri;

 
   //! Did error occur while exporting
   bool ErrorOccurred;

  bool	mExportingActionIconCalled;
  bool	mDeletingActionIconCalled;
  REXActionIconBtn * mExportIconTemp;
  REXActionIconBtn * mDeleteIconTemp;


   //! Error description
   std::string Error;

   //! Login account to use
   std::string LoginAccount;

   //! Login password to use
   std::string LoginPassword;
   
 

   //! Login handler
   LoginHandler  Login;

   //! Current icon to export (need to store during login)
   REXActionIconBtn *CurrentIcon;

};

#endif // __REXActionIconExporter_h__
