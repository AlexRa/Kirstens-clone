// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "CollisionCapsule.h"

namespace Rex
{
   CollisionCapsule::CollisionCapsule() : 
      CollisionShape()
       , Radius(Ogre::Vector3::UNIT_SCALE)
       , Height(1.0f)
       , TopCapEntity(0)
       , BottomCapEntity(0)
       , TopCapNode(0)
       , BottomCapNode(0)
   {
   }


   CollisionCapsule::~CollisionCapsule()
   {
   }

   // virtual
   void CollisionCapsule::createDebugShape(Ogre::SceneManager *sceneManager)
   {
      if (!DebugEntity)
      {
         Ogre::String name = CollisionShape::generateUniqueDebugShapeName();
#ifdef D_COLLISIONSHAPES
         DebugEntity = sceneManager->createEntity(name, "dummy_cylinder.mesh");
         TopCapEntity = sceneManager->createEntity(CollisionShape::generateUniqueDebugShapeName(), "dummy_sphere.mesh");
         BottomCapEntity = sceneManager->createEntity(CollisionShape::generateUniqueDebugShapeName(), "dummy_sphere.mesh");
#else
         DebugEntity = sceneManager->createManualObject(name);
#endif
      }

      DebugEntity->setVisible(false);
   }

   // virtual
   Ogre::Vector3 CollisionCapsule::collide(const Ogre::Vector3 &vertex, const Ogre::Vector3 &velocity)
   {
      // Define the planes of top and bottom end of the capsule and see if the vertex is between them. If not, no collision is possible

      Ogre::Vector3 topPoint = Position + Ogre::Vector3::UNIT_Y * (Height * 0.5f);
      Ogre::Vector3 bottomPoint = Position + Ogre::Vector3::NEGATIVE_UNIT_Y * (Height * 0.5f);

      // see if vertex is between planes defined by the ends of the capsule
      Ogre::Plane topPlane(Ogre::Vector3::UNIT_Y, topPoint);
      Ogre::Plane bottomPlane(Ogre::Vector3::NEGATIVE_UNIT_Y, bottomPoint);

      if (topPlane.getSide(vertex) == Ogre::Plane::NEGATIVE_SIDE &&
          bottomPlane.getSide(vertex) == Ogre::Plane::NEGATIVE_SIDE)
      {
         // We use unit sphere testing now. First we need to transform the vertex and the capsule from world space to unit sphere space.
//         topPoint = Position / Radius + Ogre::Vector3::UNIT_Y * (Height * 0.5f);
//         bottomPoint = Position / Radius + Ogre::Vector3::NEGATIVE_UNIT_Y * (Height * 0.5f);
         topPoint /= Radius;
         bottomPoint /= Radius;
         
         Ogre::Vector3 transformedVertex = vertex / Radius;

         bottomPlane.redefine(Ogre::Vector3::UNIT_Y, bottomPoint);
         Ogre::Vector3 collisionSphereCenter = bottomPoint + Ogre::Vector3::UNIT_Y * bottomPlane.getDistance(transformedVertex);
         assert(bottomPlane.getSide(transformedVertex) == Ogre::Plane::POSITIVE_SIDE);

//         assert(collisionSphereCenter.y > bottomPoint.y + 1.f);
         //Ogre::LogManager::getSingleton().logMessage("bottom: " + Ogre::StringConverter::toString(bottomPoint.y));
         //Ogre::LogManager::getSingleton().logMessage("colsph: " + Ogre::StringConverter::toString(collisionSphereCenter.y));
         //Ogre::LogManager::getSingleton().logMessage("vertex: " + Ogre::StringConverter::toString(transformedVertex.y));
         
         
         // Cap the unit sphere to cylinder caps, so we get rounded ends
         collisionSphereCenter.y = std::min(std::max(collisionSphereCenter.y, bottomPoint.y + 1.0f), topPoint.y - 1.0f);
//         collisionSphereCenter.y = std::max(collisionSphereCenter.y, bottomPoint.y + 1.0f);

         if (transformedVertex.squaredDistance(collisionSphereCenter) < 1.0f)
         {
            Ogre::Real distance = transformedVertex.distance(collisionSphereCenter);
 
//             Ogre::Real distance = transformedVertex.squaredDistance(bottomPoint);
//            Ogre::Vector3 collisionNormalVelocity = velocity / Radius; // Collision normal along velocity
            Ogre::Vector3 collisionNormal = (transformedVertex - collisionSphereCenter); // Collision normal with proper sliding
            //collisionNormalVelocity.normalise();
            collisionNormal.normalise();
            //Ogre::Quaternion quat = collisionNormal.getRotationTo(collisionNormalVelocity);
            //quat = Ogre::Quaternion::Slerp(Friction, quat, Ogre::Quaternion::IDENTITY, true);
            //collisionNormal = quat * collisionNormal;
            

//            transformedVertex += collisionNormal * (abs(Radius.x * 0.5f - distance));
            transformedVertex += collisionNormal * (abs(1.f - distance));

            return transformedVertex * Radius;
         }
      }
      
      return vertex;
   }

   void CollisionCapsule::attach(Ogre::SceneNode *node,
            const Ogre::Quaternion &offsetOrientation, 
            const Ogre::Vector3 &offsetPosition)
   {
//      CollisionShape::attach(node, offsetOrientation, offsetPosition);

      if (DebugNode)
      {
         Ogre::LogManager::getSingleton().logMessage("CollisionCapsule::attach: already attached.");
         return;
      }

      Ogre::SceneNode *newNode = node->createChildSceneNode(offsetPosition, offsetOrientation);
      newNode->attachObject(DebugEntity);
      DebugNode = newNode;

#ifdef D_COLLISIONSHAPES
      if (TopCapEntity && BottomCapEntity)
      {
         TopCapNode = newNode->createChildSceneNode(Ogre::Vector3::NEGATIVE_UNIT_Y * Height * 0.5f, offsetOrientation);
         TopCapNode->attachObject(TopCapEntity);
         TopCapNode->setInheritScale(false);
         TopCapNode->setVisible(false);
         BottomCapNode = newNode->createChildSceneNode(Ogre::Vector3::UNIT_Y * Height * 0.5f, offsetOrientation);
         BottomCapNode->attachObject(BottomCapEntity);
         BottomCapNode->setInheritScale(false);
         BottomCapNode->setVisible(false);
      }
#endif


      setPosition(offsetPosition);
      setOrientation(offsetOrientation);
   }

   void CollisionCapsule::setHeight(Ogre::Real height)
   {
      Height = height;
      DebugNode->setScale(Ogre::Vector3(Radius.x * 2.0f, Height - Radius.y * 2.f, Radius.z * 2.0f));
      if (TopCapNode && BottomCapNode)
      {
         TopCapNode->setScale(Radius * 2.f);
         BottomCapNode->setScale(Radius * 2.f);
      }
   }

   void CollisionCapsule::setRadius(const Ogre::Vector3 &radius)
   {
      Radius = radius;
      DebugNode->setScale(Ogre::Vector3(radius.x * 2.0f, Height - Radius.y * 2.f, radius.z * 2.0f));

      if (TopCapNode && BottomCapNode)
      {
         TopCapNode->setScale(Radius * 2.f);
         BottomCapNode->setScale(Radius * 2.f);
      }
   }

   void CollisionCapsule::setDebugShapeVisible(bool visible)
   {
      CollisionShape::setDebugShapeVisible(visible);
#ifdef D_COLLISIONSHAPES
      assert(TopCapEntity && BottomCapEntity);
      TopCapEntity->setVisible(visible);
      BottomCapEntity->setVisible(visible);
#endif
   }
}

