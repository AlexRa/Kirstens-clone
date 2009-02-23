// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "AvatarSerializer.h"
#include "Avatar.h"
#include "OgreAvatar.h"
#include "XmlManager.h"
#include "tinyxml.h"
#include "DynamicAnimationManager.h"
#include "DynamicAnimationSerializer.h"
#include "AvatarAttachmentSerializer.h"
#include "XmlFile.h"

namespace Rex
{
   const char* AvatarSerializer::XMLTEMPLATE =
      "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
      "<!-- Template XML file for avatar parameters -->\n"
      "<avatar>\n"
   	"<!-- version string for this file -->\n"
	   "<version>0.2</version>"
      "</avatar>";

   AvatarSerializer::AvatarSerializer()
   {
   }

   AvatarSerializer::~AvatarSerializer()
   {
   }

   void AvatarSerializer::exportAvatar(const Avatar *avatar, XmlDocument *doc, const std::vector<Rex::DynamicAnimationPtr> &animations, bool exportTextures, bool exportDeformsFull)
   {
      assert(avatar);
      assert(doc);
      
      XmlElement *root = doc->RootElement();
      if (root)
      {
         // Mesh
         Ogre::String meshName = avatar->getMesh();
         XmlElement base("base");
         base.SetAttribute("name", "default_female" );
		   base.SetAttribute("mesh", meshName);
         root->InsertEndChild(base);
   		
         // Skeleton
         Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().getByName(meshName);
         if (mesh.isNull() == false && mesh->hasSkeleton())
         {
            XmlElement skeleton("skeleton");
            Ogre::String skeletonName = "default";
            skeletonName = mesh->getSkeletonName();
            skeleton.SetAttribute("name", skeletonName);
            root->InsertEndChild(skeleton);
         }
         size_t n;
         for (n=0 ; n<avatar->getNumExtraAnimatedSkeletons() ; ++n)
         {
            XmlElement skeleton("extra_skeleton");
            Ogre::String skeletonName = avatar->getExtraAnimatedSkeleton(n);

            skeleton.SetAttribute("name", skeletonName);
            root->InsertEndChild(skeleton);
         }

         // Materials
         for (n=0 ; n<avatar->getNumMaterials() ; ++n)
         {
            XmlElement material("material");
            material.SetAttribute("name", avatar->getMaterial(n));
            root->InsertEndChild(material);
         }

         // Textures
         XmlElement face_texture("texture_face");
         face_texture.SetAttribute("name", const_cast<Avatar*>(avatar)->getTexture(Avatar::TEXTURE_FACE));
         root->InsertEndChild(face_texture);

         XmlElement body_texture("texture_body");
         body_texture.SetAttribute("name", const_cast<Avatar*>(avatar)->getTexture(Avatar::TEXTURE_BODY));
         root->InsertEndChild(body_texture);
         
         // Appearance
         XmlElement appearance("appearance");
         appearance.SetDoubleAttribute("height", avatar->getHeight());
         appearance.SetAttribute("weight", avatar->getWeight());
         root->InsertEndChild(appearance);

         // Transformation
         XmlElement transformation("transformation");
         XmlFile::setAttribute(&transformation, "position", Ogre::Vector3::ZERO);
         XmlFile::setAttribute(&transformation, "rotation", Ogre::Quaternion::IDENTITY);
         XmlFile::setAttribute(&transformation, "scale", Ogre::Vector3::UNIT_SCALE);
         root->InsertEndChild(transformation);

         // Attachments
         for (n=0 ; n<avatar->getNumAttachments() ; ++n)
         {
            const AvatarAttachment &attachment = *avatar->getAttachment(n);
            if (attachment.isAttached() == false)
               continue;

            XmlElement attachmentElement("attachment");
            AvatarAttachmentSerializer attachmentSerializer;
            bool onlyAttachedBones = true;
            attachmentSerializer.exportAttachment(&attachment, &attachmentElement, onlyAttachedBones);

            root->InsertEndChild(attachmentElement);
         }
      }  



//      // Check version (not yet implemented)
//      const XmlNode *node_const = root->FirstChild("version");
//      assert(node_const);

      //// Set transformations
      //XmlNode *node = root->FirstChild("appearance");
      //assert(node);
      //XmlElement *element = node->ToElement();
      //assert(element);
      //
      //element->SetDoubleAttribute("height", avatar->getHeight());
      //element->SetDoubleAttribute("weight", avatar->getWeight());

       Rex::DynamicAnimationSerializer animSerializer;

      const AnimationModifierVector& animationMods = avatar->getAnimationModifierVector();
      size_t n;
      for (n=0 ; n<animationMods.size() ; ++n)
      {
         DynamicAnimationPtr animation = DynamicAnimationManager::getSingleton().getByName(animationMods[n]->InternalName);
         if (animation.isNull() == false)
         {
            animation->exportTo(root);
            
            if (exportDeformsFull)
            {
               animSerializer.exportAnimation(animation.getPointer(), root);
            }
         }
      }

      for (n=0 ; n<animations.size() ; ++n)
      {
         if (animations[n].isNull() == false)
         {
            animations[n]->exportTo(root);

            if (exportDeformsFull)
            {
               animSerializer.exportAnimation(animations[n].getPointer(), root);
            }
         }
      }

      // Morph modifiers
      const MorphModifierMap& morphMods = avatar->getMorphModifierMap();
      {
         MorphModifierMap::const_iterator i = morphMods.begin();
         while (i != morphMods.end())
         {
            exportMorphModifier(i->first, i->second, doc);
            ++i;
         }
      }

      // Texture modifiers
      const TextureModifierMap& textureMods = avatar->getTextureModifierMap();
      {
         TextureModifierMap::const_iterator i = textureMods.begin();
         while (i != textureMods.end())
         {
            exportTextureModifier(i->first, i->second, doc);
            ++i;
         }
      }

      // Animation definitions
      const AnimationDefMap& animDefs = avatar->getAnimationDefMap();
      {
         AnimationDefMap::const_iterator i = animDefs.begin();
         while (i != animDefs.end())
         {
            exportAnimationDef(i->first, i->second, doc);
            ++i;
         }
      }

      // Master appearance modifiers
      const MasterModifierMap& masterModifiers = avatar->getMasterModifierMap();
      {
         MasterModifierMap::const_iterator i = masterModifiers.begin();
         while (i != masterModifiers.end())
         {
            exportMasterModifier(i->first, i->second, doc, avatar);
            ++i;
         }
      }

      // Bone constraints
      const BoneConstraintMap& boneConstraints = avatar->getBoneConstraintMap();
      {
         BoneConstraintMap::const_iterator i = boneConstraints.begin();
         while (i != boneConstraints.end())
         {
            BoneConstraintVector::const_iterator j = i->second.begin();
            while (j != i->second.end())
            {
               exportBoneConstraint(i->first, *j, doc);
               ++j;
            }
            ++i;
         }
      }

      // Generic properties
      const PropertyMap& properties = avatar->getPropertyMap();
      {
         PropertyMap::const_iterator i = properties.begin(); 
         while (i != properties.end())
	      {
            XmlElement prop("property");
            prop.SetAttribute("name", i->first);
            prop.SetAttribute("value", i->second);
            root->InsertEndChild(prop);

            ++i;
         }
	   }
   }

