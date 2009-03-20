// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __DynamicAnimationManager_h__
#define __DynamicAnimationManager_h__

#include "CommonHeaders.h"
#include "DynamicAnimation.h"
#include <OGRE/Ogre.h>
#include <vector>
#include <map>

//! realXtend Avatar library
namespace Rex
{
   //! Contains animation data for entity
   struct AnimatedEntity
   {
      //! Ogre entity
      Ogre::Entity *Entity;

      //! List of dynamic animations affecting this entity
//      std::vector<DynamicAnimationPtr> Animations;
      std::map<Ogre::String, DynamicAnimationPtr> AnimationMap;

      //! Base animations for entity. 
      //! Dynamic animations work in relation to the underlying base animation,
      //! so we need to know the base animations.
      static std::vector<Ogre::Animation*> BaseAnimations;

      bool operator == (const AnimatedEntity &other) const
      {
         return Entity == other.Entity;
      }
      bool operator != (const AnimatedEntity &other) const
      {
         return !(*this == other);
      }

      //! Update dynamic animation for this entity 
      void update(const Ogre::String &name, float pos)
      {
         if (AnimationMap.find(name) != AnimationMap.end())
            AnimationMap[name]->applyToBone(Entity, pos, this);
//         size_t n;
//         for (n=0 ; n<Animations.size() ; ++n)
//         {
//            if (name.compare(Animations[n]->getName()) == 0)
//            {
//               Animations[n]->apply(Entity, pos, this);
//            }
//         }
      }

      //! Find base animation
      Ogre::Animation *findBaseAnimation(const Ogre::String &name) const
      {
         size_t n;
         for (n=0 ; n<BaseAnimations.size() ; ++n)
         {
            if (name.compare(BaseAnimations[n]->getName()) == 0)
               return BaseAnimations[n];
         }

         return 0;
      }
   };

   //bool AnimatedEntity::operator == (const AnimatedEntity &other) const
   //{
   //   return Entity == other.Entity;
   //}
   //
   //bool AnimatedEntity::operator != (const AnimatedEntity &other) const
   //{
   //   return !(*this == other);
   //}


   //! Dynamic animation manager.
   /*! Loads dynamic animations and applies them to entities.
       Animations can be loaded any time, but must be loaded before applying them to normal animations.
       Names don't work yet, and neither does multiple animations.

       Several dynamic animations can affect single entity, as long as the animations don't
       share same type of transformation on same bone (f.ex. two different rotations on same bone).

       This is a "fake" manager in the sense that it doesn't handle loading resources from disk, it only
       handles resources created programmatically (from dynamic animation xml files). The actual
       files are handled by XmlManager.

       \note Dynamic animations should not be added/modified real-time as performance and results are bad

       \note This could be turned into Ogre resource manager and DynamicAnimation into Ogre resource,
             but that might bring too much baggage.
   */
   class DynamicAnimationManager : public Ogre::ResourceManager, public Ogre::Singleton<DynamicAnimationManager>
   {
   public:
      //! constructor
      DynamicAnimationManager();

      //! destructor
      ~DynamicAnimationManager();

      DynamicAnimationPtr load(const Ogre::String &name, const Ogre::String &group);

      static DynamicAnimationManager &getSingleton();
      static DynamicAnimationManager *getSingletonPtr();

      //! Adds dynamic animation to entity
      /*! Several dynamic animations can affect single entity.

          \param entity Ogre entity
          \param name name of the dynamic animation
          \return False, if animation could not be found, true otherwise
      */
      bool addAnimationToEntity(Ogre::Entity *entity, const Ogre::String &name, bool replace = false);

      //! Removes entity. The entity no longer gets animated by bone deforms.
      /*! \remark Make sure you call this before SceneManager::destroyEntity(), if entity
                  is affected by bone deforms.

          \param entity Ogre entity
          \return True, if entity was removed, false if entity could not be removed or found
      */
      bool removeEntity(Ogre::Entity *entity);

      //! Set position for dynamic animation. Automatically updates all entities that has the animation.
      /*! Internal function. Use DynamicAnimation::apply() instead.

          \param name Name of the animation
          \param pos Animation position, should be in range 0-1.
      */
      void setAnimationPosition(const Ogre::String &name, float pos);

      //! Export all animations
      /*!
          \param root Root xml node under which animations are exported
      */
      void exportAnimations(XmlNode *root);

      //! Returns animated entity based on Ogre entity, or 0 if animated entity not found
      const AnimatedEntity *getAnimatedEntity(const Ogre::Entity *entity) const;


      ////! Update and apply dynamic animation to entity
      ///*!
      //    \name Dynamic animation name
      //    \offset Animation position, should be between 0-1
      //*/
      //void updateDynamicAnimation(const Ogre::String &name, float pos);

      // Script handling
      const Ogre::StringVector &getScriptPatterns() const;
      Ogre::Real getLoadingOrder() const;
      void parseScript(Ogre::DataStreamPtr &stream, const Ogre::String &groupName);

   //   static const Ogre::String DYNAMICANIMATION_RESOURCE_GROUP_NAME;

   protected:

       // must implement this from ResourceManager's interface
       Ogre::Resource *createImpl(const Ogre::String &name, Ogre::ResourceHandle handle, 
           const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader, 
           const Ogre::NameValuePairList *createParams);

   private:
      //! Loads dynamic animation definitions from xml file.
      /*!
          \param filename Xml file
      */ 
      void loadDynamicAnimationDefs(const Ogre::String &filename);


      //! Helper function. Project vector to another vector.
      Ogre::Real projectVector(const Ogre::Real x1, const Ogre::Real x2, const Ogre::Real y1, const Ogre::Real y2);

      //! List of entities that use dynamic animations
      std::vector<AnimatedEntity> AnimatedEntities;

   //   float DynamicAnimationPosition;

      //! File pattern for dynamic animation files
      Ogre::StringVector ScriptPattern;

      //! Relative load order of dynamic animation scripts
      Ogre::Real LoadOrder;

      //! List of all animations cloned by this manager. The animations need to be deleted at some point.
      std::vector<Ogre::Animation*> ClonedAnimations;
   };
} // namespace Rex
#endif // __DynamicAnimationManager_h__
