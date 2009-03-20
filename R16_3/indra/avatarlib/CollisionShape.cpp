// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "CollisionShape.h"
#include <OGRE/OgreTagPoint.h>

namespace Rex
{
   CollisionShape::CollisionShape() : 
      DebugEntity(0)
      , DebugNode(0)
      , CollisionNode(0)
      , Position(Ogre::Vector3::ZERO)
      , Orientation(Ogre::Quaternion::IDENTITY)
   {
   }

   CollisionShape::~CollisionShape()
   {
      // We let Ogre take care of removing scenenode / entity.
   }

   void CollisionShape::attach(Ogre::Entity *entity, const Ogre::String &boneName,
            const Ogre::Quaternion &offsetOrientation, 
            const Ogre::Vector3 &offsetPosition)
   {
      assert(entity);
      assert(DebugEntity->isAttached() == false);
      
      try
      {
         if (DebugEntity)
         {
            DebugNode = static_cast<Ogre::Node*>(entity->attachObjectToBone(boneName, DebugEntity, offsetOrientation, offsetPosition));
            DebugNode->setInheritOrientation(true);
         }
      } catch(Ogre::Exception)
      {
         Ogre::LogManager::getSingleton().logMessage("CollisionShape::attach: Failed to attach collision shape to bone: " + boneName +
            " in entity: " + entity->getName());
      }
   }

   void CollisionShape::attach(Ogre::SceneNode *node,
            const Ogre::Quaternion &offsetOrientation, 
            const Ogre::Vector3 &offsetPosition)
   {
      if (DebugNode)
      {
         Ogre::LogManager::getSingleton().logMessage("CollisionShape::attach: already attached.");
         return;
      }

      Ogre::SceneNode *newNode = node->createChildSceneNode(offsetPosition, offsetOrientation);
      newNode->attachObject(DebugEntity);
      DebugNode = newNode;


      setPosition(offsetPosition);
      setOrientation(offsetOrientation);
   }

   void CollisionShape::detach(Ogre::Entity *entity)
   {
      assert(entity);
      assert(DebugEntity);
      assert(DebugEntity->isAttached() == true);


      try
      {
         entity->detachObjectFromBone(DebugEntity);
         if (DebugEntity)
         {
            entity->detachObjectFromBone(DebugEntity);
         }
      } catch (Ogre::Exception)
      {
         assert(false);
         Ogre::LogManager::getSingleton().logMessage("CollisionShape::detach: Failed to detach collision shape from entity: " + entity->getName());
      }
   }

   void CollisionShape::detach(Ogre::SceneNode *node)
   {
      assert(node);
      assert(DebugNode);

      try
      {
         node->removeAndDestroyChild(DebugNode->getName());
      } catch (Ogre::Exception e)
      {
         Ogre::LogManager::getSingleton().logMessage("CollisionShape::detach: Failed to detach collision shape from node: " + node->getName());
      }
   }

   //static
   Ogre::String CollisionShape::generateUniqueDebugShapeName()
   {
      static unsigned int num = 0;

      return Ogre::String("Collision_debug_shape" + Ogre::StringConverter::toString(num++));
   }

   void CollisionShape::setPosition(const Ogre::Vector3 &position)
   {
      Position = position;
   }

   const Ogre::Vector3 &CollisionShape::getPosition() const
   {
      return Position;
   }

   void CollisionShape::setOrientation(const Ogre::Quaternion &orientation)
   {
      Orientation = orientation;
   }

   const Ogre::Quaternion &CollisionShape::getOrientation() const
   {
      return Orientation;
   }

   //virtual
   void CollisionShape::setDebugShapeVisible(bool visible)
   {
#ifdef D_COLLISIONSHAPES
      assert(DebugEntity);
      DebugEntity->setVisible(visible);
#endif
   }

   Ogre::Real CollisionShape::getBoundingRadius() const
   {
      return 0.f;
   }

   Ogre::Real CollisionShape::getSquaredViewDepth(const Ogre::Camera *camera) const
   {
      return camera->getDerivedPosition().squaredLength();
   }
}