   void AvatarSerializer::exportMorphModifier(const std::string &name, const MorphModifier &morph, XmlDocument *doc)
   {
       XmlElement *root = doc->RootElement();

       XmlElement morphElement("morph_modifier");
       morphElement.SetAttribute("name", name);
       morphElement.SetAttribute("internal_name", morph.InternalName);
       morphElement.SetDoubleAttribute("influence", morph.Influence);
       // For clarity, do not export the bool if false (default value)
       if (morph.Manual) XmlFile::setAttribute(&morphElement, "manual", morph.Manual);

       root->InsertEndChild(morphElement);
   }

   void AvatarSerializer::exportTextureModifier(const std::string &name, const TextureModifier &textureMod, XmlDocument *doc)
   {
       XmlElement *root = doc->RootElement();

       XmlElement textureElement("texture_modifier");
       textureElement.SetAttribute("name", name);
       textureElement.SetAttribute("texture_name", textureMod.TextureName);
       textureElement.SetDoubleAttribute("threshold", textureMod.Threshold);
       textureElement.SetDoubleAttribute("influence", textureMod.Influence);
       textureElement.SetAttribute("submesh_index", textureMod.SubmeshIndex);

       root->InsertEndChild(textureElement);
   }

   void AvatarSerializer::exportAnimationDef(const std::string &name, const AnimationDef &animDef, XmlDocument *doc)
   {
      XmlElement *root = doc->RootElement();

      XmlElement animElement("animation");
      animElement.SetAttribute("name", name);
      animElement.SetAttribute("id", animDef.Id);
      animElement.SetAttribute("internal_name", animDef.InternalName);
      // For clarity, do not export the bools if false (default value)
      if (animDef.AlwaysRestart) XmlFile::setAttribute(&animElement, "alwaysrestart", animDef.AlwaysRestart);
      if (animDef.Exclusive) XmlFile::setAttribute(&animElement, "exclusive", animDef.Exclusive);
      if (animDef.Looped) XmlFile::setAttribute(&animElement, "looped", animDef.Looped);
      if (animDef.UseVelocity) XmlFile::setAttribute(&animElement, "usevelocity", animDef.UseVelocity);
      animElement.SetDoubleAttribute("fadein", animDef.FadeIn);
      animElement.SetDoubleAttribute("fadeout", animDef.FadeOut);
      animElement.SetDoubleAttribute("speedfactor", animDef.SpeedFactor);

      root->InsertEndChild(animElement);
   }

