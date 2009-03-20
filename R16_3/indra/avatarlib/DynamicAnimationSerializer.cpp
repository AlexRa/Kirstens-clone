// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "DynamicAnimationSerializer.h"
#include "DynamicAnimation.h"
#include "XmlManager.h"
#include "tinyxml.h"

using namespace Ogre;

namespace Rex
{
   DynamicAnimationSerializer::DynamicAnimationSerializer()
   {
   }

   DynamicAnimationSerializer::~DynamicAnimationSerializer()
   {
   }

   void DynamicAnimationSerializer::exportAnimation(DynamicAnimation *animation, XmlElement *rootElement)
   {
      assert(rootElement);
      assert(animation);

      // Root element
      XmlElement animation_element("dynamic_animation");
      animation_element.SetAttribute("name", animation->getName());
      

       // Base animations
      XmlElement base_animations("base_animations");
      
      const Ogre::StringVector &baseAnims = animation->getBaseAnimations();
      size_t n;
      for (n=0 ; n<baseAnims.size() ; ++n)
      {
         XmlElement base_animation_element("animation");
         base_animation_element.SetAttribute("name", baseAnims[n]);
         base_animations.InsertEndChild(base_animation_element);
      }
      animation_element.InsertEndChild(base_animations);

      // Bones
      XmlElement bones_element("bones");
      const BoneVector &bones =  animation->getBones();
      for (n=0 ; n<bones.size() ; ++n)
      {
         XmlElement bone_element("bone");
         bone_element.SetAttribute("name", bones[n].Name);
         
   
         XmlElement bone_rotation("rotation");
         XmlFile::setAttribute(&bone_rotation, "start", bones[n].Rotation.first);
         XmlFile::setAttribute(&bone_rotation, "end", bones[n].Rotation.second);
         if (bones[n].RotationMode == MODIFY_ABSOLUTE)
            bone_rotation.SetAttribute("mode", "absolute");
         if (bones[n].RotationMode == MODIFY_RELATIVE)
            bone_rotation.SetAttribute("mode", "relative");
         if (bones[n].RotationMode == MODIFY_CUMULATIVE)
            bone_rotation.SetAttribute("mode", "cumulative");
         bone_element.InsertEndChild(bone_rotation);

         XmlElement bone_translation("translation");
         XmlFile::setAttribute(&bone_translation, "start", bones[n].Translation.first);
         XmlFile::setAttribute(&bone_translation, "end", bones[n].Translation.second);
         if (bones[n].TranslationMode == MODIFY_ABSOLUTE)
            bone_translation.SetAttribute("mode", "absolute");
         else
            bone_translation.SetAttribute("mode", "relative");
         bone_element.InsertEndChild(bone_translation);

         XmlElement bone_scale("scale");
         XmlFile::setAttribute(&bone_scale, "start", bones[n].Scale.first);
         XmlFile::setAttribute(&bone_scale, "end", bones[n].Scale.second);
         bone_element.InsertEndChild(bone_scale);

         bones_element.InsertEndChild(bone_element);
      }
      animation_element.InsertEndChild(bones_element);

      rootElement->InsertEndChild(animation_element);
   }

   bool DynamicAnimationSerializer::importAnimation(const XmlElement *rootElement, DynamicAnimation *animationDest)
   {
      assert(rootElement);
      assert(animationDest);

      const Ogre::String *name = rootElement->Attribute(Ogre::String("name"));
      if (!name)
         return false;

      const XmlElement *baseanimation_element = rootElement->FirstChildElement("base_animations");
      if (!baseanimation_element)
      {
         Ogre::LogManager::getSingleton().logMessage("No base animation(s) defined in: " + *name + ".");
         return false;
      }

      const XmlElement *animation_element = baseanimation_element->FirstChildElement();
      while (animation_element)
      {
         const Ogre::String *baseAnimationName = animation_element->Attribute(Ogre::String("name"));
         if (baseAnimationName != 0)
            animationDest->getBaseAnimations().push_back(*baseAnimationName);

         animation_element = animation_element->NextSiblingElement();
      }

      // Bones
      //
      const XmlElement *bonelist_element = rootElement->FirstChildElement("bones");
      if (!bonelist_element)
      {
         Ogre::LogManager::getSingleton().logMessage("No bones in: " + *name + ".");
         return false;
      }

      const XmlElement *bone_element = bonelist_element->FirstChildElement();
      const XmlElement *next_bone_element = bone_element;
      while(next_bone_element)
      {
         DynamicAnimationBone bone;
         
         bone_element = next_bone_element;
         next_bone_element = next_bone_element->NextSiblingElement();
         const Ogre::String *name = bone_element->Attribute(Ogre::String("name"));
         if (!name || name->empty())
         {
            Ogre::LogManager::getSingleton().logMessage("No name for bone in: " + *name + ".");
            continue;
         }
         bone.Name = *name;

         const XmlElement *bone_rot = bone_element->FirstChildElement("rotation");
         if (!bone_rot)
         {
            Ogre::LogManager::getSingleton().logMessage("No rotation for bone: " + *name + ".");
            continue;
         }
         XmlFile::queryVector3Attribute(bone_rot, "start", &bone.Rotation.first);
         XmlFile::queryVector3Attribute(bone_rot, "end", &bone.Rotation.second);
         const Ogre::String* rotationMode = bone_rot->Attribute(Ogre::String("mode"));
         if (rotationMode)
         {
            if (!rotationMode->compare("absolute"))
               bone.RotationMode = MODIFY_ABSOLUTE;
            else if (!rotationMode->compare("relative"))
               bone.RotationMode = MODIFY_RELATIVE;
            else if (!rotationMode->compare("cumulative"))
               bone.RotationMode = MODIFY_CUMULATIVE;
         }

         const XmlElement *bone_trans = bone_element->FirstChildElement("translation");
         if (!bone_trans)
         {
            Ogre::LogManager::getSingleton().logMessage("No translation for bone in: " + *name + ".");
            continue;
         }
         XmlFile::queryVector3Attribute(bone_trans, "start", &bone.Translation.first);
         XmlFile::queryVector3Attribute(bone_trans, "end", &bone.Translation.second);
         const Ogre::String* translationMode = bone_trans->Attribute(Ogre::String("mode"));
         if (translationMode)
         {
            if (!translationMode->compare("absolute"))
               bone.TranslationMode = MODIFY_ABSOLUTE;
            else if (!translationMode->compare("relative"))
               bone.TranslationMode = MODIFY_RELATIVE;
         }

         const XmlElement *bone_scale = bone_element->FirstChildElement("scale");
         if (!bone_scale)
         {
            Ogre::LogManager::getSingleton().logMessage("No scale for bone in: " + *name + ".");
            continue;
         }
         XmlFile::queryVector3Attribute(bone_scale, "start", &bone.Scale.first);
         XmlFile::queryVector3Attribute(bone_scale, "end", &bone.Scale.second);


         animationDest->addBone(bone);
      }
      return true;
   }
}


