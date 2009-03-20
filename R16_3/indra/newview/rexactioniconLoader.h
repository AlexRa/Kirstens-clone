// Copyright (c) 2007-2008 adminotech
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __ActionIconLoader_h__
#define __ActionIconLoader_h__

#include <linden_common.h>
#include <message.h>
#include <llhttpclient.h>
#include <RexAvatar.h>

class LLPanelActionIcons;

//! Responder for rex avatar (storage) messages
/*! Handles messages for avatar updates.
   
    Gets hashes for avatar assets and either retrieves the assets from cache
    or requests the assets from avatar storage.
*/
class ActionIconResponder : public LLHTTPClient::Responder
{
public:
   //! default constructor
   ActionIconResponder();

   //! Constructor that specifies avatar
   /*!
       \param avatar realXtend avatar
   */
   ActionIconResponder(LLPanelActionIcons *mIconList);

   //! destructor
   virtual ~ActionIconResponder();

//   //! Set the avatar object that uses this object
//   void setAvatar(LLAvatar *avatar);
   
   //! Error occurred, handle it
   virtual void error(U32 status, const std::string& reason);	// called with bad status codes

   //! Parse the received content and handle it
   virtual void result(const LLSD& content);

protected:
   //! Avatar object that uses this responder
  // Rex::ClientAvatar *Avatar;
//   LLAvatar *mAvatar;
   LLPanelActionIcons *mIconList;
};

//! Responder for rex avatar (storage) asset messages
/*! Handles messages for avatar asset updates.
    Generally meshes and skeletons should go together. If you create a whole new skeleton,
    a whole new mesh should be also created for it.

    Additional skeletons with same (or similar) hierarchy and bone names are used as new
    animations for the avatar. In this way it is possible to create completely new avatar,
    export it to avatar storage and then use it on the avatar on the client in which ever
    way one chooses.

    Similarly materials and textures should go together, if you make a new texture, a
    new material specifying the new texture should go along with it.
*/
class ActionIconAssetResponder : public ActionIconResponder
{
public:
   //! default constructor
   ActionIconAssetResponder() : ActionIconResponder(), mType(LLAssetType::AT_NONE) {};

   //! Constructor that specifies avatar
   /*!
       \param avatar realXtend avatar
   */
   ActionIconAssetResponder(LLPanelActionIcons *IconList, const std::string &name, LLAssetType::EType type, const std::string &hash) : ActionIconResponder(IconList), mName(name), mType(type), mHash(hash) {};

   //! destructor
   virtual ~ActionIconAssetResponder() {};

   //! Error occurred, handle it
   virtual void error(U32 status, const std::string& reason);	// called with bad status codes

   //! Parse the received content and handle it
   virtual void result(const LLSD& content);

   //! Sets asset type for this responder
   virtual void setAssetType(LLAssetType::EType type)
   {
      mType = type;
   }

private:
   LLAssetType::EType mType;

   //! Asset base64 hash
   std::string mHash;

   //! Asset name
   std::string mName;
};

//! Loads avatar either from local source (cache) or from external source (download from avatar storage server)
/*!
    \todo Finish this.
*/
class ActionIconLoader
{
private:
   ActionIconLoader() {}

public:
   ~ActionIconLoader() {}

   //! Loads avatar. Basically imports avatar from external source.
   static bool importAvatar(Rex::ClientAvatar *avatar);

   static LLSD blockingGet(const std::string& url);
};

#endif // __ActionIconLoader_h__