   void AvatarSerializer::exportMasterModifier(const std::string &name, const MasterModifier &masterModifier, XmlDocument *doc, const Avatar *avatarSrc)
   {
      XmlElement *root = doc->RootElement();

      XmlElement masterElement("master_modifier");
      masterElement.SetAttribute("name", name);
      masterElement.SetDoubleAttribute("position", masterModifier.Position);

      if (!masterModifier.Category.empty())
      {
          masterElement.SetAttribute("category", masterModifier.Category);
      }

      AppearanceModifierVector::const_iterator i = masterModifier.Modifiers.begin();
      while (i != masterModifier.Modifiers.end())
      {
          XmlElement modifierElement("target_modifier");
          Ogre::String name = i->Name;
          switch (i->Type)
          {
          case MODIFIER_BONE:
              modifierElement.SetAttribute("type", "dynamic_animation");
              // Must find internal name of the associated animation, because that will be exported
              {
                  const AnimationModifierMap &animMap = avatarSrc->getAnimationModifierMap();
                  AnimationModifierMap::const_iterator k = animMap.find(name);
                  if (k != animMap.end()) name = k->second.InternalName;
              }
              break;
          case MODIFIER_MORPH:
              modifierElement.SetAttribute("type", "morph");
              break;

          case MODIFIER_TEXTURE:
              modifierElement.SetAttribute("type", "texture");
              break;
          }
          modifierElement.SetAttribute("name", name);

          switch(i->Mode)
          {
          case MODE_CUMULATIVE:
              modifierElement.SetAttribute("mode", "cumulative");
              break;
          case MODE_AVERAGE:
              modifierElement.SetAttribute("mode", "average");
              break;
          }
          
          PositionMappingVector::const_iterator j = i->Positions.begin();
          while (j != i->Positions.end())
          {
              XmlElement mappingElement("position_mapping");
              mappingElement.SetDoubleAttribute("master", j->MasterPosition);
              mappingElement.SetDoubleAttribute("target", j->Position);
              modifierElement.InsertEndChild(mappingElement);
              ++j;
          }
          masterElement.InsertEndChild(modifierElement);
          ++i;
      }

      root->InsertEndChild(masterElement);
   }

   void AvatarSerializer::exportBoneConstraint(const std::string &boneName, const BoneConstraint &constraint, XmlDocument *doc)
   {
      XmlElement *root = doc->RootElement();

      XmlElement constraintElement("bone_constraint");
      constraintElement.SetAttribute("bone", boneName);
      constraintElement.SetAttribute("target", constraint.Target);
      switch (constraint.Type)
      {
      case CONSTRAINT_POSITION:
          constraintElement.SetAttribute("type", "position");
          break;
      case CONSTRAINT_LOOKAT:
          constraintElement.SetAttribute("type", "lookat");
          break;
      }

      root->InsertEndChild(constraintElement);
   }

