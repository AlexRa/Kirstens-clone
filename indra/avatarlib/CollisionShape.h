// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __CollisionShape_h__
#define __CollisionShape_h__

#include <OGRE/Ogre.h>

namespace Rex
{
   //! A simple collision shape for fast collision detection. Does collision detection and handling.
   class CollisionShape// : public Ogre::SimpleRenderable
   {
   public:
      //! default constructor
      CollisionShape();

      //! destructor
      virtual ~CollisionShape();

      //! Collides vertex against the collision shape
      /*!
          \param vertex position to collide with
          \param velocity current velocity of the vertex
          \return end position of the vertex after potential collision
      */
      virtual Ogre::Vector3 collide(const Ogre::Vector3 &vertex, const Ogre::Vector3 &velocity) = 0;

      //! Attach to bone in entity
      virtual void attach(Ogre::Entity *entity, const Ogre::String &boneName,
            const Ogre::Quaternion &offsetOrientation = Ogre::Quaternion::IDENTITY, 
            const Ogre::Vector3 &offsetPosition = Ogre::Vector3::ZERO);

      //! Attach to scenenode
      virtual void attach(Ogre::SceneNode *node,
            const Ogre::Quaternion &offsetOrientation = Ogre::Quaternion::IDENTITY, 
            const Ogre::Vector3 &offsetPosition = Ogre::Vector3::ZERO);

      //! Detach from bone in entity
      virtual void detach(Ogre::Entity *entity);

      //! Detach from scenenode
      virtual void detach(Ogre::SceneNode *node);

      //! Set position
      virtual void setPosition(const Ogre::Vector3 &position);

      //! get position
      virtual const Ogre::Vector3 &getPosition() const;

      //! set orientation
      virtual void setOrientation(const Ogre::Quaternion &orientation);

      //! Get orientation
      virtual const Ogre::Quaternion &getOrientation() const;

      //! Creates a visible debug shape.
      /*!
          \note Call before using any other function
      */
      virtual void createDebugShape(Ogre::SceneManager *sceneManager) = 0;

      //! Shows or hides the debug shape
      virtual void setDebugShapeVisible(bool visible);

      virtual Ogre::Real getBoundingRadius() const;
      Ogre::Real getSquaredViewDepth(const Ogre::Camera *camera) const;

   protected:
      static Ogre::String generateUniqueDebugShapeName();

      //! Entity for collision debug shape
      Ogre::MovableObject *DebugEntity;

      //! Node for collision debug shape
      Ogre::Node *DebugNode;

      //! Node for collision
      Ogre::Node *CollisionNode;

      //! Position
      Ogre::Vector3 Position;

      //! Orientation
      Ogre::Quaternion Orientation;
   };
}

#endif // __CollisionShape_h__
