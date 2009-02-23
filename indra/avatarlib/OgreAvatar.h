// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __OgreAvatar_h__
#define __OgreAvatar_h__

#include "Avatar.h"
#include "AvatarAttachment.h"
#include <OGRE/Ogre.h>
#include "Cloth.h"


namespace Rex
{
   //! Ogre specific realXtend avatar implementation.
   /*!
       \todo Only height and mesh, no weight, materials, skeletons, sliders..
   */
   class OgreAvatar : public Avatar
   {
   public:
      //! Constructor that takes Ogre entity and scenenode
      OgreAvatar(Ogre::Entity *ent, Ogre::SceneNode* node, Ogre::SceneManager *sceneManager);

      //! Destructor
      virtual ~OgreAvatar();

      //! Resets the avatar to basic state (no materials, attachment...)
      virtual void clear(bool deleteAttachments = true);

      //! Clears all attachments
      virtual void clearAttachments();

      //! Clones the mesh of the avatar if necessary. The clone is unloaded when the avatar is destroyed.
      virtual Ogre::MeshPtr cloneMesh(const Ogre::MeshPtr &mesh);

      //! Returns ogre entity used by this avatar
      virtual const Ogre::Entity *getEntity() const;

      //! Returns ogre entity used by this avatar
      virtual Ogre::Entity *getEntity();

      //! Returns Ogre SceneNode used by this avatar
      virtual const Ogre::SceneNode *getSceneNode() const;

      //! Returns Ogre SceneNode used by this avatar
      virtual Ogre::SceneNode *getSceneNode();

      //! Set height of avatar in units
      virtual void setHeight(Ogre::Real height);

      //! returns the original height of the avatar
      float getDefaultHeight() { return DefaultHeight; }

      //! Sets mesh
      /*!
          \param mesh mesh
      */
      virtual void setMesh(const std::string &mesh);

      //! Sets new entity and scenenode for this avatar. The entity and scenenode are left untouched, except for attachments.
      /*!
          \param entity new entity to replace old one
          \param node new scenenode to replace old one
          \param attach Attach all attachments automatically. If set to false, attachments need to be attached manually.
      */
      virtual void setPhysicalAppearance(Ogre::Entity *entity, Ogre::SceneNode *node, bool attachNow = true);

      //! Returns true if Ogre entity for this avatar has been created
      virtual bool hasPhysicalAppearance() const { return getEntity() != 0; };

      //! Adds new attachment to avatar
      /*! The returned attachment is not the same as the supplied attachment, if the same attachment
          is already in the avatar. The copy is returned in that case, the original attachment that was supplied
          should be deleted and the returned copy used instead.

          \note If the attachment was added properly, the avatar takes care of deleting it after it's no longer needed.

          \param attachment Attachment to add
          \return Supplied attachment if it was added properly, copy of the attachment if the attachment already existed, or zero if there was error.
      */
      virtual AvatarAttachment *addAttachment(AvatarAttachment *attachment);

      //! Remove the physical appearance of an attachment. Doesn't delete the actual attachment.
      /*!
          \note Removes the attachment based on it's category
      */
      virtual void removeAttachmentByCategory(AvatarAttachment *attachment);

      //! Remove the physical appearance of an attachment. Doesn't delete the actual attachment.
      /*! 
          \note Removes the attachment from all the bones
      */
      virtual void removeAttachment(AvatarAttachment *attachment);

      //! Remove the physical appearance of an attachment. Doesn't delete the actual attachment.
      /*!
          \param index Index of the attachment
          \param bone Name of the bone where to remove the attachment. Leave empty to remove from all bones.
      */
      virtual void removeAttachment(size_t index, const std::string &bone = std::string());

      //! Attach attachment to bone(s) based on it's category
      virtual void attachByCategory(AvatarAttachment *attachment);

      //! Attach attachment at index to bone
      /*!
          \param index Index of the attachment
          \param bone Which bone to attach to
      */
      virtual void attach(size_t index, const std::string &bone);

      //! Set weight of avatar in range [0,1], where 0 is thin, 1 is heavy
      virtual void setWeight(Ogre::Real weight);

      //! Removes all attachments
      virtual void removeAllAttachments();