   bool AvatarSerializer::importAvatar(const XmlDocument *doc, Avatar *avatarDest, bool deleteAttachments)
   {
      assert(doc);
      assert(avatarDest);

      // Out with the old stuff
      avatarDest->clear(deleteAttachments);

      // In with the new stuff

      const XmlElement *root = doc->RootElement();

      if (!root)
         return false;

      // Check version (
      const XmlElement *node_const = root->FirstChildElement("version");
      //assert(node_const);
      if (!node_const)
         return false;
      else
      {
         const char *value = node_const->GetText();
         
         if (value)
         {
            avatarDest->setVersion(Ogre::StringConverter::parseReal(Ogre::String(value)));
         }
      }

      const XmlElement *mesh_element = root->FirstChildElement("base");
      if (!mesh_element)
         return false;

      const Ogre::String *mesh = mesh_element->Attribute(Ogre::String("mesh"));
      if (!mesh)
         return false;
      avatarDest->setMesh(*mesh);


      // Skeletons
      //
      const XmlElement *skeleton_element = root->FirstChildElement("skeleton");
      if (skeleton_element)
      {
         const Ogre::String *skeleton = skeleton_element->Attribute(Ogre::String("name"));
         if (skeleton)
         {
            avatarDest->setSkeleton(*skeleton);
         }
      }

      const XmlElement *extraSkeleton_element = root->FirstChildElement("extra_skeleton");
      while (extraSkeleton_element)
      {
         const XmlElement *current = extraSkeleton_element;
         extraSkeleton_element = extraSkeleton_element->NextSiblingElement("extra_skeleton");
   
         const Ogre::String *name = current->Attribute(Ogre::String("name"));
         if (name)
         {
            avatarDest->addExtraAnimatedSkeleton(*name);
         }
      }
      
      const XmlElement *appearance_element = root->FirstChildElement("appearance");
      if (appearance_element)
      {
         float height;
         if (appearance_element->QueryFloatAttribute("height", &height) != TIXML_SUCCESS)
            return false;

         float weight;
         if (appearance_element->QueryFloatAttribute("weight", &weight) != TIXML_SUCCESS)
            return false;

         avatarDest->setHeight(height);
         avatarDest->setWeight(weight);
      }

      const XmlElement *material_element = root->FirstChildElement("material");
      while (material_element)
      {
         const XmlElement *current = material_element;
         material_element = material_element->NextSiblingElement("material");
   
         const Ogre::String *name = current->Attribute(Ogre::String("name"));
         if (name)
         {
            avatarDest->addMaterial(*name);
         }
      }

      const XmlElement *texture_face_element = root->FirstChildElement("texture_face");
      if (texture_face_element)
      {
         const Ogre::String *name = texture_face_element->Attribute(Ogre::String("name"));
         if (name && (*name).empty() == false)
         {
            avatarDest->setTexture(Avatar::TEXTURE_FACE, *name);
         }
      }
      const XmlElement *texture_body_element = root->FirstChildElement("texture_body");
      if (texture_body_element)
      {
         const Ogre::String *name = texture_body_element->Attribute(Ogre::String("name"));
         if (name && (*name).empty() == false)
         {
            avatarDest->setTexture(Avatar::TEXTURE_BODY, *name);
         }
      }


      const XmlElement *attachment_element = root->FirstChildElement("attachment");
      while (attachment_element)
      {
         AvatarAttachment *attachment = new AvatarAttachment();
         AvatarAttachmentSerializer serializer;
         try
         {
            serializer.importAttachment(attachment_element, attachment);
         
            AvatarAttachment *newAttachment = avatarDest->addAttachment(attachment);
            if (newAttachment != attachment)
            {
               delete attachment;
            }
         } catch (RexException e)
         {
            Ogre::LogManager::getSingleton().logMessage("Failed to import attachment.");
         }

         attachment_element = attachment_element->NextSiblingElement("attachment");
      }

      // Morph modifiers
      const XmlElement *morph_element = root->FirstChildElement("morph_modifier");
      while (morph_element)
      {
         importMorphModifier(morph_element, avatarDest);
         morph_element = morph_element->NextSiblingElement("morph_modifier");
      }

      // (Possibly) mesh specific data: animations, bone deforms, master modifiers
      importMeshSpecificData(doc, avatarDest);

      return true;
   }

