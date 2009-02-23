// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __AvatarSerializer_h__
#define __AvatarSerializer_h__

#include "XmlFile.h"
#include "DynamicAnimation.h"
#include "Avatar.h"
#include <OGRE/OgreSerializer.h>

namespace Rex
{
   //! Export / import functions for realXtend Avatar.
   class AvatarSerializer : public Ogre::Serializer
   {
   public:
      //! default constructor
      AvatarSerializer();

      //! destructor
      virtual ~AvatarSerializer();

      //! Exports non-implementation specific avatar data to xml
      /*!
          \param avatar Avatar to export
          \param doc Xml document to export to
          \param animations Extra dynamic animations (bone deforms) to export with the avatar. These might be specified for the entity used by the avatar, but not specified for the avatar itself.
          \param exportTextures If true, exports face and body textures also.
          \param exportDeformsFull Exports dynamic animations fully, not just in parametric form
      */
      void exportAvatar(const Avatar *avatar, XmlDocument *doc, const std::vector<DynamicAnimationPtr> &animations = std::vector<DynamicAnimationPtr>(), bool exportTextures = false, bool exportDeformsFull = false);

      //! Import avatar from xml. Wipes the destination avatar before importing new one.
      /*! \todo Does not handle skeleton

          \param doc Xml document to import from
          \param avatarDest Avatar object to import to
          \param removeAttachments All old attachment are completely deleted before importing new avatar.
          \return True on success, false otherwise
      */
      bool importAvatar(const XmlDocument *doc, Avatar *avatarDest,  bool deleteAttachments = true);

      //! Import mesh-specific data (animation names/parameters, complex deformations) from an additional xml file. Wipes those parts of destination avatar before import. 
      /*! Note that if this data is already present in the main avatar xml file, it will be imported by importAvatar(); 
       */
      bool importMeshSpecificData(const XmlDocument *doc, Avatar *avatarDest);

      //! Xml Template for avatar export
      static const char* XMLTEMPLATE;

   protected:
      bool importDynamicAnimations(const XmlDocument *doc, Avatar *avatarDest);
      bool importAnimationDef(const XmlElement *anim_element, Avatar *avatarDest);
      bool importMorphModifier(const XmlElement *morph_element, Avatar *avatarDest);
      bool importTextureModifier(const XmlElement *morph_element, Avatar *avatarDest);
      bool importMasterModifier(const XmlElement *master_element, Avatar *avatarDest);
      bool importBoneConstraint(const XmlElement *boneconstraint_element, Avatar *avatarDest);

      void exportAnimationDef(const std::string &name, const AnimationDef &animDef, XmlDocument *doc);
      void exportMorphModifier(const std::string &name, const MorphModifier &morph, XmlDocument *doc);
      void exportTextureModifier(const std::string &name, const TextureModifier &textureMod, XmlDocument *doc);
      void exportMasterModifier(const std::string &name, const MasterModifier &masterModifier, XmlDocument *doc, const Avatar *avatarSrc);
      void exportBoneConstraint(const std::string &boneName, const BoneConstraint &constraint, XmlDocument *doc);
   };
} // namespace Rex

#endif // __AvatarSerializer_h__
