// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __CollisionEllipsoid_h__
#define __CollisionEllipsoid_h__

#include "CollisionShape.h"

namespace Rex
{
   //! Ellipsoid collision shape for fast and simple collision detection / handling.
   /*! The ellipsoid is simply defined by radius (xyz) and center point (position).

       collide() - function does collision detection and handling for a vertex.
   */
   class CollisionEllipsoid : public CollisionShape
   {
   public:
      //! default constructor
      CollisionEllipsoid();

      //! Constructs ellipsoid with the specified size
      CollisionEllipsoid(const Ogre::Vector3 &center, const Ogre::Vector3 &size);

      //! destructor
      virtual ~CollisionEllipsoid();

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

      //! set size
      void setSize(const Ogre::Vector3 &size);

      //! get size
      const Ogre::Vector3 &getSize() const;

      //! set friction. A value between 0.0 - 1.0 where 1 is no friction
      void setFriction(float friction);

      //! get friction
      float getFriction() const;

      virtual Ogre::Real getBoundingRadius() const;

   protected:

      //! Center point of the ellipsoid
//      Ogre::Vector3 Center;

      //! Radius of the ellipsoid
      Ogre::Vector3 Radius;

      //! surface friction
      float Friction;
   };
}

#endif // __CollisionEllipsoid_h__
