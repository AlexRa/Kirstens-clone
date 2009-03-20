/**
 * @file rexclienteffect.cpp
 * @brief RexClientEffect class implementation
 *
 * Copyright (c) 2007-2008 LudoCraft
 */
// reX: new file

#include "llviewerprecompiledheaders.h"
#include "rexclienteffect.h"
#include "llogre.h"
#include "llogreobject.h"
#include "llogreassetloader.h"
#include "rexparticlescript.h"
#include <OGRE/Ogre.h>

extern LLDispatcher gGenericDispatcher;

EffectParticleDispatchHandler RexClientEffect::smDispatchEffectParticle;
const LLString EffectParticleDispatchHandler::smMessageID("RexCSEffect");
bool EffectParticleDispatchHandler::operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
      const LLDispatchHandler::sparam_t& string)
{
   if (string.size() < 6)
   {
      llwarns << "Wrong number of parameters in generic message '" << smMessageID << "'." << llendl;
      return true;
   }
   F32 timeUntilLaunch = Ogre::StringConverter::parseReal(string[1]);
   RexClientEffect::ParticleEffect *particle = new RexClientEffect::ParticleEffect(timeUntilLaunch, LLUUID(string[0]));

   particle->mTimeUntilDeath = Ogre::StringConverter::parseReal(string[2]);
   if (LLVector3::parseVector3(string[3].c_str(), &particle->mPosition) == FALSE)
   {
      llwarns << "Failed to parse vector in generic message '" << smMessageID << "'." << llendl;
      return true;
   }
   if (LLQuaternion::parseQuat(string[4].c_str(), &particle->mRotation) == FALSE)
   {
      llwarns << "Failed to parse quaternion in generic message '" << smMessageID << "'." << llendl;
      return true;
   }
   particle->mSpeed = Ogre::StringConverter::parseReal(string[5]);


   return true;
}

EffectStationaryParticleDispatchHandler RexClientEffect::smDispatchEffectStaticParticle;
const LLString EffectStationaryParticleDispatchHandler::smMessageID("RexSCSEffect");
bool EffectStationaryParticleDispatchHandler::operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
      const LLDispatchHandler::sparam_t& string)
{
   if (string.size() < 4)
   {
      llwarns << "Wrong number of parameters in generic message '" << smMessageID << "'." << llendl;
      return true;
   }

   LLUUID id(string[0]);
   bool enable = Ogre::StringConverter::parseBool(string[3]);
   if (!enable)
   {
      RexClientEffect::removeEffect(id);
   } else
   {
      LLQuaternion rot;
      if (LLQuaternion::parseQuat(string[2].c_str(), &rot) == FALSE)
      {
         llwarns << "Failed to parse quaternion in generic message '" << smMessageID << "'." << llendl;
         return true;
      }

      RexClientEffect::CameraParticleEffect *particle = new RexClientEffect::CameraParticleEffect(LLOgreRenderer::getPointer()->getCamera(), id, rot);
      particle->mTimeUntilDeath = 0;

      if (LLVector3::parseVector3(string[1].c_str(), &particle->mPosition) == FALSE)
      {
         llwarns << "Failed to parse vector in generic message '" << smMessageID << "'." << llendl;
         return true;
      }
   }


   return true;
}


// -----------------------------------------------------------------


GenericClientEffect::GenericClientEffect(F32 timeUntilLaunch) //: LLEventTimer(timeUntilLaunch)
{
   RexClientEffect::addEffect(this);
}

GenericClientEffect::~GenericClientEffect()
{
   RexClientEffect::removeEffect(this);
}


// -----------------------------------------------------------------


RexClientEffect::ClientEffectContainer RexClientEffect::smClientEffects;

RexClientEffect::ParticleEffect::ParticleEffect(F32 timeUntilLaunch, const LLUUID &id) :    
   GenericClientEffect(timeUntilLaunch),
   LLEventTimer(timeUntilLaunch),
   mParticleSystem(0),
   mSceneNode(0),
   mAnimation(0),
   mAnimationState(0)
{
   mID = id;
   if (mID != LLUUID::null)
   {
      LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();

      if (!assetLoader->areParticleScriptsLoaded(mID))
      {
         assetLoader->loadAsset(mID, LLAssetType::AT_PARTICLE_SCRIPT, 0);
      }
      else
      {
         RexParticleScript* particle = assetLoader->getParticleScript(mID);
         if (particle)
            particle->pumpImages();
      }
   }
}

