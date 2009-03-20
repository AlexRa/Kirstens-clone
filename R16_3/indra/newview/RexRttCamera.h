/** 
 * @file RexRttCamera.h
 * @brief RexRttCamera class header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */
// reX new file

#ifndef LL_REXRTTCAMERA_H					
#define LL_REXRTTCAMERA_H

#include "lldispatcher.h"
#include <OGRE/OgreRenderTargetListener.h>

namespace Ogre
{
   class Camera;
   class Texture;
   class SceneNode;
   class Viewport;
}

class RexOgreLegacyMaterial;

//! Dispatcher for rtt camera generic message.
class RttCameraDispatchHandler : public LLDispatchHandler
{
public:
   //! Handle dispatch
	virtual bool operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
		const sparam_t& string);

   static const LLString mMessageID;
};


//! Dispatcher for viewport generic message.
class RexViewportDispatchHandler : public LLDispatchHandler
{
public:
   //! Handle dispatch
	virtual bool operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
		const sparam_t& string);

   static const LLString mMessageID;
};

//! Render to texture / viewport camera
/*!
    Renders the specified camera view to a texture or an additional viewport.
    The texture and the viewport are destroyed with this class.
    
    \note Do not use directly, use the manager instead.
*/
class RexRttCamera
{
   friend class RexRttCameraManager;
private:
   //! default constructor
   RexRttCamera();

public:
   //! destructor
   ~RexRttCamera();

private:
   //! Remove camera from scene
   void remove();

   //! remove rtt texture
   void removeTexture();

   //! set camera position
   void setPosition(const LLVector3 &position);

   //! set look at target
   void lookAt(const LLVector3 &targe);

   //! Set parent scenenode for the camera
   void setParent(Ogre::SceneNode *node);

   void setViewport(Ogre::Viewport *viewport);
   const Ogre::Viewport *getViewport() const { return mViewport; }

   void setTexture(RexOgreLegacyMaterial *material, int width, int height);

private:
   //! Assigns the texture to the specified material
   void setTextureToMaterial(RexOgreLegacyMaterial *material, const std::string &texture);

   static unsigned int smNumInstance;

   //! Ogre camera
   Ogre::Camera *mCamera;

   //! Ogre viewport
   Ogre::Viewport *mViewport;

   //! Texture names
   std::string mTextureName;
   std::string mOldTextureName;

   //! Material name
   std::string mMaterialName;

   RexOgreLegacyMaterial *mMaterial;

   //! Unique name for objects instantiated from this class
   const std::string mInstanceName;
};
typedef boost::shared_ptr<RexRttCamera> RexRttCameraPtr;

//! Manager for rtt cameras
/*!
    \note use RexRttCameraManager::getSingleton() to get instance of this class
*/
class RexRttCameraManager : public Ogre::RenderTargetListener
{
private:
   //! default constructor
   RexRttCameraManager();

public:
   //! destructor
   ~RexRttCameraManager() {}

   //! Removes all rtt cameras and restores textures.
   void clear();

   static RexRttCameraManager &getInstance();
   static RexRttCameraManager *getInstancePtr();

   //! Adds or updates rtt camera. 
   /*! If the camera with the name doesn't already exist it is created. 
       If it does exist, it will be updated with the specified options.
   */
   void addCamera(const std::string &name, const LLUUID &texture, const LLVector3 &position, const LLVector3 &lookAt,
                  int texWidth, int texHeight);
   //! Remove the named rtt camera
   void removeCamera(const std::string &name);

   //! Adds viewport to scene
   void addViewport(const std::string &name, float x, float y, float width, float height);

   //! Remove the named viewport from scene
   void removeViewport(const std::string &name);
   //! Remove Ogre viewport. Should not be used directly, remove viewports by name
   void _removeViewport(Ogre::Viewport *viewport);

   virtual void preRenderTargetUpdate (const Ogre::RenderTargetEvent &evt);
   virtual void postRenderTargetUpdate (const Ogre::RenderTargetEvent &evt);

private:
   static RttCameraDispatchHandler smDispatchRttCam;
   static RexViewportDispatchHandler smDispatchViewport;

   typedef std::map<std::string, RexRttCameraPtr> RttCameraMap;
   typedef std::map<std::string, Ogre::Viewport*> ViewportMap;
   typedef std::queue<int> ViewportZList;

   //! map of rtt cameras
   RttCameraMap mRttCameras;
   //! visible viewports
   ViewportMap mViewports;
   //! list of available z depths for viewports
   ViewportZList mAvailableZ;

   //! previous fog settings. Fog settings are altered during rtt camera rendering and restored after they've been rendered
   Ogre::Real mPrevFogStart;
   Ogre::Real mPrevFogEnd;
};

#endif // LL_REXRTTCAMERA_H
