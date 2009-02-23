// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __DynamicAnimation_h__
#define __DynamicAnimation_h__

#ifdef LL_LINUX
#undef Status
#endif

#include <OGRE/Ogre.h>
//#include <OGRE/OgreResourceManager.h>
#include "XmlFile.h"

namespace Rex
{
   // forward decl
   struct AnimatedEntity;
   class DynamicAnimationPtr;

   //! Bone modification mode
   enum BoneModificationMode
   {
       MODIFY_ABSOLUTE = 0,
       MODIFY_RELATIVE,
       MODIFY_CUMULATIVE
   };

   //! Bone for dynamic animation
   struct DynamicAnimationBone
   {
      //! Bone name in skeleton
      Ogre::String Name;

      //! Bone rotation, start and end
      std::pair<Ogre::Vector3, Ogre::Vector3> Rotation;

      //! Bone translation, start and end
      std::pair<Ogre::Vector3, Ogre::Vector3> Translation;

      //! Bone scale, start and end
      std::pair<Ogre::Vector3, Ogre::Vector3> Scale;

      //! Translation mode (absolute, or relative to original position in original skeleton)
      BoneModificationMode TranslationMode;

      //! Rotation mode (absolute, or relative to original orientation)
      BoneModificationMode RotationMode;

      //! Default constructor
      DynamicAnimationBone() : TranslationMode(MODIFY_ABSOLUTE), RotationMode(MODIFY_ABSOLUTE) {};
   };

   //! Vector of bones, typedef'ed for easier handling.
   typedef std::vector<DynamicAnimationBone> BoneVector;

   //! Dynamic animation.
   /*! Modifies animations in real-time to generate new animations.

       \note Should not be applied in real-time, as the resulting animation will look bad
             and because implementation is not performance-optimised.
   */
   class DynamicAnimation : public Ogre::Resource
   {
   public:
      //! constructor
   //   DynamicAnimation() {}
      DynamicAnimation(Ogre::ResourceManager *creator, const Ogre::String &name, Ogre::ResourceHandle handle,
         const Ogre::String &group, bool isManual = false, Ogre::ManualResourceLoader *loader = 0);

      //! destructor
      ~DynamicAnimation();// {}

      //! Assignment operator to allow easy copying between materials.
      DynamicAnimation& operator=( const DynamicAnimation& rhs );

      //! Reset the animation
      void clear();

      //! Load animation from xml element
      bool load(XmlElement *element);

      //! Exports animation under xml node
      /*!
          \param root xml node under which animation is exported
      */
      void exportTo(XmlNode *root);

      //! Returns animation bones
      const BoneVector &getBones() const;
      //! Returns animation bones
      BoneVector &getBones();

      //! Adds a bone to animation
      void addBone(const DynamicAnimationBone &bone);

      //! Returns base animations
      const Ogre::StringVector &getBaseAnimations() const;
      //! Returns base animations
      Ogre::StringVector &getBaseAnimations();

      //! Adds a baseanimation
      /*!
          \param name Base animation name
      */
      void addBaseAnimation(const Ogre::String &name);

      //! Apply the animation to all entities affected by this animation, at position pos
      /*!
          \param pos Position for the animation, in range 0-1
      */
      void apply(float pos);

      //! Apply the animation to entity at position pos
      /*! Mostly internal function, use apply(pos) instead to update/apply to all entities

          \param ent Entity to apply the animation to
      */
      void apply(Ogre::Entity *ent, float pos, const AnimatedEntity *animatedEntity);

      void applyToBone(Ogre::Entity *ent, float pos, const AnimatedEntity *animatedEntity);

      //! Get current animation position, [0,1]
      float getPosition() const;

      //! Set animation position, [0,1]
      void setPosition(float pos);

      //! Clone animation
      /*!
         \param newName The name for the cloned material
	   	\param changeGroup If true, the resource group of the clone is changed
		   \param newGroup Only required if changeGroup is true; the new group to assign
      */
      DynamicAnimationPtr clone(const Ogre::String &newName, bool changeGroup = false,
                                const Ogre::String &newGroup = Ogre::StringUtil::BLANK) const;

   protected:
      void loadImpl();
      void unloadImpl();
      size_t calculateSize() const;

      //! Base animation(s) this dynamic animation modifies
      Ogre::StringVector BaseAnimation;
      
      //! List of bones affected by this animation
      BoneVector Bones;

      // Position of the animation
      float Position;
   };

   //! Specialisation of SharedPtr to allow SharedPtr to be assigned to DynamicAnimationPtr.
   /*! \note Has to be a subclass since we need operator=. We could templatise this instead of repeating per Resource subclass, except to do so requires a form VC6 does not support i.e. ResourceSubclassPtr<T> : public SharedPtr<T>
   */
   class DynamicAnimationPtr : public Ogre::SharedPtr<DynamicAnimation>
   {
   public:
      DynamicAnimationPtr() : Ogre::SharedPtr<DynamicAnimation>() {}
      explicit DynamicAnimationPtr(DynamicAnimation *rep) : Ogre::SharedPtr<DynamicAnimation>(rep) {}
      DynamicAnimationPtr(const DynamicAnimationPtr &rep) : Ogre::SharedPtr<DynamicAnimation>(rep) {}
      DynamicAnimationPtr(const Ogre::ResourcePtr &r) : Ogre::SharedPtr<DynamicAnimation>()
      {
         // lock & copy other mutex pointer
         OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
            OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<DynamicAnimation*>(r.getPointer());
         pUseCount = r.useCountPointer();
         if (pUseCount)
         {
            ++(*pUseCount);
         }
      }

      /// Operator used to convert a ResourcePtr to a DynamicAnimationPtr
      DynamicAnimationPtr& operator=(const Ogre::ResourcePtr& r)
      {
        if (pRep == static_cast<DynamicAnimation*>(r.getPointer()))
            return *this;
        release();
        // lock & copy other mutex pointer
        OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
            OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<DynamicAnimation*>(r.getPointer());
        pUseCount = r.useCountPointer();
        if (pUseCount)
        {
            ++(*pUseCount);
        }
        return *this;
      }
   };

} // namespace Rex

#endif // __DynamicAnimation_h__