// virtual
BOOL RexClientEffect::ParticleEffect::tick()
{
   if (mID != LLUUID::null)
   {
      if (mParticleSystem)
      {
         removeParticleSystem();
         return TRUE; // This effect has ran it's course and it's time to delete it
      } else
      {
         return createParticleSystem();
      }
   }

   return TRUE;
}

RexClientEffect::ParticleEffect::~ParticleEffect()
{
    removeParticleSystem();
}

void RexClientEffect::ParticleEffect::removeParticleSystem()
{
   LLOgreRenderer *renderer = LLOgreRenderer::getPointer();

   if (mParticleSystem)
   {
      renderer->setOgreContext();
      mSceneNode->getParentSceneNode()->removeAndDestroyChild(mSceneNode->getName());
      renderer->getSceneMgr()->destroyParticleSystem(mParticleSystem);
      renderer->setSLContext();
      mParticleSystem = 0;
   }

   if (mAnimation)
   {
      LLOgreObject::removeAnimationState(mAnimationState);
      renderer->getSceneMgr()->destroyAnimationState(mAnimationState->getAnimationName());
      renderer->getSceneMgr()->destroyAnimation(mAnimation->getName());
      mAnimation = 0;
   }
}

BOOL RexClientEffect::ParticleEffect::createParticleSystem()
{
   LLOgreRenderer *renderer = LLOgreRenderer::getPointer();
   LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();

   RexParticleScript* script = assetLoader->getParticleScript(mID);

   if (script)
   {
      const std::vector<std::string>& names = script->getParticleSystemNames();
      bool flipY = (script->getNumImages() != 0); // Flip if particle system uses downloaded images

      std::vector<std::string>::const_iterator iter = names.begin();
      for ( ; iter != names.end() ; ++iter)
      {
         renderer->setOgreContext();
         Ogre::ParticleSystem *system = renderer->createParticleSystem(*iter, flipY);
         renderer->setSLContext();

         if (system)
         {
            mSceneNode = renderer->getSceneMgr()->getRootSceneNode()->createChildSceneNode();
            mSceneNode->attachObject(system);
            Ogre::Vector3 position = LLOgreRenderer::toOgreVector(mPosition);
            mSceneNode->setPosition(position);
            Ogre::Quaternion quat = LLOgreRenderer::toOgreQuaternion(mRotation);
            mSceneNode->setOrientation(quat);
            mParticleSystem = system;

            if (Ogre::Math::Abs(mSpeed) > 0.00001f)
            {
               Ogre::String animName = mSceneNode->getName() + "_animation";
               mAnimation = renderer->getSceneMgr()->createAnimation(animName, mTimeUntilDeath);
               mAnimation->setInterpolationMode(Ogre::Animation::IM_LINEAR);
               Ogre::NodeAnimationTrack *track = mAnimation->createNodeTrack(0, mSceneNode);
               Ogre::TransformKeyFrame *kf = track->createNodeKeyFrame(0.f);
               kf->setTranslate(position);
               kf = track->createNodeKeyFrame(mTimeUntilDeath);
               kf->setTranslate(position + quat * Ogre::Vector3::NEGATIVE_UNIT_X * mSpeed * mTimeUntilDeath * 1.f);
               
               mAnimationState = renderer->getSceneMgr()->createAnimationState(animName);
               mAnimationState->setEnabled(true);
               LLOgreObject::addAnimationState(mAnimationState);
            }
                    
            this->mPeriod = mTimeUntilDeath;

            if (Ogre::Math::Abs(mTimeUntilDeath) > 0.00001f)
            {
               return FALSE;
            }
         }
      }
   }
   return TRUE;
}


// -----------------------------------------------------


