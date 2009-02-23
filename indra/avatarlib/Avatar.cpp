// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "Avatar.h"
#include <OGRE/Ogre.h>

namespace Rex
{
    const float Avatar::AVATAR_BASE_VERSION = 0.1;

    const float Avatar::BaseHeight = 1.8f;

   Avatar::Avatar() : Height(1.8f), Weight(1.0), Version(AVATAR_BASE_VERSION)
   {
   	  clear();
   }

   Avatar::Avatar(const std::string &uid) : Uid(uid)
   {
      clear();
   }

   Avatar::~Avatar()
   {
      clear();
   }

   void Avatar::clear(bool deleteAttachments)
   {
      MeshName = "";
      SkeletonName = "";
      Height = 1.8;
      Weight = 1.0;

      if (deleteAttachments)
      {
          clearAttachments();
      }
      MaterialList.clear();
      TextureList.clear();
      AnimationModifiers.clear();
      AnimationModifiersVector.clear();
      MorphModifiers.clear();
      TextureModifiers.clear();
      AnimationDefs.clear();
      MasterModifiers.clear();
      BoneConstraints.clear();
      Properties.clear();

      // Should not clear uid, since after clearing we still may need to know our uid
   }

   void Avatar::clearAttachments()
   {
      size_t n;
      for (n=0 ; n<AttachmentList.size() ; ++n)
      {
         if (AttachmentList[n] != 0)
         {
            delete (AttachmentList[n]);
            AttachmentList[n] = 0;
         }
      }  
      AttachmentList.clear();
   }

   float Avatar::getHeight() const
   {
      return Height;
   }

   void Avatar::setHeight(float height)
   {
      Height = height;
   }

   void Avatar::setWeight(float weight)
   {
      Weight = weight;
   }

   float Avatar::getWeight() const
   {
      return Weight;
   }

   std::string Avatar::getMesh() const
   {
      return MeshName;
   }

   void Avatar::setVersion(float version)
   {
       if (version < AVATAR_BASE_VERSION) version = AVATAR_BASE_VERSION;
       Version = version;
   }

   float Avatar::getVersion() const
   {
      return Version;
   }

   void Avatar::setMesh(const std::string &mesh)
   {
      MeshName = mesh;
   }

   void Avatar::setTexture(TEXTURE position, const std::string &name)
   {
      TextureList[position] = name;
   }
   void Avatar::setSkeleton(const std::string &skeleton)
   {
      SkeletonName = skeleton;
   }
   std::string Avatar::getTexture(TEXTURE position)
   {
      return (TextureList[position]);
   }

   const std::string &Avatar::getSkeleton() const
   {
      return SkeletonName;
   }

   size_t Avatar::addMaterial(const std::string &name)
   {
      MaterialList.push_back(name);

      return (MaterialList.size() - 1);
   }

   void Avatar::setMaterial(size_t index, const std::string &name)
   {
      if (index >= MaterialList.size())
      {
         MaterialList.resize(index + 1);
      }

      MaterialList[index] = name;
   }

   const std::string &Avatar::getMaterial(size_t index) const
   {
      assert(index < MaterialList.size());
      return MaterialList[index];
   }

   size_t Avatar::getNumMaterials() const
   {
      return MaterialList.size();
   }

   size_t Avatar::getNumAttachments() const
   {
      return AttachmentList.size();
   }

   const AvatarAttachment *Avatar::getAttachment(size_t index) const
   {
      return AttachmentList[index];
   }

   void Avatar::setAnimationModifier(const std::string &name, const DynamicAnimationModifier &animation)
   {
      bool previousExists = false;
      if (AnimationModifiers.find(name) != AnimationModifiers.end())
      {
         previousExists = true;
      }

      AnimationModifiers[name] = animation;

      // Only add to vector, if the modifier isn't already there
      if (!previousExists)
      {
         AnimationModifiersVector.push_back(&AnimationModifiers[name]);
      }
   }

   const AnimationModifierMap &Avatar::getAnimationModifierMap() const
   {
      return AnimationModifiers;
   }

   const AnimationModifierVector &Avatar::getAnimationModifierVector() const
   {
      return AnimationModifiersVector;
   }

   const DynamicAnimationModifier &Avatar::getAnimationModifier(const std::string &name)
   {
      return AnimationModifiers[name];
   }

   void Avatar::setAnimationModifierPosition(const std::string &name, float position)
   {
      AnimationModifiers[name].Position = position;
   }

  const MorphModifierMap &Avatar::getMorphModifierMap() const
   {
      return MorphModifiers;
   }

   void Avatar::setMorphModifier(const std::string &name, const MorphModifier& morph)
   {
      MorphModifiers[name] = morph;

      MorphModifier& newMorph = MorphModifiers[name];
      if (newMorph.Influence < 0.0f) newMorph.Influence = 0.0f;
      if (newMorph.Influence > 1.0f) newMorph.Influence = 1.0f;
   }

