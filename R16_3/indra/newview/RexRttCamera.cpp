/** 
 * @file RexRttCamera.cpp
 * @brief RexRttCamera class implementation
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"
#include "RexRttCamera.h"
#include "llogre.h"
#include "llviewerimagelist.h"
#include "llviewerobjectlist.h"
#include "rexogrelegacymaterial.h"
#include "llogreobject.h"
#include "llagent.h"
#include "llvoavatar.h"
#include "llogrepostprocess.h"

#include <OGRE/Ogre.h>

extern LLDispatcher gGenericDispatcher;

unsigned int RexRttCamera::smNumInstance = 0;
RttCameraDispatchHandler RexRttCameraManager::smDispatchRttCam;
RexViewportDispatchHandler RexRttCameraManager::smDispatchViewport;

const LLString RttCameraDispatchHandler::mMessageID("RexRttCam");
bool RttCameraDispatchHandler::operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
      const LLDispatchHandler::sparam_t& string)
{
   LLString strCommand = string[0];
   LLString name = string[1];
   LLString imageId = string[2];
   LLString strPos = string[3];
   LLString strLookAt = string[4];
   LLString strWidth = string[5];
   LLString strHeight = string[6];

   LLVector3 pos(0,0,0);
   LLVector3 lookat(0,0,0);
	if (!LLVector3::parseVector3(strPos.c_str(), &pos))
      return false;
   if (!LLVector3::parseVector3(strLookAt.c_str(), &lookat))
      return false;

   int command = Ogre::StringConverter::parseInt(strCommand);


   Ogre::Real width = Ogre::StringConverter::parseReal(strWidth);
   Ogre::Real height = Ogre::StringConverter::parseReal(strHeight);

   if (name.empty())
      name = "default";

   if (command == 1)
      RexRttCameraManager::getInstance().addCamera(name, LLUUID(imageId), pos, lookat, width, height);
   else if (command == 0)
      RexRttCameraManager::getInstance().removeCamera(name);

   return true;
}

const LLString RexViewportDispatchHandler::mMessageID("RexSetViewport");
bool RexViewportDispatchHandler::operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
      const LLDispatchHandler::sparam_t& string)
{
   LLString strCommand = string[0];
   LLString name = string[1];

   int command = Ogre::StringConverter::parseInt(strCommand);

   Ogre::Real x = Ogre::StringConverter::parseReal(string[2]);
   Ogre::Real y = Ogre::StringConverter::parseReal(string[3]);
   Ogre::Real width = Ogre::StringConverter::parseReal(string[4]);
   Ogre::Real height = Ogre::StringConverter::parseReal(string[5]);

   // clamp values to something resembling sane (viewport at pos 1,1 with dimensions 0,0 should be legal, albeit invisible)
   x = std::min(1.0f, std::max(0.f, x));
   y = std::min(1.0f, std::max(0.f, y));
   width = std::min(1.f - x, std::max(0.f, width));
   height = std::min(1.f - y, std::max(0.f, height));


   if (name.empty())
      name = "default";

   if (command == 1)
      RexRttCameraManager::getInstance().addViewport(name, x, y, width, height);
   else if (command == 0)
      RexRttCameraManager::getInstance().removeViewport(name);

   return true;
}

RexRttCamera::RexRttCamera() : mInstanceName("RttCam" + Ogre::StringConverter::toString(smNumInstance)), mCamera(0), mMaterial(0), mViewport(0)
{
   smNumInstance++;
   mCamera = LLOgreRenderer::getPointer()->getSceneMgr()->createCamera("Rtt_Camera_" + mInstanceName);
   mCamera->setFixedYawAxis(false);
   mCamera->setNearClipDistance(0.3f);
   mCamera->setFarClipDistance(40.f);
   mCamera->setPosition(Ogre::Vector3::ZERO);
   mCamera->lookAt(Ogre::Vector3::UNIT_Z);
}

RexRttCamera::~RexRttCamera()
{
}

void RexRttCamera::remove()
{   
   if (mViewport)
   {
      mViewport->setCamera(0);
      Ogre::RenderTarget *rt = mViewport->getTarget();
      rt->removeViewport(mViewport->getZOrder());
      mViewport = 0;
   }
   removeTexture();

   if (mCamera)
   {
      LLOgreRenderer::getPointer()->getSceneMgr()->destroyCamera(mCamera);
      mCamera = 0;
   }
   if (mMaterial)
   {
      mMaterial->setStatic(false);
   }
}

void RexRttCamera::removeTexture()
{
   Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(mTextureName);
   if (texture.isNull() == false)
   {
      Ogre::RenderTarget *rt = texture->getBuffer()->getRenderTarget();
      rt->removeAllListeners();
      rt->removeAllViewports();

      Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(mMaterialName);
      if (material.isNull() == false)
      {
         Ogre::TextureUnitState *tu = material->getTechnique(0)->getPass(0)->getTextureUnitState(0);
         if (tu && mOldTextureName.empty() == false)
         {
            tu->setTextureName(mOldTextureName);
         }
      }
      Ogre::TextureManager::getSingleton().remove(static_cast<Ogre::ResourcePtr>(texture));
      mMaterial->updateVariationsTexture();
   }
}

void RexRttCamera::setPosition(const LLVector3 &position)
{
   mCamera->setPosition(LLOgreRenderer::getPointer()->toOgreVector(position));
}

void RexRttCamera::lookAt(const LLVector3 &target)
{
   mCamera->lookAt(LLOgreRenderer::getPointer()->toOgreVector(target));
}

void RexRttCamera::setParent(Ogre::SceneNode *node)
{
   mCamera->setPosition(Ogre::Vector3::ZERO);
   if (mCamera->isAttached() == false)
   {
      Ogre::SceneNode *cameraNode = node->createChildSceneNode();
      cameraNode->attachObject(mCamera);
   }
}

void RexRttCamera::setViewport(Ogre::Viewport *viewport)
{
   if (viewport)
      viewport->setCamera(mCamera);

   mViewport = viewport;
}

void RexRttCamera::setTexture(RexOgreLegacyMaterial *material, int width, int height)
{
   mTextureName = "rtt_texture" + mInstanceName;
   mMaterialName = material->getName();

   removeTexture(); // Remove old rtt texture

   mMaterial = material;
   Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(mTextureName,
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D,
      width, height, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET);

   Ogre::RenderTexture *renderTexture = texture->getBuffer()->getRenderTarget();

   //mViewport = 
   renderTexture->addViewport(mCamera);
   renderTexture->getViewport(0)->setClearEveryFrame(true);
   renderTexture->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Green);
   renderTexture->getViewport(0)->setOverlaysEnabled(false);
   renderTexture->getViewport(0)->setShadowsEnabled(false);
//   renderTexture->getViewport(0)->setSkiesEnabled(false);
   renderTexture->addListener(RexRttCameraManager::getInstancePtr());
   mCamera->setAspectRatio(height / (float)width);

   setTextureToMaterial(material, mTextureName);
   mMaterial->setStatic(true);
}

void RexRttCamera::setTextureToMaterial(RexOgreLegacyMaterial *material, const std::string &texture)
{
   Ogre::MaterialPtr pmaterial = Ogre::MaterialManager::getSingleton().getByName(material->getName());
   if (pmaterial.isNull() == false)
   {
      Ogre::TextureUnitState *tu = pmaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0);
      if (mOldTextureName.empty())
      {
         mOldTextureName = tu->getTextureName();
      }
      tu->setTextureName(texture);
   }
   material->updateVariationsTexture();
}


// *****************

RexRttCameraManager::RexRttCameraManager() :  mPrevFogStart(0.f), mPrevFogEnd(0.f)
{
   gGenericDispatcher.addHandler(smDispatchRttCam.mMessageID, &smDispatchRttCam);
   gGenericDispatcher.addHandler(smDispatchViewport.mMessageID, &smDispatchViewport);

   for (int i=1 ; i<8 ; ++i)
      mAvailableZ.push(i);
}

void RexRttCameraManager::clear()
{
   RttCameraMap::iterator iter = mRttCameras.begin();
   for ( ; iter != mRttCameras.end() ; ++iter)
      iter->second->remove();

   mRttCameras.clear();

   ViewportMap::iterator vpiter = mViewports.begin();
   for ( ; vpiter != mViewports.end() ; ++vpiter)
   {
      _removeViewport(vpiter->second);
   }
   mViewports.clear();
}

//static 
RexRttCameraManager &RexRttCameraManager::getInstance()
{
   static RexRttCameraManager instance;
   return instance;
}

// static
RexRttCameraManager *RexRttCameraManager::getInstancePtr()
{
   return &getInstance();
}

void RexRttCameraManager::addCamera(const std::string &name, const LLUUID &texture, const LLVector3 &position, const LLVector3 &lookAt,
                                               int texWidth, int texHeight)
{
   llinfos << "Adding render to texture camera: " << name << " to scene..." << llendl;

   // limit texture size to something resembling sanity (but not really)
   texWidth = std::min(texWidth, 2048);
   texWidth = std::min(texHeight, 2048);
   if (LLOgreRenderer::getPointer()->isInitialized())
   {
      LLOgreRenderer::getPointer()->setOgreContext();
      RexRttCameraPtr camera;
      RttCameraMap::iterator iter = mRttCameras.find(name);
      if (iter == mRttCameras.end())
      {
         camera = RexRttCameraPtr(new RexRttCamera);
         mRttCameras[name] = camera;
      } else
      {
         camera = iter->second;
      }

      // update camera with options
      camera->setPosition(position);
      camera->lookAt(lookAt);

      LLUUID objectId(name);
      LLViewerObject *object = gObjectList.findObject(objectId);
      if (object)
      {
         LLOgreObject *ogreObject = object->getOgreObject();
         if (ogreObject)
         {
            camera->setParent(ogreObject->getSceneNode());
         }

      }
      if (mViewports.find(name) != mViewports.end())
      {
         // Attach camera to pre-specified viewport
         camera->setViewport(mViewports[name]);
      } else
      {
         // Render to texture
         LLViewerImage *image = gImageList.getImage(texture);

         if (image)
         {
            RexOgreLegacyMaterial *material = image->getOgreMaterial();
            camera->setTexture(material, texWidth, texHeight);
         }
      }
      LLOgreRenderer::getPointer()->setSLContext();
   }
}

void RexRttCameraManager::removeCamera(const std::string &name)
{
   llinfos << "Removing render to texture camera: " << name << " from scene..." << llendl;
   RttCameraMap::iterator iter = mRttCameras.find(name);
   if (iter != mRttCameras.end())
   {
      removeViewport(name);
      iter->second->remove();
      mRttCameras.erase(iter);
   }
}

void RexRttCameraManager::addViewport(const std::string &name, float x, float y, float width, float height)
{
   llinfos << "Adding viewport: " << name << "..." << llendl;
   if (mViewports.find(name) == mViewports.end())
   {
      if (LLOgreRenderer::getPointer()->isInitialized())
      {
         LLOgreRenderer::getPointer()->setOgreContext();

         if (mAvailableZ.empty() == false) // 8 viewports max
         {
            int zOrder = mAvailableZ.front();
            mAvailableZ.pop();
            Ogre::Viewport *defaultVp = LLOgreRenderer::getPointer()->getViewport();
            Ogre::RenderTarget *rt = defaultVp->getTarget();
            Ogre::Camera *defaultCamera = LLOgreRenderer::getPointer()->getCamera();
            Ogre::Viewport *newViewport = rt->addViewport(defaultCamera, zOrder, x, y, width, height);
            mViewports[name] = newViewport;
         } else
         {
            llwarns << "Too many viewports, remove previous one(s) to add new!" << llendl;
         }
         LLOgreRenderer::getPointer()->setSLContext();
      }
   }
}

void RexRttCameraManager::removeViewport(const std::string &name)
{
   llinfos << "Removing viewport: " << name << "..." << llendl;
   ViewportMap::iterator iter = mViewports.find(name);
   if (iter != mViewports.end())
   {
      Ogre::Viewport *viewport = iter->second;
      RttCameraMap::iterator rtiter = mRttCameras.begin();
      for( ; rtiter != mRttCameras.end() ; ++rtiter)
      {
         if (rtiter->second->getViewport() == viewport)
         {
            rtiter->second->setViewport(0);
         }
      }

      _removeViewport(viewport);

      mViewports.erase(iter);
      
   }
}

void RexRttCameraManager::_removeViewport(Ogre::Viewport *viewport)
{
   int zOrder = viewport->getZOrder();
      
   Ogre::Viewport *defaultVp = LLOgreRenderer::getPointer()->getViewport();
   Ogre::RenderTarget *rt = defaultVp->getTarget();
   rt->removeViewport(zOrder);
   mAvailableZ.push(zOrder);

   // restore the main viewport as the current one for the default camera, otherwise 
   // LLOgreRenderer::getPointer()->getCamera()->getViewport() may return a viewport that actually is not the main,
   // causing problems with post process effects
   // Note: he problem is, Ogre::Camera->getViewport() returns the last viewport that was attached to the camera, not the main one
   //       necessarily.
   LLOgreRenderer::getPointer()->getRenderWindow()->getViewport(0)->setCamera(LLOgreRenderer::getPointer()->getCamera());

   // restore all enabled post process effects as they get wiped when we mess around with viewports
   LLOgreRenderer::getPointer()->getPostprocess()->restoreOldSettings(LLOgreRenderer::getPointer()->getViewport());
}

//virtual
void RexRttCameraManager::preRenderTargetUpdate (const Ogre::RenderTargetEvent &evt)
{
   Ogre::SceneManager *manager = LLOgreRenderer::getPointer()->getSceneMgr();

   mPrevFogStart = manager->getFogStart();
   mPrevFogEnd = manager->getFogEnd();
   Ogre::Real farclip = evt.source->getViewport(0)->getCamera()->getFarClipDistance();

   manager->setFog(manager->getFogMode(), manager->getFogColour(), manager->getFogDensity(), farclip * 0.5f, farclip);

   // if we didn't want to be such haxors about this, we would use Ogre SceneManager's setVisibilityFlag() and visibility masks on objects
   // make avatar visible if we are in mouse look mode (normally the avatar is invisible, causing the avatar not to get rendered on rtt surfaces)
   if (gAgent.cameraMouselook())
   {
      gAgent.getAvatarObject()->getOgreObject()->setVisible(true);
   }
}

//virtual
void RexRttCameraManager::postRenderTargetUpdate (const Ogre::RenderTargetEvent &evt)
{
   Ogre::SceneManager *manager = LLOgreRenderer::getPointer()->getSceneMgr();
   manager->setFog(manager->getFogMode(), manager->getFogColour(), manager->getFogDensity(), mPrevFogStart, mPrevFogEnd);

   if (gAgent.cameraMouselook()) // hide it afterwards
   {
      gAgent.getAvatarObject()->getOgreObject()->setVisible(false);
   }
}

