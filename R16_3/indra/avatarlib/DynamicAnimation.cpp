// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "DynamicAnimation.h"
#include "DynamicAnimationManager.h"
#include "DynamicAnimationSerializer.h"
#include <OGRE/Ogre.h>

using namespace Ogre;

namespace Rex
{
   DynamicAnimation::DynamicAnimation(Ogre::ResourceManager *creator, const Ogre::String &name,
                    Ogre::ResourceHandle handle, const Ogre::String &group,
                    bool isManual, Ogre::ManualResourceLoader *loader) : 
      Ogre::Resource(creator, name, handle, group, isManual, loader), Position(0.5f)
   {
      if (createParamDictionary("DynamicAnimation"))
      {
         ParamDictionary* dict = getParamDictionary();
      }
   }

   DynamicAnimation::~DynamicAnimation()
   {
   }


   DynamicAnimation& DynamicAnimation::operator=( const DynamicAnimation& rhs )
   {
      mName = rhs.mName;
		mGroup = rhs.mGroup;
		mCreator = rhs.mCreator;
		mIsManual = rhs.mIsManual;
		mLoader = rhs.mLoader;
      mHandle = rhs.mHandle;
      mSize = rhs.mSize;
      mLoadingState = rhs.mLoadingState;
		mIsBackgroundLoaded = rhs.mIsBackgroundLoaded;

      Position = rhs.Position;
      size_t n;
      for (n=0 ; n<rhs.BaseAnimation.size() ; ++n)
      {
         BaseAnimation.push_back(rhs.BaseAnimation[n]);
      }

      for (n=0 ; n<rhs.Bones.size() ; ++n)
      {
         Bones.push_back(rhs.Bones[n]);
      }

      assert(isLoaded() == rhs.isLoaded());

      return *this;
   }

   void DynamicAnimation::clear()
   {
      Position = 0.5f;
      BaseAnimation.clear();
      Bones.clear();
   }
   
   //! Load animation from xml element
   bool DynamicAnimation::load(XmlElement *element)
   {
      assert(element);

      DynamicAnimationSerializer serializer;
      return (serializer.importAnimation(element, this));
   }

   void DynamicAnimation::exportTo(XmlNode *root)
   {
      XmlElement animation("dynamic_animation_parameter");
      animation.SetAttribute("name", getName());

      Ogre::String animPos = Ogre::StringConverter::toString(getPosition());
	   animation.SetAttribute("position", animPos);
      root->InsertEndChild(animation);
   }

   void DynamicAnimation::apply(float pos)
   {
      assert(pos >= 0.0 && pos <= 1.0);
      DynamicAnimationManager::getSingleton().setAnimationPosition(getName(), pos);
      setPosition(pos);
   }


   void DynamicAnimation::loadImpl()
   {
   }

   void DynamicAnimation::unloadImpl()
   {
   }

   size_t DynamicAnimation::calculateSize() const
   {
      return 0;
   }

   //! Returns animation bones
   const BoneVector &DynamicAnimation::getBones() const
   {
      return Bones;
   }

   //! Returns animation bones
   BoneVector &DynamicAnimation::getBones()
   {
      return Bones;
   }

   //! Adds a bone to animation
   void DynamicAnimation::addBone(const DynamicAnimationBone &bone)
   {
      Bones.push_back(bone);
   }


   //! Returns base animations
   const Ogre::StringVector &DynamicAnimation::getBaseAnimations() const
   {
      return BaseAnimation;
   }

   //! Returns base animations
   Ogre::StringVector &DynamicAnimation::getBaseAnimations()
   {
      return BaseAnimation;
   }

   void DynamicAnimation::addBaseAnimation(const Ogre::String &name)
   {
      BaseAnimation.push_back(name);
   }

