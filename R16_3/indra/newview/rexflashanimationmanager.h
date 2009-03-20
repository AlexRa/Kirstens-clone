/** 
 * @file rexflashanimationmanager.h
 * @brief RexFlashAnimationManager header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

// reX new file

#ifndef LL_REXFLASHANIMATIONMANAGER_H
#define LL_REXFLASHANIMATIONMANAGER_H


#include "llogreassetloader.h"
#include "rexogreassetnotifiable.h"
#include "OGRE/OgrePrerequisites.h"

namespace Ogre
{
   class TextAreaOverlayElement;
   class PanelOverlayElement;
}


#ifdef USE_HIKARI
#  include "Hikari.h"
#else
namespace Hikari
{
   class HikariManager;
   class FlashControl;
   typedef char* FlashValue;
   typedef int Arguments;
}
#include <OGRE/Ogre.h>
#endif

//! Dispatcher for flash animation from server.
class FlashAnimationDispatchHandler : public LLDispatchHandler
{
public:
   //! Handle dispatch
	virtual bool operator()(const LLDispatcher* dispatcher, const std::string& key,
		                     const LLUUID& invoice, const sparam_t& string);
   static const LLString mMessageID;
};

//! Interface for rex timer event handler
class IRexTimerEventHandler
{
public:
   //! return true to destroy the timer, return false to keep calling this function periodically
   virtual BOOL tick() = 0;
};

struct RexEventTimer : public LLEventTimer
{
   RexEventTimer(F32 period) : LLEventTimer(period), mHandler(0) {}
   virtual ~RexEventTimer() {}
   RexEventTimer(const RexEventTimer &rhs) : LLEventTimer(rhs.mPeriod), mHandler(rhs.mHandler) {}

   //! handle timer event
   virtual BOOL tick()
   {
      return mHandler ? mHandler->tick() : FALSE;
   }

   IRexTimerEventHandler *mHandler;
};

//! Flash animation player (swf)
/*! Plays flash animations in an overlay over the viewport, over specified area.
    When animation is playing, rendering and key/mouse input is disabled and ui is hidden.
    Currently only supports playing back one flash file at a time.
*/
class RexFlashAnimationManager : public RexOgreAssetNotifiable, public IRexTimerEventHandler
{
private:
   struct FlashDesc
   {
      std::string mName;
      Ogre::FloatRect mRect;
      F32 mTimeToDeath;
   };

public:
   //! default constructor
   RexFlashAnimationManager();
   //! destructor
   virtual ~RexFlashAnimationManager();

   //! remove all temp files
   void removeAllTempFiles();

   //! Create flash animation with the specified asset id and dimensions.
   /*! The animation is fetched from server/asset cache
   */
   void createFlashAnimation(const LLUUID &assetId, const Ogre::FloatRect &rect, F32 timeToDeath = 0.f);

   //! Creates a fullscreen flash animation using a flash animation with the spceficied uuid.
   /*! Fetches the flash animation file swf from server/asset cache.

       \note Only one full screen flash animation can be running at any one time
   */
   void createFullscreenFlashAnimation(const LLUUID &flashAnimation);

   //! Creates a fullscreen flash animation using a flash animation with the spceficied name
   /*!
       \note Only one full screen flash animation can be running at any one time
   */
   void createFullscreenFlashAnimation(const Ogre::String &name);

   //! Create flash animation with the specified absolute path and position indicated by rect
   /*!
       \param name Absolute path to the flash file
       \param rect Position/size of the flash control
   */
   void createFlashAnimation(const Ogre::String &name, const FlashDesc &crect);

   void update()
   {
#ifdef USE_HIKARI
      mHikariMgr->update();
#endif
   }

   //! Inject mouse button down event to Hikari. Uses same codes as OIS (lmb = 0, rmb = 1, mmb = 2...)
   BOOL mousebuttonDown(int button)
   {
#ifdef USE_HIKARI
      if (smAnimationRunning)
      {
         mHikariMgr->injectMouseDown(button);
         return TRUE;
      }
#endif
      return FALSE;
   }

   BOOL mousebuttonUp(int button)
   {
#ifdef USE_HIKARI
      if (smAnimationRunning)
      {
         mHikariMgr->injectMouseUp(button);
         return TRUE;
      }
#endif
      return FALSE;
   }

   void mouseMove(float x, float y)
   {
#ifdef USE_HIKARI
      mHikariMgr->injectMouseMove(x, y);
#endif
   }

   //! Returns true if full screen flash animation is currently running
   static bool isAnimationRunning() { return smAnimationRunning; }

   //! destroy specified flash animation by lluuid
   void destroyCurrentFlashAnimation(const LLUUID &assetId);

   //! Destroys specified flash animation by name
   void destroyCurrentFlashAnimation(const Ogre::String &name);

   //! destroy all current flash animations
   void destroyAll();

   //! Save flash file to temp file
   bool saveToTemp(const LLUUID &id, U8* buffer, S32 size);

   //! event handling for loading swf file from server
   virtual void onOgreAssetLoaded(LLAssetType::EType assetType, const LLUUID& uuid);

   //! handle timer event
   virtual BOOL tick();

   //! Callback from flash for animation end
   Hikari::FlashValue onAnimationEnd(Hikari::FlashControl* caller, const Hikari::Arguments& args);

private:
   //! enables normal rendering of the world
   void enableBackgroundRendering();

   //! remove loading overlay
   void removeLoadingOverlay();

   //! generate name for the flash file
   std::string generateName(const LLUUID &id) const;

   //! Delete specified temporary file
   void deleteTemp(const std::string &filename);

   //! Flash animation generic message handler
   static FlashAnimationDispatchHandler smDispatchFlashAnimation;

   typedef std::map<LLUUID, std::string> FlashFileMap;
   typedef std::map<LLUUID, FlashDesc> FlashAnimationMap;

   //! Map of loaded (usable) flash files
   FlashFileMap LoadedFlashFiles;

   //! map of current pending flash animations
   FlashAnimationMap FlashAnimations;

   //! the hikari manager
   Hikari::HikariManager *mHikariMgr;

   //! timer for destroying flash animation
   RexEventTimer *mDestructionTimer;

   //! Is fullscreen flash animation currently running
   static bool smAnimationRunning;

   //! overlay that is shown when flash movie is loading
   Ogre::Overlay *mLoadingOverlay;
   Ogre::TextAreaOverlayElement *mLoadingTextArea;
   Ogre::PanelOverlayElement *mLoadingOverlayPanel;
};

#endif // LL_REXFLASHANIMATIONMANAGER_H

