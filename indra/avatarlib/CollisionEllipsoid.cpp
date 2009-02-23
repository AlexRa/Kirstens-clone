// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "CollisionEllipsoid.h"

namespace Rex
{
   CollisionEllipsoid::CollisionEllipsoid() : 
      CollisionShape()
//    , Center(Ogre::Vector3::ZERO)
    , Radius(Ogre::Vector3::UNIT_SCALE)
    , Friction(0.1f)
   {
   }

   CollisionEllipsoid::CollisionEllipsoid(const Ogre::Vector3 &center, const Ogre::Vector3 &size) : 
      CollisionShape()
//      , Center(center)
      , Radius(size)
      , Friction(0.1f)
   {
      Position = center;
   }

   CollisionEllipsoid::~CollisionEllipsoid()
   {
   }

   void CollisionEllipsoid::createDebugShape(Ogre::SceneManager *sceneManager)
   {
      if (!DebugEntity)
      {
//         DebugEntity = sceneManager->createEntity(CollisionShape::generateUniqueDebugShapeName(),
//            Ogre::SceneManager::PT_SPHERE);
         Ogre::String name = CollisionShape::generateUniqueDebugShapeName();
#ifdef D_COLLISIONSHAPES
         DebugEntity = sceneManager->createEntity(name, "dummy_sphere.mesh");
#else
         DebugEntity = sceneManager->createManualObject(name);
#endif
      }

      DebugEntity->setVisible(false);

      //if (!DebugNode)
      //{
      //   Ogre::SceneNode *node = sceneManager->getRootSceneNode()->createChildSceneNode();
      //   node->attachObject(DebugEntity);

      //   DebugNode = node;
      //}

      //if (DebugNode)
      //{
      //   if (DebugEntity->isAttached() == false)
      //   {
      //      static_cast<Ogre::SceneNode*>(DebugNode)->attachObject(DebugEntity);
      //   }
      //   DebugNode->setScale(Radius * 0.019f);
      //}

      //setSize(Radius);
   }

   Ogre::Vector3 CollisionEllipsoid::collide(const Ogre::Vector3 &vertex, const Ogre::Vector3 &velocity)
   {
      Ogre::Vector3 transformedVertex = vertex / Radius;
      Ogre::Vector3 center = Position / Radius;
      if (transformedVertex.squaredDistance(center) < 1.f)
      {
         Ogre::Real distance = transformedVertex.distance(center);
//       if (distance < 1.f)
//         {
//         vertex = vertexPrevFrame;
//         Ogre::Vector3 collisionNormalVelocity = (vertexPrevFrame - vertex) / Radius; // Collision normal along velocity
         Ogre::Vector3 collisionNormalVelocity = velocity / Radius; // Collision normal along velocity
         Ogre::Vector3 collisionNormal = (transformedVertex - center); // Collision normal with proper sliding
         collisionNormalVelocity.normalise();
         collisionNormal.normalise();
         Ogre::Quaternion quat = collisionNormal.getRotationTo(collisionNormalVelocity);
         quat = Ogre::Quaternion::Slerp(Friction, quat, Ogre::Quaternion::IDENTITY, true);
         collisionNormal = quat * collisionNormal;
         

         transformedVertex += collisionNormal * (abs(1.f - distance)) * 1.05f;
         return (transformedVertex * Radius);
//         }
      }

      return vertex;
   }

   void CollisionEllipsoid::setSize(const Ogre::Vector3 &size)
   {
      Radius = size;

      if (DebugNode)
   //      DebugNode->setScale(Radius * 0.019f);
         DebugNode->setScale(Radius*2);
   }

   const Ogre::Vector3 &CollisionEllipsoid::getSize() const
   {
      return Radius;
   }

   void CollisionEllipsoid::setFriction(float friction)
   {
      Friction = std::min(std::max(0.0f, friction), 1.f);
   }

   float CollisionEllipsoid::getFriction() const
   {
      return Friction;
   }

   Ogre::Real CollisionEllipsoid::getBoundingRadius() const
   {
      return std::max(std::max(Radius.x, Radius.y), Radius.z);
   }
}