   bool AvatarSerializer::importMeshSpecificData(const XmlDocument *doc, Avatar *avatarDest)
   {
      const XmlElement *root = doc->RootElement();

      if (!root)
         return false;

      // Import dynamic animations
      importDynamicAnimations(doc, avatarDest);

      // Import texture modifiers
      const XmlElement *texture_element = root->FirstChildElement("texture_modifier");
      while (texture_element)
      {
         importTextureModifier(texture_element, avatarDest);
         texture_element = texture_element->NextSiblingElement("texture_modifier");
      }

      // Clear old animation definitions
      avatarDest->clearAnimationDefs();

      // Import animation definitions
      const XmlElement *animation_element = root->FirstChildElement("animation");
      while (animation_element)
      {
         importAnimationDef(animation_element, avatarDest);
         animation_element = animation_element->NextSiblingElement("animation");
      }

      // Clear old master modifiers
      avatarDest->clearMasterModifiers();

      // Import master modifiers
      const XmlElement *master_element = root->FirstChildElement("master_modifier");
      while (master_element)
      {
         importMasterModifier(master_element, avatarDest);
         master_element = master_element->NextSiblingElement("master_modifier");
      }

      // Clear old bone constraints
      avatarDest->clearBoneConstraints();

      // Import bone constraints ("stretchy bones")
      const XmlElement *boneconstraint_element = root->FirstChildElement("bone_constraint");
      while (boneconstraint_element)
      {
         importBoneConstraint(boneconstraint_element, avatarDest);
         boneconstraint_element = boneconstraint_element->NextSiblingElement("bone_constraint");
      }

      // Import generic properties
      avatarDest->unsetProperty("basemesh");
      // Hack: clear the basebone element from old avatar data, in case it isn't defined in the new data
      avatarDest->unsetProperty("basebone");
      avatarDest->unsetProperty("baseoffset");
      const XmlElement *prop_element = root->FirstChildElement("property");
      while (prop_element)
      {
         const Ogre::String *name = prop_element->Attribute(Ogre::String("name"));
         const Ogre::String *value = prop_element->Attribute(Ogre::String("value"));
         if (name && value)
         {
            avatarDest->setProperty(*name, *value);
         }
         
         prop_element = prop_element->NextSiblingElement("property");
      }

      return true;
   }

   bool AvatarSerializer::importDynamicAnimations(const XmlDocument *doc, Avatar *avatarDest)
   {
      const XmlElement *root = doc->RootElement();

      if (!root)
         return false;

      const XmlElement *dynanim_element = root->FirstChildElement("dynamic_animation");
      while (dynanim_element)
      {
         DynamicAnimationModifier animationModifier;

         const XmlElement *current = dynanim_element;
         dynanim_element = dynanim_element->NextSiblingElement("dynamic_animation");
         
         std::string name = "dyn_dynamic_animation";
         const Ogre::String *anim_name = current->Attribute(Ogre::String("name"));
         if (!anim_name)
            continue;

         name = *anim_name;
      //   animationModifier.Name = name;

         // Generate unique name for each dyn anim
         int value = 1;
         name.append(Ogre::StringConverter::toString(value));
         
         while (DynamicAnimationManager::getSingleton().resourceExists(name) == true)
         {
            value++;
            name = *anim_name;
            name.append(Ogre::StringConverter::toString(value));
         }
         
         animationModifier.InternalName = name;
         DynamicAnimationPtr animation = DynamicAnimationManager::getSingleton().create(name, XmlManager::XMLDATA_RESOURCE_GROUP_NAME, true);

         DynamicAnimationSerializer serializer;
         serializer.importAnimation(current, animation.getPointer());

         avatarDest->setAnimationModifier(*anim_name, animationModifier);
      }

      const XmlElement *dynanim_param_element = root->FirstChildElement("dynamic_animation_parameter");
      while (dynanim_param_element)
      {
         const XmlElement *current = dynanim_param_element;
         dynanim_param_element = dynanim_param_element->NextSiblingElement("dynamic_animation_parameter");

         const Ogre::String *name = current->Attribute(Ogre::String("name"));
         if (!name)
            continue;

         float value = 0.5f;
         current->QueryFloatAttribute("position", &value);

         avatarDest->setAnimationModifierPosition(*name, value);
      }

      return true;
   }