RexClientEffect::CameraParticleEffect::CameraParticleEffect(const Ogre::Camera *camera, const LLUUID &id, const LLQuaternion &rot) :
   GenericClientEffect(0),
   mCamera(camera),
   mSceneNode(0),
   mParticleSystem(0)
{
   mID = id;
   mRotation = rot;

   if (mID != LLUUID::null)
   {
      LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();

      if (!assetLoader->areParticleScriptsLoaded(mID))
      {
         assetLoader->loadAsset(mID, LLAssetType::AT_PARTICLE_SCRIPT, this);
      }
      else
      {
         RexParticleScript* particle = assetLoader->getParticleScript(mID);
         if (particle)
         {
            particle->pumpImages();
            createParticleSystem(particle);
         }
      }
   }
}

RexClientEffect::CameraParticleEffect::~CameraParticleEffect()
{
   removeParticleSystem();
}

void RexClientEffect::CameraParticleEffect::createParticleSystem(RexParticleScript* script)
{
   LLOgreRenderer *renderer = LLOgreRenderer::getPointer();

   const std::vector<std::string>& names = script->getParticleSystemNames();
   bool flipY = (script->getNumImages() != 0); // Flip if particle system uses downloaded images

   std::vector<std::string>::const_iterator iter = names.begin();
   for ( ; iter != names.end() ; ++iter)
   {
      renderer->setOgreContext();
      Ogre::ParticleSystem *system = renderer->createParticleSystem(*iter, flipY);
      renderer->setSLContext();

      if (system)
      {
         system->setSortingEnabled(false);
         system->setRenderQueueGroup(Ogre::RENDER_QUEUE_8);
         Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(system->getMaterialName());
         material->setDepthCheckEnabled(false);
//         material->setFog(true);
//         system->setKeepParticlesInLocalSpace(true);
         mSceneNode = renderer->getSceneMgr()->getRootSceneNode()->createChildSceneNode();
         mSceneNode->attachObject(system);

         mSceneNode->setOrientation(LLOgreRenderer::toOgreQuaternion(mRotation));
         mParticleSystem = system;
      }
   }
}

void RexClientEffect::CameraParticleEffect::removeParticleSystem()
{
   if (mParticleSystem && mSceneNode)
   {
      LLOgreRenderer *renderer = LLOgreRenderer::getPointer();

      renderer->setOgreContext();
      mSceneNode->getParentSceneNode()->removeAndDestroyChild(mSceneNode->getName());
      renderer->getSceneMgr()->destroyParticleSystem(mParticleSystem);
      renderer->setSLContext();
      
      mParticleSystem = 0;
      mSceneNode = 0;
   }
}

// virtual
void RexClientEffect::CameraParticleEffect::onOgreAssetLoaded(LLAssetType::EType assetType, const LLUUID& uuid)
{
   LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();

   RexParticleScript* particle = assetLoader->getParticleScript(mID);
   if (particle)
   {
      particle->pumpImages();
      createParticleSystem(particle);
   }
}

//virtual
void RexClientEffect::CameraParticleEffect::update()
{
   if (mSceneNode)
   {
      mSceneNode->setPosition(mCamera->getDerivedPosition() + mCamera->getOrientation() * LLOgreRenderer::toOgreVector(mPosition));
   }
}

//static
void RexClientEffect::removeEffect(const LLUUID &id)
{
   ClientEffectContainer::iterator iter = smClientEffects.begin();
   while (iter != smClientEffects.end())
   {
      if ((*iter)->getID() == id)
      {
         ClientEffectContainer::iterator oldIter = iter;
         ++iter;
         // derived classes may be already destroyed, so this would be unsafe
         //(*oldIter)->removeParticleSystem();
         smClientEffects.erase(oldIter);
         continue;
      }

      ++iter;
   }
}

//static 
void RexClientEffect::registerHandlers()
{
   gGenericDispatcher.addHandler(EffectParticleDispatchHandler::smMessageID, &smDispatchEffectParticle);
   gGenericDispatcher.addHandler(EffectStationaryParticleDispatchHandler::smMessageID, &smDispatchEffectStaticParticle);  
}


