// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __CollisionCapsule_h__
#define __CollisionCapsule_h__

#include "CollisionShape.h"

namespace Rex
{
   //! Capsule collision shape for fast and simple collision detection / handling.
   /*! The capsule is defined by a (normalised) up vector, radius (xyz), height and center point (position).

       collide() - function does collision detection and handling for a vertex.
   */
   class CollisionCapsule : public CollisionShape
   {
   public:
      //! default constructor
      CollisionCapsule();

      //! destructor
      virtual ~CollisionCapsule();

      //! Creates a visible debug shape.
      /*!
          \note Call before using any other function
      */
      virtual void createDebugShape(Ogre::SceneManager *sceneManager);

      //! Collides vertex against the collision shape
      /*!
          \param vertex position to collide with
          \param velocity current velocity of the vertex
          \return end position of the vertex after potential collision
      */
      virtual Ogre::Vector3 collide(const Ogre::Vector3 &vertex, const Ogre::Vector3 &velocity);

      //! Attach to scenenode
      virtual void attach(Ogre::SceneNode *node,
            const Ogre::Quaternion &offsetOrientation = Ogre::Quaternion::IDENTITY, 
            const Ogre::Vector3 &offsetPosition = Ogre::Vector3::ZERO);

      //! set height of the capsule
      void setHeight(Ogre::Real height);

      //! Set radius for the capsule
      void setRadius(const Ogre::Vector3 &radius);

      //! Shows or hides the debug shape
      virtual void setDebugShapeVisible(bool visible);

   protected:
//      //! Height of the capsule
      Ogre::Real Height;
      //! Size of the capsule
      Ogre::Vector3 Radius;

      //! Debug entity for the caps
      Ogre::Entity *TopCapEntity;
      Ogre::Entity *BottomCapEntity;

      //! Debug scenenodes for the caps
      Ogre::SceneNode *BottomCapNode;
      Ogre::SceneNode *TopCapNode;
   };
}

#endif // __CollisionCapsule_h__
