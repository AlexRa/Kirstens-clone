// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __AvatarAttachment_h__
#define __AvatarAttachment_h__

#include "Cloth.h"
#include <OGRE/Ogre.h>

namespace Rex
{
   //! Category for attachment
   struct AttachmentCategory
   {
      //! Name of the category
      std::string Name;

      //! List of bones the attachment in the category should be attached to
      Ogre::StringVector Bones;
   };

   typedef std::vector<AttachmentCategory> AttachmentCategoryVector;
   typedef Ogre::SharedPtr<AttachmentCategoryVector> AttachmentCategoryPtr;

   //! Offset for bone
   struct Offset
   {
      Ogre::Vector3 Translation;
      Ogre::Quaternion Orientation;
      Ogre::Vector3 Scale;

      bool operator ==(const Offset &rhs) const;
      bool operator !=(const Offset &rhs) const;
   };

   struct AttachmentBone
   {
      //! Default constructor
      AttachmentBone() : IsAttached(false), Entity(0), Node(0) {}

      //! Offset for the bone
      Offset BoneOffset;

      //! (Optional) Avatar mesh for the bone
      std::string AvatarMesh;

      //! Is attached
      bool IsAttached;

      Ogre::Entity *Entity;
      Ogre::Node *Node;
   };

   typedef std::map<std::string, AttachmentBone> AttachmentBoneMap;

   //! Attachment to avatar. Attachments are meshes attached to a bone.
   /*! One attachment can be attached to several bones.
   */
   class AvatarAttachment
   {
   public:
      typedef std::map<unsigned long, bool> PolygonMap;

      bool operator == (const AvatarAttachment& rhs) const;
      bool operator != (const AvatarAttachment& rhs) const;

      //! default constructor
      AvatarAttachment() {}

      //! Copy constructor
      AvatarAttachment(const AvatarAttachment &rhs);

      void operator = (const AvatarAttachment &rhs);

      //! Add a polygon to a list of polygons that should be removed from the avatar
      void setAvatarPolygon(unsigned long idx, bool select = true);

      //! Returns true if attachment is attached to the avatar
      bool isAttached() const;

      //! List of avatar polygons that need to be removed to avoid clipping with this attachment
      PolygonMap AvatarPolygons;

      //! Name
      std::string Name;

      //! mesh
      std::string Mesh;

      //! Optional avatar mesh this attachment is related to
      std::string AvatarMesh;

      //! Name of the cloth file associated with this attachment
      std::string ClothName;

      //! Actual cloth instance used by this attachment.
      //! \remark Not necessarily the same as ClothName, because ClothName specifies the
      //!         base file used for the cloth physics.
      ClothPtr ClothInstance;

      //! Materials used by the mesh
      std::vector<std::string> MaterialList;

      //! Link skeleton of the attachment to the skeleton of the avatar
      bool LinkSkeleton;

      //! Inherit orientation from parent
      bool InheritOrientation;

      AttachmentBoneMap BoneMap;

      //! Category for the attachment
      AttachmentCategory Category;

      //! Resource group for attachment files
      static const std::string AttachmentResourceGroup;
   };

   typedef AvatarAttachment OgreAvatarAttachment;
}

#endif // __AvatarAttachment_h__