   void DynamicAnimation::apply(Ogre::Entity *ent, float pos, const AnimatedEntity *animatedEntity)
   {
      assert(ent != NULL);
      assert(pos >= 0 && pos <= 1);
      assert(animatedEntity);

//      Ogre::LogManager::getSingleton().logMessage("Entity: " +ent->getName() + ".");
//      Ogre::LogManager::getSingleton().logMessage("Anim: " + this->getName() + ".");

      Ogre::SkeletonInstance *skeleton = ent->getSkeleton();
      size_t n;
      for (n=0 ; n<BaseAnimation.size() ; ++n)
      {
         Ogre::Animation* newAnimation = skeleton->getAnimation(BaseAnimation[n]);
         Ogre::Animation* baseAnimation = animatedEntity->findBaseAnimation(BaseAnimation[n]);

         if (!baseAnimation)
         {
            // Specified base animation not found
            Ogre::LogManager::getSingleton().logMessage("Base animation not found: " + BaseAnimation[n] + ".");
            continue;
         }


         Ogre::Animation::NodeTrackIterator trackIter = newAnimation->getNodeTrackIterator();
         Ogre::Animation::NodeTrackIterator oldTrackIter = baseAnimation->getNodeTrackIterator();
         while (trackIter.hasMoreElements() && oldTrackIter.hasMoreElements())
         {
            Ogre::NodeAnimationTrack* track = trackIter.getNext();
            Ogre::NodeAnimationTrack* oldTrack = oldTrackIter.getNext();

            Ogre::Bone* bone;
            try
            {
               bone = skeleton->getBone(track->getHandle());
            } catch(Ogre::Exception) { continue; }
      
            size_t j;
            for (j=0 ; j<Bones.size() ; ++j)
            {
               DynamicAnimationBone *dynamicBone = &Bones[j];
               if (bone->getName().compare(dynamicBone->Name) == 0)
               {
                  int m;
                  for (m=0 ; m<track->getNumKeyFrames() ; ++m)
                  {
                     // Rotation
                     ///////////
                     Ogre::TransformKeyFrame *newKF = track->getNodeKeyFrame(m);
                     Ogre::TransformKeyFrame *oldKF = oldTrack->getNodeKeyFrame(m);
                     Ogre::Quaternion newRot = newKF->getRotation();
                     Ogre::Quaternion rot_second = newKF->getRotation();
                     Ogre::Quaternion rot = oldKF->getRotation();
                     Ogre::Matrix3 rotMatrix;
                 //    Ogre::Matrix3 newRotMatrix;
                 //    rot.ToRotationMatrix(rotMatrix);
                 //    newRot.ToRotationMatrix(rotMatrix);
                     Ogre::Radian rotX(0);
                     Ogre::Radian rotY(0);
                     Ogre::Radian rotZ(0);
                //     rotMatrix.ToEulerAnglesXYZ(rotX, rotY, rotZ);


//                     rot.

                     Ogre::Matrix3 rotMatrix2;
                     rot_second.ToRotationMatrix(rotMatrix2);
                     Ogre::Radian rotX2;
                     Ogre::Radian rotY2;
                     Ogre::Radian rotZ2;
                     rotMatrix2.ToEulerAnglesXYZ(rotX2, rotY2, rotZ2);
                     
                     if (dynamicBone->Rotation.second.x != 0 || dynamicBone->Rotation.first.x != 0)
                     {
                        rotX += Ogre::Degree((dynamicBone->Rotation.second.x - dynamicBone->Rotation.first.x) * pos + dynamicBone->Rotation.first.x);
                     } else
                        rotX = rotX2;
 //                    else
 //                       rotX = newRot.getPitch() - rot.getPitch();
                     if (dynamicBone->Rotation.second.y != 0 || dynamicBone->Rotation.first.y != 0)
                     {
                        rotY += Ogre::Degree((dynamicBone->Rotation.second.y - dynamicBone->Rotation.first.y) * pos + dynamicBone->Rotation.first.y);
                     } else
                        rotY = rotY2;
//                     else
//                        rotY = newRot.getYaw() - rot.getYaw();
                     if (dynamicBone->Rotation.second.z != 0 || dynamicBone->Rotation.first.z != 0)
                     {
                        rotZ += Ogre::Degree((dynamicBone->Rotation.second.z - dynamicBone->Rotation.first.z) * pos + dynamicBone->Rotation.first.z);
                     } else
                        rotZ = rotZ2;
//                     else
//                        rotZ = newRot.getRoll() - rot.getRoll();


                     rotMatrix.FromEulerAnglesXYZ(rotX, rotY, rotZ);
//                     rot = rot * Ogre::Quaternion(rotMatrix);
                     rot = Ogre::Quaternion(rotMatrix);
                     newKF->setRotation(rot);

                     // Translation
                     //////////////
                     Ogre::Vector3 trans = oldKF->getTranslate();
                     Ogre::Vector3 transDiff(0, 0, 0);
                     
                     if (dynamicBone->Translation.second.x != 0 || dynamicBone->Translation.first.x != 0)
                     //   transDiff.x = pos * (dynamicBone->Translation.second.x - dynamicBone->Translation.first.x) + dynamicBone->Translation.first.x;
                        trans.x += pos * (dynamicBone->Translation.second.x - dynamicBone->Translation.first.x) + dynamicBone->Translation.first.x;
                     else
                        trans.x = newKF->getTranslate().x;
                     if (dynamicBone->Translation.second.y != 0 || dynamicBone->Translation.first.y != 0)
                    //    transDiff.y = pos * (dynamicBone->Translation.second.y - dynamicBone->Translation.first.y) + dynamicBone->Translation.first.y;
                        trans.y += pos * (dynamicBone->Translation.second.y - dynamicBone->Translation.first.y) + dynamicBone->Translation.first.y;
                     else
                        trans.y = newKF->getTranslate().y;
                     if (dynamicBone->Translation.second.z != 0 || dynamicBone->Translation.first.z != 0)
                        //transDiff.z = pos * (dynamicBone->Translation.second.z - dynamicBone->Translation.first.z) + dynamicBone->Translation.first.z;
                        trans.z += pos * (dynamicBone->Translation.second.z - dynamicBone->Translation.first.z) + dynamicBone->Translation.first.z;
                     else
                        trans.z = newKF->getTranslate().z;

               //      trans += transDiff;
                     newKF->setTranslate(trans);

                     // Scale
                     //////////////
                     Ogre::Vector3 scale = newKF->getScale();
//                     Ogre::Vector3 scaleDiff(1, 1, 1);
                     Ogre::Vector3 scaleDiff = oldKF->getScale();
                     
                     if (dynamicBone->Scale.second.x != 0 || dynamicBone->Scale.first.x != 0)
                        scaleDiff.x = pos * (dynamicBone->Scale.second.x - dynamicBone->Scale.first.x) + dynamicBone->Scale.first.x;
                     if (dynamicBone->Scale.second.y != 0 || dynamicBone->Scale.first.y != 0)
                        scaleDiff.y = pos * (dynamicBone->Scale.second.y - dynamicBone->Scale.first.y) + dynamicBone->Scale.first.y;
                     if (dynamicBone->Scale.second.z != 0 || dynamicBone->Scale.first.z != 0)
                        scaleDiff.z = pos * (dynamicBone->Scale.second.z - dynamicBone->Scale.first.z) + dynamicBone->Scale.first.z;

                     if (Ogre::Math::RealEqual(scaleDiff.x, 0) || Ogre::Math::RealEqual(scaleDiff.x, oldKF->getScale().x))
                        scaleDiff.x = scale.x;
                     if (Ogre::Math::RealEqual(scaleDiff.y, 0) || Ogre::Math::RealEqual(scaleDiff.y, oldKF->getScale().y))
                        scaleDiff.y = scale.y;
                     if (Ogre::Math::RealEqual(scaleDiff.z, 0) || Ogre::Math::RealEqual(scaleDiff.z, oldKF->getScale().z))
                        scaleDiff.z = scale.z;

//                     scale += scaleDiff;
                     scale = scaleDiff;
                     newKF->setScale(scale);
                  }
               }
            }
         }
      }
   }
   
