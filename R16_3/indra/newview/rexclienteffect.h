/**
 * @file rexclienteffect.h
 * @brief RexClientEffect class header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */
// reX: new file

#ifndef LL_REXCLIENTEFFECT_H
#define LL_REXCLIENTEFFECT_H

#include "lldispatcher.h"
#include "rexogreassetnotifiable.h"

namespace Ogre
{
   class ParticleSystem;
   class SceneNode;
   class Animation;
   class AnimationState;
   class Camera;
};
class RexParticleScript;

//! Dispatcher for particle effect generic message.
class EffectParticleDispatchHandler : public LLDispatchHandler
{
public:
   //! Handle dispatch
	virtual bool operator()(const LLDispatcher* dispatcher, const std::string& key,
		const LLUUID& invoice, const sparam_t& string);

   static const LLString smMessageID;
};

//! Dispatcher for particle effect generic message.
class EffectStationaryParticleDispatchHandler : public LLDispatchHandler
{
public:
   //! Handle dispatch
	virtual bool operator()(const LLDispatcher* dispatcher, const std::string& key,
		const LLUUID& invoice, const sparam_t& string);

   static const LLString smMessageID;
};

class GenericClientEffect// : public LLEventTimer
{
   friend class EffectParticleDispatchHandler;
   friend class EffectStationaryParticleDispatchHandler;

public:
   explicit GenericClientEffect(F32 timeUntilLaunch);
   virtual ~GenericClientEffect();

   virtual void update() {}
   virtual LLUUID getID() const { return mID; }
   virtual void removeParticleSystem() = 0;

protected:
   //! Asset id used by the effect
   LLUUID mID;
   F32 mTimeUntilDeath;
   LLVector3 mPosition;
   LLQuaternion mRotation;
   F32 mSpeed;
};
typedef boost::shared_ptr<GenericClientEffect> GenericClientEffectPtr;

//! Handles client-side special effects sent from server
/*! Currently only handles particle effects sent from server using generic message.

    \note call registerHandlers() at program initialization time.
*/
class RexClientEffect
{
public:
   class ParticleEffect : public GenericClientEffect, public LLEventTimer
   {
   public:
      explicit ParticleEffect(F32 timeUntilLaunch, const LLUUID &id);
      virtual ~ParticleEffect();

      virtual BOOL tick();

      //! removes the particle system from scene
      virtual void removeParticleSystem();

   protected:
      //! creates the particle system and adds it to scene
      virtual BOOL createParticleSystem();

      Ogre::ParticleSystem *mParticleSystem;
      Ogre::SceneNode *mSceneNode;
      Ogre::Animation *mAnimation;
      Ogre::AnimationState *mAnimationState;
   };
   typedef boost::shared_ptr<ParticleEffect> ParticleEffectPtr;

   class CameraParticleEffect : public GenericClientEffect, public RexOgreAssetNotifiable
   {
   public:
      explicit CameraParticleEffect(const Ogre::Camera *camera, const LLUUID &id, const LLQuaternion &rot);
      virtual ~CameraParticleEffect();

      virtual void onOgreAssetLoaded(LLAssetType::EType assetType, const LLUUID& uuid);
      void createParticleSystem(RexParticleScript* script);
      //! removes the particle system from scene
      virtual void removeParticleSystem();


      virtual void update();
   protected:

      //! camera
      const Ogre::Camera *mCamera;
      Ogre::ParticleSystem *mParticleSystem;
      Ogre::SceneNode *mSceneNode;
   };

   //! default constructor
   RexClientEffect() {}

   //! destructor
   ~RexClientEffect() {}

   static void update()
   {
      ClientEffectContainer::iterator iter = smClientEffects.begin();
      for ( ; iter != smClientEffects.end() ; ++iter)
      {
         (*iter)->update();
      }
   }

   static bool hasEffect(GenericClientEffect *effect)
   {
      return (smClientEffects.find(effect) != smClientEffects.end());
   }

   static void addEffect(GenericClientEffect *effect)
   {
      smClientEffects.insert(effect);
   }

   static void removeEffect(GenericClientEffect *effect)
   {
      smClientEffects.erase(effect);
   }

   //! remove all effects that uses the specified id
   static void removeEffect(const LLUUID &id);

   //! Register generic message handlers. Must be called at program initialization time.
   static void registerHandlers();

private:
   typedef std::set<GenericClientEffect*> ClientEffectContainer;

   //! list of client effects
   static ClientEffectContainer smClientEffects;

   //! Generic message handler for particle effects
   static EffectParticleDispatchHandler smDispatchEffectParticle;
   static EffectStationaryParticleDispatchHandler smDispatchEffectStaticParticle;
};

#endif // LL_REXCLIENTEFFECT_H