   bool AvatarSerializer::importMorphModifier(const XmlElement *morph_element, Avatar *avatarDest)
   {
      const Ogre::String *name = morph_element->Attribute(Ogre::String("name"));
      const Ogre::String *internalName = morph_element->Attribute(Ogre::String("internal_name"));

      if ((!name) || (!internalName)) 
          return false;

      MorphModifier newMorph;
      newMorph.Influence = 0.0f;
      newMorph.InternalName = *internalName;
      newMorph.Manual = false;
      morph_element->QueryFloatAttribute("influence", &newMorph.Influence);
      XmlFile::queryBoolAttribute(morph_element, "manual", &newMorph.Manual);

      avatarDest->setMorphModifier(*name, newMorph);

      return true;
   }

   bool AvatarSerializer::importTextureModifier(const XmlElement *texture_element, Avatar *avatarDest)
   {
      const Ogre::String *name = texture_element->Attribute(Ogre::String("name"));
      const Ogre::String *textureName = texture_element->Attribute(Ogre::String("texture_name"));

      if ((!name) || (!textureName))
          return false;

      TextureModifier newTexture;
      newTexture.Influence = 0.0f;
      newTexture.Threshold = 0.5f;
      newTexture.TextureName = *textureName;
      texture_element->QueryFloatAttribute("influence", &newTexture.Influence);
      texture_element->QueryFloatAttribute("threshold", &newTexture.Threshold);
      texture_element->QueryIntAttribute("submesh_index", &newTexture.SubmeshIndex);

      avatarDest->setTextureModifier(*name, newTexture);

      return true;
   }

   bool AvatarSerializer::importAnimationDef(const XmlElement *anim_element, Avatar *avatarDest)
   {
      const Ogre::String *name = anim_element->Attribute(Ogre::String("name"));
      const Ogre::String *id = anim_element->Attribute(Ogre::String("id"));
      if (!id) id = anim_element->Attribute(Ogre::String("uuid")); // Legacy name
      const Ogre::String *internalName = anim_element->Attribute(Ogre::String("internal_name"));
      if (!internalName) internalName = anim_element->Attribute(Ogre::String("ogrename")); // Legacy name

      // Must have all these strings defined to be of use
      if ((!name) || (!id) || (!internalName))
         return false;

      AnimationDef newAnimDef;
      newAnimDef.Id = *id;
      newAnimDef.InternalName = *internalName;
      newAnimDef.Looped = false;
      newAnimDef.Exclusive = false;
      newAnimDef.UseVelocity = false;
      newAnimDef.AlwaysRestart = false;
      newAnimDef.FadeIn = 0.0f;
      newAnimDef.FadeOut = 0.0f;
      newAnimDef.SpeedFactor = 1.0f;

      XmlFile::queryBoolAttribute(anim_element, "looped", &newAnimDef.Looped);
      XmlFile::queryBoolAttribute(anim_element, "exclusive", &newAnimDef.Exclusive);
      XmlFile::queryBoolAttribute(anim_element, "usevelocity", &newAnimDef.UseVelocity);
      XmlFile::queryBoolAttribute(anim_element, "alwaysrestart", &newAnimDef.AlwaysRestart);
      anim_element->QueryFloatAttribute("fadein", &newAnimDef.FadeIn);
      anim_element->QueryFloatAttribute("fadeout", &newAnimDef.FadeOut);
      anim_element->QueryFloatAttribute("speedfactor", &newAnimDef.SpeedFactor);
      if (newAnimDef.FadeIn < 0.0f) newAnimDef.FadeIn = 0.0f;
      if (newAnimDef.FadeOut < 0.0f) newAnimDef.FadeOut = 0.0f;
      if (newAnimDef.SpeedFactor < 0.0f) newAnimDef.SpeedFactor = 0.0f;

      avatarDest->setAnimationDef(*name, newAnimDef);

      return true;
   }