   void DynamicAnimation::applyToBone(Ogre::Entity *ent, float pos, const AnimatedEntity *animatedEntity)
   {
      assert(ent != NULL);
      assert(pos >= 0 && pos <= 1);
      assert(animatedEntity);

//      Ogre::LogManager::getSingleton().logMessage("Entity: " +ent->getName() + ".");
//      Ogre::LogManager::getSingleton().logMessage("Anim: " + this->getName() + ".");

      if (ent->hasSkeleton() == false)
         return;

      Ogre::SkeletonInstance *skeleton = ent->getSkeleton();

      // Find bone from the original source skeleton as well, so we can get the original initial state for relative translation
      const Ogre::SkeletonPtr& origSkeleton = ent->getMesh()->getSkeleton();
      if (origSkeleton.isNull()) return;

      //size_t n;
      //for (n=0 ; n<BaseAnimation.size() ; ++n)
      //{
      //   Ogre::Animation* newAnimation = skeleton->getAnimation(BaseAnimation[n]);
      //   Ogre::Animation* baseAnimation = animatedEntity->findBaseAnimation(BaseAnimation[n]);

      //   if (!baseAnimation)
      //   {
      //      // Specified base animation not found
      //      Ogre::LogManager::getSingleton().logMessage("Base animation not found: " + BaseAnimation[n] + ".");
      //      continue;
      //   }


      //   Ogre::Animation::NodeTrackIterator trackIter = newAnimation->getNodeTrackIterator();
      //   Ogre::Animation::NodeTrackIterator oldTrackIter = baseAnimation->getNodeTrackIterator();
      //   while (trackIter.hasMoreElements() && oldTrackIter.hasMoreElements())
      //   {
      //      Ogre::NodeAnimationTrack* track = trackIter.getNext();
      //      Ogre::NodeAnimationTrack* oldTrack = oldTrackIter.getNext();
      //      Ogre::Bone* bone = skeleton->getBone(track->getHandle());
      
            size_t j;
            for (j=0 ; j<Bones.size() ; ++j)
            {
               DynamicAnimationBone *dynamicBone = &Bones[j];
               Ogre::Bone* bone = 0;
               Ogre::Bone* origBone = 0;
               try
               {
                  bone = skeleton->getBone(dynamicBone->Name);
                  origBone = origSkeleton->getBone(dynamicBone->Name);
               } catch (Ogre::Exception e)
               {
                  // bone not found, nothing to do
                  continue;
               }
           //    if (bone->getName().compare(dynamicBone->Name) == 0)
           //    {
                  //int m;
                  //for (m=0 ; m<track->getNumKeyFrames() ; ++m)
                  //{
                     // Rotation
                     ///////////
             //        Ogre::TransformKeyFrame *newKF = track->getNodeKeyFrame(m);
             //        Ogre::TransformKeyFrame *oldKF = oldTrack->getNodeKeyFrame(m);
                     Ogre::Quaternion newRot = bone->getInitialOrientation();
                     Ogre::Quaternion rot_second = bone->getInitialOrientation();
                     Ogre::Quaternion rot = bone->getInitialOrientation();
                     Ogre::Matrix3 rotMatrix;
                     Ogre::Radian rotX(0);
                     Ogre::Radian rotY(0);
                     Ogre::Radian rotZ(0);

                     Ogre::Matrix3 rotMatrix2;
                     rot_second.ToRotationMatrix(rotMatrix2);
                     Ogre::Radian rotX2;
                     Ogre::Radian rotY2;
                     Ogre::Radian rotZ2;
                     rotMatrix2.ToEulerAnglesXYZ(rotX2, rotY2, rotZ2);

                     if (dynamicBone->RotationMode == MODIFY_RELATIVE)
                     {
                         rot_second = origBone->getInitialOrientation();
                         rot_second.ToRotationMatrix(rotMatrix2);
                         rotMatrix2.ToEulerAnglesXYZ(rotX2, rotY2, rotZ2);
                         rotX = rotX2;
                         rotY = rotY2;
                         rotZ = rotZ2;
                     }
                     // Very scary mode. To only be used in mastermodifiers that have to affect the same bones multiple times
                     else if (dynamicBone->RotationMode == MODIFY_CUMULATIVE)
                     {
                         rotX = rotX2;
                         rotY = rotY2;
                         rotZ = rotZ2;
                     }

                     if (dynamicBone->Rotation.second.x != 0 || dynamicBone->Rotation.first.x != 0)
                     {
                        rotX += Ogre::Degree((dynamicBone->Rotation.second.x - dynamicBone->Rotation.first.x) * pos + dynamicBone->Rotation.first.x);
                     } else
                        rotX = rotX2;

                     if (dynamicBone->Rotation.second.y != 0 || dynamicBone->Rotation.first.y != 0)
                     {
                        rotY += Ogre::Degree((dynamicBone->Rotation.second.y - dynamicBone->Rotation.first.y) * pos + dynamicBone->Rotation.first.y);
                     } else
                        rotY = rotY2;

                     if (dynamicBone->Rotation.second.z != 0 || dynamicBone->Rotation.first.z != 0)
                     {
                        rotZ += Ogre::Degree((dynamicBone->Rotation.second.z - dynamicBone->Rotation.first.z) * pos + dynamicBone->Rotation.first.z);
                     } else
                        rotZ = rotZ2;

                     rotMatrix.FromEulerAnglesXYZ(rotX, rotY, rotZ);
                     rot = Ogre::Quaternion(rotMatrix);
                     bone->setOrientation(rot);

                     // Translation
                     //////////////
                     Ogre::Vector3 trans = bone->getInitialPosition();
                     Ogre::Vector3 transDiff(0, 0, 0);
                     
                     if (dynamicBone->TranslationMode == MODIFY_ABSOLUTE)
                     {
                         if (dynamicBone->Translation.second.x != 0 || dynamicBone->Translation.first.x != 0)
                        //   transDiff.x = pos * (dynamicBone->Translation.second.x - dynamicBone->Translation.first.x) + dynamicBone->Translation.first.x;
                            trans.x = pos * (dynamicBone->Translation.second.x - dynamicBone->Translation.first.x)  + dynamicBone->Translation.first.x;
                    //     else
                    //        trans.x = bone->getInitialPosition().x;
    
                        if (dynamicBone->Translation.second.y != 0 || dynamicBone->Translation.first.y != 0)
                        //    transDiff.y = pos * (dynamicBone->Translation.second.y - dynamicBone->Translation.first.y) + dynamicBone->Translation.first.y;
                            trans.y = pos * (dynamicBone->Translation.second.y - dynamicBone->Translation.first.y) + dynamicBone->Translation.first.y;
                //      else
                //         trans.y = bone->getInitialPosition().y;
    
                        if (dynamicBone->Translation.second.z != 0 || dynamicBone->Translation.first.z != 0)
                            //transDiff.z = pos * (dynamicBone->Translation.second.z - dynamicBone->Translation.first.z) + dynamicBone->Translation.first.z;
                            trans.z = pos * (dynamicBone->Translation.second.z - dynamicBone->Translation.first.z) + dynamicBone->Translation.first.z;
                //      else
                //         trans.z = bone->getInitialPosition().z;
                     }
                     else
                     {
                        if (dynamicBone->Translation.second.x != 0 || dynamicBone->Translation.first.x != 0)
                           trans.x = origBone->getPosition().x + pos * (dynamicBone->Translation.second.x - dynamicBone->Translation.first.x)  + dynamicBone->Translation.first.x;

                        if (dynamicBone->Translation.second.y != 0 || dynamicBone->Translation.first.y != 0)
                           trans.y = origBone->getPosition().y + pos * (dynamicBone->Translation.second.y - dynamicBone->Translation.first.y)  + dynamicBone->Translation.first.y;

                        if (dynamicBone->Translation.second.z != 0 || dynamicBone->Translation.first.z != 0)
                           trans.z = origBone->getPosition().z + pos * (dynamicBone->Translation.second.z - dynamicBone->Translation.first.z)  + dynamicBone->Translation.first.z;
                     }
                 //    newKF->setTranslate(trans);
                     bone->setPosition(trans);

                     // Scale
                     //////////////
                     Ogre::Vector3 scale = bone->getInitialScale();
//                     Ogre::Vector3 scaleDiff(1, 1, 1);
                     Ogre::Vector3 scaleDiff = bone->getInitialScale();
                     
                     if (dynamicBone->Scale.second.x != 1 || dynamicBone->Scale.first.x != 1)
                        scaleDiff.x = pos * (dynamicBone->Scale.second.x - dynamicBone->Scale.first.x) + dynamicBone->Scale.first.x;
                     if (dynamicBone->Scale.second.y != 1 || dynamicBone->Scale.first.y != 1)
                        scaleDiff.y = pos * (dynamicBone->Scale.second.y - dynamicBone->Scale.first.y) + dynamicBone->Scale.first.y;
                     if (dynamicBone->Scale.second.z != 1 || dynamicBone->Scale.first.z != 1)
                        scaleDiff.z = pos * (dynamicBone->Scale.second.z - dynamicBone->Scale.first.z) + dynamicBone->Scale.first.z;

                     //if (Ogre::Math::RealEqual(scaleDiff.x, 0) || Ogre::Math::RealEqual(scaleDiff.x, bone->getInitialScale().x))
                     //   scaleDiff.x = scale.x;
                     //if (Ogre::Math::RealEqual(scaleDiff.y, 0) || Ogre::Math::RealEqual(scaleDiff.y, bone->getInitialScale().y))
                     //   scaleDiff.y = scale.y;
                     //if (Ogre::Math::RealEqual(scaleDiff.z, 0) || Ogre::Math::RealEqual(scaleDiff.z, bone->getInitialScale().z))
                     //   scaleDiff.z = scale.z;

//                     scale += scaleDiff;
                     scale = scaleDiff;
             //        newKF->setScale(scale);
                     bone->setScale(scale);
//                     bone->setManuallyControlled(true);
//                     bone->setScale(Ogre::Vector3(5, 5, 5));
                     
                     bone->setInitialState();
               //      bone->setBindingPose();
                  }
      //         }
      //      }
      //   }
      //}
   }

   float DynamicAnimation::getPosition() const
   {
      return Position;
   }

      //! Set animation position, [0,1]
   void DynamicAnimation::setPosition(float pos)
   {
      assert(pos >=0.0 && pos <= 1.0);
      Position = pos;
   }

   DynamicAnimationPtr DynamicAnimation::clone(const Ogre::String &newName, bool changeGroup,
                                const Ogre::String &newGroup) const
   {
      DynamicAnimationPtr newAnim;
		if (changeGroup)
		{
			newAnim = DynamicAnimationManager::getSingleton().create(newName, newGroup);
		}
		else
		{
         newAnim = DynamicAnimationManager::getSingleton().create(newName, mGroup);
		}

      // Keep handle (see below, copy overrides everything)
      ResourceHandle newHandle = newAnim->getHandle();
      // Assign values from this
      *newAnim = *this;
		// Restore new group if required, will have been overridden by operator
		if (changeGroup)
		{
			newAnim->mGroup = newGroup;
		}
		
      // Correct the name & handle, they get copied too
      newAnim->mName = newName;
      newAnim->mHandle = newHandle;

      return newAnim;
   }

} // namespace Rex

