/**
 * @file rexflashanimationmanager.cpp
 * @brief RexFlashAnimationManager class implementation
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

// reX new file

#include "llviewerprecompiledheaders.h"

#include "rexflashanimationmanager.h"
#include "llviewerwindow.h"
#include "lldebugview.h"
#include "llogre.h"
#include <OGRE/OgreTextAreaOverlayElement.h>
#include <OGRE/OgrePanelOverlayElement.h>

extern LLDispatcher gGenericDispatcher;

using namespace Hikari;

bool RexFlashAnimationManager::smAnimationRunning = false;

FlashAnimationDispatchHandler RexFlashAnimationManager::smDispatchFlashAnimation;

const LLString FlashAnimationDispatchHandler::mMessageID("RexFlashAnim");
bool FlashAnimationDispatchHandler::operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
      const LLDispatchHandler::sparam_t& string)
{
   if (string.size() < 5)
      return false;

   LLUUID id(string[0]);

   Ogre::FloatRect rect;
   rect.left = Ogre::StringConverter::parseReal(string[1]);
   rect.top = Ogre::StringConverter::parseReal(string[2]);
   rect.right = Ogre::StringConverter::parseReal(string[3]);
   rect.bottom = Ogre::StringConverter::parseReal(string[4]);

   F32 timeToDeath = 0;
   if (string.size() == 6)
      timeToDeath = Ogre::StringConverter::parseReal(string[5]);


   LLOgreRenderer::getPointer()->getFlashAnimationManager()->createFlashAnimation(id, rect, timeToDeath);

   llinfos << "Playing flash animation."  << llendl;

   return true;
}


RexFlashAnimationManager::RexFlashAnimationManager() : 
mHikariMgr(0)
, mLoadingOverlay(0)
, mLoadingTextArea(0)
, mLoadingOverlayPanel(0)
, mDestructionTimer(0)
{
#ifdef USE_HIKARI
   mHikariMgr = new HikariManager("media\\flash");
   
   gGenericDispatcher.addHandler(smDispatchFlashAnimation.mMessageID, &smDispatchFlashAnimation);
   
#endif
}

RexFlashAnimationManager::~RexFlashAnimationManager()
{
   removeAllTempFiles();

   removeLoadingOverlay();

   if (mDestructionTimer)
   {
      delete mDestructionTimer;
      mDestructionTimer = 0;
   }

#ifdef USE_HIKARI
   if (mHikariMgr)
      delete mHikariMgr;
#endif
}

void RexFlashAnimationManager::removeAllTempFiles()
{
   // Remove temp files, if any
   FlashFileMap::const_iterator iter = LoadedFlashFiles.begin();
   for ( ; iter != LoadedFlashFiles.end() ; ++iter)
   {
      deleteTemp(iter->second);
   }
}

void RexFlashAnimationManager::createFlashAnimation(const LLUUID &assetId, const Ogre::FloatRect &rect, F32 timeToDeath)
{
#ifdef USE_HIKARI
   if (smAnimationRunning)
      return;
   if (FlashAnimations.find(assetId) != FlashAnimations.end())
      return;

   FlashDesc flash;
   flash.mRect = rect;
   flash.mTimeToDeath = timeToDeath;
   flash.mName = generateName(assetId);

   FlashAnimations[assetId] = flash;

   LLOgreRenderer *renderer = LLOgreRenderer::getPointer();
   renderer->setOgreContext();

   removeLoadingOverlay();
   Ogre::OverlayManager *manager = Ogre::OverlayManager::getSingletonPtr();
   //Ogre::PanelOverlayElement *
   mLoadingOverlayPanel = static_cast<Ogre::PanelOverlayElement*>(manager->createOverlayElement("Panel", "flash_loading_panel"));
	mLoadingOverlayPanel->setMetricsMode(Ogre::GMM_RELATIVE);
   mLoadingOverlayPanel->setDimensions(1, 1);
   mLoadingOverlayPanel->setPosition(0, 0);
//   mLoadingOverlayPanel->setMaterialName("fem_hiphop_pants_brown");
   mLoadingOverlayPanel->setEnabled(true);

   //Ogre::TextAreaOverlayElement *
   mLoadingTextArea = static_cast<Ogre::TextAreaOverlayElement*>(manager->createOverlayElement("TextArea", "flash_loading_text"));
   mLoadingTextArea->setMetricsMode(Ogre::GMM_RELATIVE);
	mLoadingTextArea->setDimensions(0.6, 0.4);
   mLoadingTextArea->setFontName("BlueHighway");
   mLoadingTextArea->setCharHeight(0.05f);
   mLoadingTextArea->setCaption("Loading movie...");
   mLoadingTextArea->setPosition(0.37, 0.46);
//   textArea->setMaterialName("fem_hiphop_pants_brown");
   mLoadingOverlayPanel->addChild(mLoadingTextArea);
	
   if (!mLoadingOverlay)
      mLoadingOverlay = manager->create("Flash_loader_overlay");

   mLoadingOverlay->show();

   mLoadingOverlay->add2D(mLoadingOverlayPanel);
	mLoadingOverlay->setZOrder(100);

   renderer->setSLContext();
   
   LLOgreAssetLoader *loader = LLOgreRenderer::getPointer()->getAssetLoader();
   loader->loadAsset(assetId, LLAssetType::AT_FLASH_ANIMATION, this);

#endif
}

void RexFlashAnimationManager::createFullscreenFlashAnimation(const LLUUID &flashAnimation)
{
#ifdef USE_HIKARI
   if (smAnimationRunning)
      return;

   if (FlashAnimations.find(flashAnimation) != FlashAnimations.end())
      return;

   FlashDesc flash;
   flash.mName = generateName(flashAnimation);
   flash.mRect = Ogre::FloatRect(0, 0, 1, 1);
   flash.mTimeToDeath = 0;

   FlashAnimations[flashAnimation] = flash;

   LLOgreAssetLoader *loader = LLOgreRenderer::getPointer()->getAssetLoader();
   loader->loadAsset(flashAnimation, LLAssetType::AT_FLASH_ANIMATION, this);
#endif
}

void RexFlashAnimationManager::createFullscreenFlashAnimation(const Ogre::String &name)
{
   if (smAnimationRunning)
      return;

   FlashDesc flash;
   flash.mName = name;
   flash.mRect = Ogre::FloatRect(0, 0, 1, 1);
   flash.mTimeToDeath = 0;

   createFlashAnimation(name, flash);
}

void RexFlashAnimationManager::createFlashAnimation(const Ogre::String &name, const FlashDesc &desc)
{
#ifdef USE_HIKARI
   if (smAnimationRunning)
      return;

   if (mHikariMgr->getFlashControl(desc.mName) != 0)
   {
      llwarns << "Trying to create flash animation " << name << ", but it already exists" << llendl;
      return; // animation already exists
   }

   Ogre::FloatRect rect(desc.mRect);


   LLOgreRenderer *renderer = LLOgreRenderer::getPointer();
   renderer->setOgreContext();

   removeLoadingOverlay();

   bool loaded = false;
   try
   {
      Ogre::Viewport *vp = renderer->getViewport();

      int width = vp->getActualWidth() * rect.width();
      int height = vp->getActualHeight() * rect.height();
      short posX = vp->getActualWidth() * rect.left;
      short posY = vp->getActualHeight() * rect.top;


      posX = std::min(std::max(posX, (short)0), (short)vp->getActualWidth());
      posY = std::min(std::max(posY, (short)0), (short)vp->getActualHeight());
      width = std::max(std::min(width, vp->getActualWidth() - (int)posX), 1);
      height = std::max(std::min(height, vp->getActualHeight() - (int)posY), 1);


      FlashControl* myControl = mHikariMgr->createFlashOverlay(desc.mName, vp, width, height, Position(posX, posY), 500);
      myControl->setDraggable(false);
      myControl->bind("exit", FlashDelegate(this, &RexFlashAnimationManager::onAnimationEnd));

      loaded = myControl->loadDirect(name);
      if (!loaded)
      {
         mHikariMgr->destroyFlashControl(desc.mName);
      }
   } catch (Ogre::Exception)
   {
      // No flash player
      loaded = false;
   }
   renderer->setSLContext();

   if (loaded)
   {
      smAnimationRunning = true;

      gViewerWindow->setNormalControlsVisible(false);
      gViewerWindow->setCursor(UI_CURSOR_ARROW);

      renderer->getSceneMgr()->addSpecialCaseRenderQueue(Ogre::RENDER_QUEUE_OVERLAY);
      renderer->getSceneMgr()->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE);

      if (gDebugView)
          gDebugView->pushVisible(FALSE);
      if (gFloaterView)
          gFloaterView->pushVisible(FALSE);

      if (Ogre::Math::RealEqual(desc.mTimeToDeath, 0.f) == false)
      {
         if (mDestructionTimer)
         {
            delete mDestructionTimer;
            mDestructionTimer = 0;
         }
         mDestructionTimer = new RexEventTimer(desc.mTimeToDeath);
         mDestructionTimer->mHandler = this;
      }
   }
#endif
}

void RexFlashAnimationManager::destroyCurrentFlashAnimation(const LLUUID &assetId)
{
#ifdef USE_HIKARI
   FlashAnimationMap::iterator iter = FlashAnimations.find(assetId);
   if (iter != FlashAnimations.end())
   {
      mHikariMgr->destroyFlashControl(iter->second.mName); // happens at next update so shouldn't be necessary to switch contexts
      FlashAnimations.erase(iter);
   }

   if (mDestructionTimer)
   {
      delete mDestructionTimer;
      mDestructionTimer = 0;
   }
   enableBackgroundRendering();
#endif
}

void RexFlashAnimationManager::destroyCurrentFlashAnimation(const Ogre::String &name)
{
#ifdef USE_HIKARI
   //FlashAnimationMap::iterator iter = FlashAnimations.find(name);
   //if (iter != FlashAnimations.end())
   //{
   //   mHikariMgr->destroyFlashControl(iter->second.mName); // happens at next update so shouldn't be necessary to switch contexts
   //   FlashAnimations.erase(iter);
   //}
   //enableBackgroundRendering();
   //if (mDestructionTimer)
   //{
   //   delete mDestructionTimer;
   //   mDestructionTimer = 0;
   //}
#endif
}

void RexFlashAnimationManager::destroyAll()
{
#ifdef USE_HIKARI
   mHikariMgr->destroyAllControls();
   FlashAnimations.clear();
   removeAllTempFiles();

   enableBackgroundRendering();
   if (mDestructionTimer)
   {
      delete mDestructionTimer;
      mDestructionTimer = 0;
   }
#endif
}

void RexFlashAnimationManager::enableBackgroundRendering()
{
   if (smAnimationRunning)
   {
      LLOgreRenderer *renderer = LLOgreRenderer::getPointer();
      smAnimationRunning = false;

      gViewerWindow->setNormalControlsVisible(true);

      renderer->getSceneMgr()->clearSpecialCaseRenderQueues();
      renderer->getSceneMgr()->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_EXCLUDE);

      if (gDebugView)
          gDebugView->pushVisible(TRUE);
      if (gFloaterView)
          gFloaterView->pushVisible(TRUE);
   }
}

bool RexFlashAnimationManager::saveToTemp(const LLUUID &id, U8* buffer, S32 size)
{
   if (smAnimationRunning)
      return false;

   bool success = false;
#ifdef USE_HIKARI
   char *tempName = tmpnam(0);

   if (tempName)
   {
      std::string filename = gDirUtilp->getTempDir() + std::string(tempName) + "swf";
      std::ofstream outfile(filename.c_str(), std::ios::out | std::ios::binary);
      outfile.write((char*)buffer, size);
      outfile.close();
      
      success = (outfile.fail() == false);
      if (success)
      {
         LoadedFlashFiles[id] = filename;
      }
   }
#endif

   return success;
}

// virtual
void RexFlashAnimationManager::onOgreAssetLoaded(LLAssetType::EType assetType, const LLUUID& uuid)
{
#ifdef USE_HIKARI
   if (smAnimationRunning)
      return;

   FlashFileMap::iterator iter = LoadedFlashFiles.find(uuid);
   if (iter != LoadedFlashFiles.end())
   {
      FlashAnimationMap::iterator animIter = FlashAnimations.find(uuid);
      if (animIter != FlashAnimations.end())
         createFlashAnimation(iter->second, animIter->second);
      else
         createFullscreenFlashAnimation(iter->second);
      deleteTemp(iter->second);
      LoadedFlashFiles.erase(iter);
   }
#endif
}

//virtual
BOOL RexFlashAnimationManager::tick()
{
   mDestructionTimer = 0;
#ifdef USE_HIKARI
   destroyAll();
#endif
  
   return TRUE;
}

FlashValue RexFlashAnimationManager::onAnimationEnd(FlashControl* caller, const Arguments& args)
{
   destroyAll();
   return "Ok";
}

void RexFlashAnimationManager::removeLoadingOverlay()
{
   LLOgreRenderer::getPointer()->setOgreContext();
   // order is important
   if (mLoadingTextArea)
      Ogre::OverlayManager::getSingleton().destroyOverlayElement(mLoadingTextArea);
   if (mLoadingOverlayPanel)
      Ogre::OverlayManager::getSingleton().destroyOverlayElement(mLoadingOverlayPanel);
   if (mLoadingOverlay)
      Ogre::OverlayManager::getSingleton().destroy(mLoadingOverlay);

   mLoadingOverlay = 0;
   mLoadingOverlayPanel = 0;
   mLoadingTextArea = 0;
   LLOgreRenderer::getPointer()->setSLContext();
}

std::string RexFlashAnimationManager::generateName(const LLUUID &id) const
{
   return id.asString() + "_flash_control";
}

void RexFlashAnimationManager::deleteTemp(const std::string &filename)
{
#ifdef USE_HIKARI
   int result = remove(filename.c_str());
   if (result)
   {
      llwarns << "Failed to remove temporary flash file: " << filename << llendl;
   }
#endif
}