   const MorphModifier& Avatar::getMorphModifier(const std::string &name)
   {
      return MorphModifiers[name];
   }

   void Avatar::setMorphModifierInfluence(const std::string &name, float influence)
   {
      if (influence < 0.0f) influence = 0.0f;
      if (influence > 1.0f) influence = 1.0f;

      MorphModifiers[name].Influence = influence;
   }

   void Avatar::setMorphModifierManual(const std::string &name, bool manual)
   {
      MorphModifiers[name].Manual = manual;
   }

   const TextureModifierMap &Avatar::getTextureModifierMap() const
   {
      return TextureModifiers;
   }

   void Avatar::setTextureModifier(const std::string &name, const TextureModifier& textureMod)
   {
      TextureModifiers[name] = textureMod;

      TextureModifier& newMod = TextureModifiers[name];
      if (newMod.Influence < 0.0f) newMod.Influence = 0.0f;
      if (newMod.Influence > 1.0f) newMod.Influence = 1.0f;
   }

   const TextureModifier& Avatar::getTextureModifier(const std::string &name)
   {
      return TextureModifiers[name];
   }

   void Avatar::setTextureModifierInfluence(const std::string &name, float influence)
   {
      if (influence < 0.0f) influence = 0.0f;
      if (influence > 1.0f) influence = 1.0f;

      TextureModifiers[name].Influence = influence;
   }

   void Avatar::clearAnimationDefs()
   {
       AnimationDefs.clear();
   }

   const AnimationDef& Avatar::getAnimationDef(const std::string& name)
   {
       return AnimationDefs[name];
   }

   void Avatar::setAnimationDef(const std::string& name, const AnimationDef& animationDef)
   {
       AnimationDefs[name] = animationDef;
   }

   const AnimationDefMap& Avatar::getAnimationDefMap() const
   {
       return AnimationDefs;
   }

   AnimationDefMap& Avatar::getAnimationDefMap()
   {
       return AnimationDefs;
   }

   const MasterModifierMap& Avatar::getMasterModifierMap() const
   {
       return MasterModifiers;
   }

   const MasterModifier& Avatar::getMasterModifier(const std::string& name)
   {
       return MasterModifiers[name];
   }

   void Avatar::setMasterModifier(const std::string& name, const MasterModifier& modifier)
   {
      MasterModifiers[name] = modifier;
      MasterModifier& newModifier = MasterModifiers[name];
      if (newModifier.Position < 0.0f) newModifier.Position = 0.0f;
      if (newModifier.Position > 1.0f) newModifier.Position = 1.0f;

      // Because may affect modifiers in complex ways, recalculate all
      calculateMasterModifiers();
   }

   void Avatar::setMasterModifierPosition(const std::string& name, float position)
   {
      if (position < 0.0f) position = 0.0f;
      if (position > 1.0f) position = 1.0f;
      MasterModifiers[name].Position = position;

      // Because may affect modifiers in complex ways, recalculate all
      calculateMasterModifiers();
   }

   void Avatar::clearMasterModifiers()
   {
       MasterModifiers.clear();
   }

   void Avatar::calculateMasterModifiers()
   {
      ModifierAccumulationMap boneMap;
      ModifierAccumulationMap morphMap;
      ModifierAccumulationMap textureMap;

      // First calculate all
      MasterModifierMap::const_iterator i = MasterModifiers.begin();
      while (i != MasterModifiers.end())
      {
         calculateMasterModifier(i->second, boneMap, morphMap, textureMap);
         ++i;
      }

      // Then apply accumulated data
      ModifierAccumulationMap::const_iterator j = morphMap.begin();
      while (j != morphMap.end())
      {
          // Do not override manually set morph
          if (!getMorphModifier(j->first).Manual)
              setMorphModifierInfluence(j->first, j->second.Position);
          ++j;
      }
      j = boneMap.begin();
      while (j != boneMap.end())
      {
          setAnimationModifierPosition(j->first, j->second.Position);
          ++j;
      }
      j = textureMap.begin();
      while (j != textureMap.end())
      {
          setTextureModifierInfluence(j->first, j->second.Position);
          ++j;
      }
   }