   bool AvatarSerializer::importMasterModifier(const XmlElement *master_element, Avatar *avatarDest)
   {
      const Ogre::String *name = master_element->Attribute(Ogre::String("name"));
      if (!name) return false;

      MasterModifier newMasterModifier;
      newMasterModifier.Position = 0.0f;
      master_element->QueryFloatAttribute("position", &newMasterModifier.Position);

      const Ogre::String *category = master_element->Attribute(Ogre::String("category"));
      if (category)
      {
         newMasterModifier.Category = *category;
      }

      const XmlElement *modifier_element = master_element->FirstChildElement("target_modifier");
      while (modifier_element)
      {
         const XmlElement *master_element2 = modifier_element;
         modifier_element = master_element2->NextSiblingElement("target_modifier");

         AppearanceModifier newModifier;
         const Ogre::String *name = master_element2->Attribute(Ogre::String("name"));
         const Ogre::String *type = master_element2->Attribute(Ogre::String("type"));
         if ((!name) || (!type)) continue;

         newModifier.Name = *name;
         if (!type->compare(Ogre::String("dynamic_animation")))
            newModifier.Type = MODIFIER_BONE;
         else if (!type->compare(Ogre::String("bone")))
            newModifier.Type = MODIFIER_BONE;
         else if (!type->compare(Ogre::String("morph")))
            newModifier.Type = MODIFIER_MORPH;
         else if (!type->compare(Ogre::String("texture")))
            newModifier.Type = MODIFIER_TEXTURE;
         else continue;

         newModifier.Mode = MODE_CUMULATIVE;
         const Ogre::String *mode = master_element2->Attribute(Ogre::String("mode")); 
         if (mode)
         {
             if (!mode->compare(Ogre::String("average")))
                 newModifier.Mode = MODE_AVERAGE;
             if (!mode->compare(Ogre::String("cumulative")))
                 newModifier.Mode = MODE_CUMULATIVE;
         }

         const XmlElement *position_element = master_element2->FirstChildElement("position_mapping");
         while (position_element)
         {
            const XmlElement *master_element3 = position_element;
            position_element = master_element3->NextSiblingElement("position_mapping");

            PositionMapping newMapping;

            newMapping.MasterPosition = 0.0f;
            newMapping.Position = 0.0f;
            master_element3->QueryFloatAttribute("master", &newMapping.MasterPosition);
            master_element3->QueryFloatAttribute("target", &newMapping.Position);
            if (newMapping.MasterPosition < 0.0f) newMapping.MasterPosition = 0.0f;
            if (newMapping.MasterPosition > 1.0f) newMapping.MasterPosition = 1.0f;
            if (newMapping.Position < 0.0f) newMapping.Position = 0.0f;
            if (newMapping.Position > 1.0f) newMapping.Position = 1.0f;
            newModifier.Positions.push_back(newMapping);
         }
         
         // Sort the mappings to ascending master position for correct interpolation
         std::sort(newModifier.Positions.begin(), newModifier.Positions.end());

         newMasterModifier.Modifiers.push_back(newModifier);
      }

      avatarDest->setMasterModifier(*name, newMasterModifier);

      return true;
   }

   bool AvatarSerializer::importBoneConstraint(const XmlElement *boneconstraint_element, Avatar *avatarDest)
   {
      BoneConstraint newConstraint;

      const Ogre::String* bone = boneconstraint_element->Attribute(Ogre::String("bone"));
      const Ogre::String* target = boneconstraint_element->Attribute(Ogre::String("target"));
      const Ogre::String* type = boneconstraint_element->Attribute(Ogre::String("type"));

      if ((bone) && (target) && (type))
      {
         newConstraint.Target = *target;
         newConstraint.Type = CONSTRAINT_POSITION;
         if (!type->compare("lookat"))
            newConstraint.Type = CONSTRAINT_LOOKAT;

         avatarDest->addBoneConstraint(*bone, newConstraint);
         return true;
      }
      else return false;
   }
}