      //! Physically attaches all attachments to avatar
      virtual void attachAll();

      //! Creates a new empty avatar
      /*!
          \param uid Avatar id
          \param sceneManager Ogre scenemanager that contains this avatar
      */
      static OgreAvatar *create(const std::string &uid, Ogre::SceneManager *sceneManager);

      //! Creates a new avatar from file.
      /*!
          \note Doesn't create Ogre scenenode or entity. Call setPhysicalAppearance() to
                set them, and to set the avatar height proper.
      */
      static OgreAvatar *create(const Ogre::String &name, const std::string &uid, Ogre::SceneManager *sceneManager);

      //! Returns an attachment associated with this avatar. The attachment may not actually be attached.
      virtual const OgreAvatarAttachment *getAttachment(size_t index) const;

      //! Set texture at position. Changes the texture from the material also.
      virtual void setTexture(TEXTURE position, const std::string &name);

      //! Set material. Changes the material for the entity also, if one is specified
      virtual void setMaterial(size_t index, const std::string &name);

      //! Get material. If material at specified index doesn't exist, finds the material from entity.
      virtual const std::string &getMaterial(size_t index) const;

      //! Returns number of physics clothes on this avatar
      virtual size_t getNumPhysicsClothes() const { return PhysicsCloth.size(); }

      //! Returns physics cloth
      virtual const ClothPtr &getPhysicsCloth(size_t index) const { return PhysicsCloth[index]; }

      //! Returns physics cloth
      virtual ClothPtr &getPhysicsCloth(size_t index) { return PhysicsCloth[index]; }

      //! Updates list of morph modifiers from current mesh
      virtual void getMorphModifiersFromMesh();

      //! (re)adds dynamic animations to entity and applies them
      virtual void addDynamicAnimations();

      //! Applies morph modifier to current entity
      virtual void applyMorphModifier(const MorphModifier& morph);

      //! Applies morph modifier to the specified entity, for internal use
      virtual void _applyMorphModifier(const MorphModifier& morph, Ogre::Entity *entity);

      //! Applies texture modifier to current entity
      virtual void applyTextureModifier(const TextureModifier& textureMod);

      //! Recalculates and applies all master appearance modifiers
      virtual void applyMasterModifiers();

      //! Applies bone constraints ("stretchy bones") to whole avatar skeleton 
      virtual void applyBoneConstraints();

      //! Gets offset of specific "base bone" for automatic offset height adjustment. Returns zero vector if no basebone set or no skeleton
      virtual Ogre::Vector3 getBaseOffset();

      //! Resource group for avatar meshes
      static const std::string MESH_RESOURCEGROUP;
      //! Resource group for avatar texture group 0 (face)
      static const std::string TEXTURE0_RESOURCEGROUP;
      //! Resource group for avatar texture group 1 (body)
      static const std::string TEXTURE1_RESOURCEGROUP;

   protected:
      //! constructor
      OgreAvatar();

      //! constructor with uid
      OgreAvatar(const std::string &uid);

      //! Applies height and weight to scenenode
      virtual void calculateAndApplySize();

      //! Applies texture to texture position
      void applyTexture(TEXTURE position, const std::string &name);

      //! Returns material based on texture position, f.ex. face material.
      Ogre::MaterialPtr getMaterialFromTexturePosition(TEXTURE position);

      //! Applies bone constraints to a specific bone
      virtual void applyBoneConstraints(Ogre::SkeletonInstance *skeleton, Ogre::Bone *bone);

      //! Ogre entity
      Ogre::Entity *Entity;

      //! Ogre scenenode
      Ogre::SceneNode *SceneNode;

      //! Scenemanager for this avatar
      Ogre::SceneManager *SceneManager;

      //! Array of physics clothes
      std::vector<ClothPtr> PhysicsCloth;

      //! Optional cloned mesh for the avatar
      Ogre::MeshPtr CloneMesh;

      //! the default (original) height of the avatar
      Ogre::Real DefaultHeight;

      //! List of polygons that should be removed from the avatar entity
      AvatarAttachment::PolygonMap PolygonsToRemove;
   };
}

#endif // __OgreAvatar_h__