   void Avatar::calculateMasterModifier(const MasterModifier& modifier, ModifierAccumulationMap& boneMap, ModifierAccumulationMap& morphMap, ModifierAccumulationMap& textureMap)
   {
      float masterPosition = modifier.Position;

      std::vector<AppearanceModifier>::const_iterator i = modifier.Modifiers.begin();
      while (i != modifier.Modifiers.end())
      {
         float position = interpolateAppearanceModifier(*i, masterPosition);
    
         switch (i->Type)
         {
         case MODIFIER_BONE:
            if (boneMap.find(i->Name) == boneMap.end())
            {
                ModifierAccumulationData empty;
                boneMap[i->Name] = empty;
            }
            switch (i->Mode)
            {
            case MODE_AVERAGE:
                boneMap[i->Name].addAverage(position);
                break;
            case MODE_CUMULATIVE:
                boneMap[i->Name].addCumulative(position);
                break;
            }
            break;

         case MODIFIER_MORPH:
            if (morphMap.find(i->Name) == morphMap.end())
            {
                ModifierAccumulationData empty;
                morphMap[i->Name] = empty;
            }
            switch (i->Mode)
            {
            case MODE_AVERAGE:
                morphMap[i->Name].addAverage(position);
                break;
            case MODE_CUMULATIVE:
                morphMap[i->Name].addCumulative(position);
                break;
            }
            break;

         case MODIFIER_TEXTURE:
            if (textureMap.find(i->Name) == textureMap.end())
            {
                ModifierAccumulationData empty;
                textureMap[i->Name] = empty;
            }
            switch (i->Mode)
            {
            case MODE_AVERAGE:
                textureMap[i->Name].addAverage(position);
                break;
            case MODE_CUMULATIVE:
                textureMap[i->Name].addCumulative(position);
                break;
            }
            break;
         }
         ++i;
      }
   }

   float Avatar::interpolateAppearanceModifier(const AppearanceModifier& modifier, float masterPosition)
   {
      // If no positions to interpolate, map master slider directly to modifier pos (correct or not)
      if (modifier.Positions.size() < 2)
      {
         return masterPosition;
      }

      // Find out the minimum/maximum range of supported master positions
      float minPosition = 1.0f;
      float maxPosition = 0.0f;

      size_t i;
      for (i = 0; i < modifier.Positions.size(); ++i)
      {
         if (modifier.Positions[i].MasterPosition < minPosition)
            minPosition = modifier.Positions[i].MasterPosition;
         if (modifier.Positions[i].MasterPosition > maxPosition)
            maxPosition = modifier.Positions[i].MasterPosition;
      }

      // Now cap the master position according to what is supported
      if (masterPosition < minPosition)
          masterPosition = minPosition;
      if (masterPosition > maxPosition)
          masterPosition = maxPosition;

      // Find beginning pos. of interpolation
      for (i = modifier.Positions.size()-1; i >= 0; --i)
      {
         if (modifier.Positions[i].MasterPosition <= masterPosition)
            break;
      }

      // If at the endpoint, simply return the value at end
      if (i == modifier.Positions.size()-1)
      {
          return modifier.Positions[i].Position;
      }

      float delta = modifier.Positions[i+1].Position - modifier.Positions[i].Position;
      float masterDelta = modifier.Positions[i+1].MasterPosition - modifier.Positions[i].MasterPosition;
      float weight = 0.0f;
      if (masterDelta > 0.0f)
      {
         weight = (masterPosition - modifier.Positions[i].MasterPosition) / masterDelta;
      }

      return modifier.Positions[i].Position + weight * delta;
   }

   const BoneConstraintMap& Avatar::getBoneConstraintMap() const
   {
       return BoneConstraints;
   }
   void Avatar::clearBoneConstraints()
   {
      BoneConstraints.clear();
   }

   void Avatar::setBoneConstraints(const std::string &bone, const BoneConstraintVector &constraints)
   {
      BoneConstraints[bone] = constraints;
      // Make sure the bone constraints for a single bone are ordered position first, then lookat
      std::sort(BoneConstraints[bone].begin(), BoneConstraints[bone].end());
   }

   void Avatar::addBoneConstraint(const std::string &bone, const BoneConstraint &constraint)
   {
      BoneConstraints[bone].push_back(constraint);
      // Make sure the bone constraints for a single bone are ordered position first, then lookat
      std::sort(BoneConstraints[bone].begin(), BoneConstraints[bone].end());
   }

   const PropertyMap& Avatar::getPropertyMap() const
   {
      return Properties;
   }

   void Avatar::setProperty(const std::string& key, const std::string& value)
   {
      Properties[key] = value;
   }

   const std::string& Avatar::getProperty(const std::string& key)
   {
      static std::string empty;

      PropertyMap::iterator i = Properties.find(key);
      if (i != Properties.end())
      {
         return i->second;
      }
      else return empty;
   }

   bool Avatar::hasProperty(const std::string& key) const
   {
      PropertyMap::const_iterator i = Properties.find(key);
      if (i != Properties.end())
      {
         if (!i->second.empty()) return true;
      }
      return false;
   }

   void Avatar::unsetProperty(const std::string& key)
   {
      PropertyMap::iterator i = Properties.find(key);
      if (i != Properties.end())
      {
         Properties.erase(i);
      }
   }

   void Avatar::clearProperties()
   {
      Properties.clear();
   }

   // virtual
   size_t Avatar::getNumExtraAnimatedSkeletons() const
   {
      return ExtraSkeletons.size();
   }

   // virtual 
   const std::string &Avatar::getExtraAnimatedSkeleton(size_t index) const
   {
      assert(index > 0 && index < ExtraSkeletons.size());
      return ExtraSkeletons[index];
   }

   void Avatar::addExtraAnimatedSkeleton(const std::string &name)
   {
      ExtraSkeletons.push_back(name);
   }
}
