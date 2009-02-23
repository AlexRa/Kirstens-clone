// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "AvatarAttachment.h"

namespace Rex
{
   const std::string AvatarAttachment::AttachmentResourceGroup = "Attachments";

///////////////////////////////////////////////

   bool Offset::operator ==(const Offset &rhs) const
   {
      return (Translation == rhs.Translation && Orientation == rhs.Orientation &&
              Scale == rhs.Scale);
   }
   bool Offset::operator !=(const Offset &rhs) const
   {
      return !(*this == rhs);
   }

///////////////////////////////////////////////

   AvatarAttachment::AvatarAttachment(const AvatarAttachment &rhs)
   {
      *this = rhs;
   }

   void AvatarAttachment::operator = (const AvatarAttachment &rhs)
   {
      Name = rhs.Name;
      Mesh = rhs.Mesh;

      AvatarMesh = rhs.AvatarMesh;

      ClothName = rhs.ClothName;

      ClothInstance = rhs.ClothInstance;

      MaterialList = rhs.MaterialList;

      LinkSkeleton = rhs.LinkSkeleton;

      InheritOrientation = rhs.InheritOrientation;

      BoneMap = rhs.BoneMap;
   }


   bool AvatarAttachment::operator == (const AvatarAttachment& rhs) const
   {
      return Mesh == rhs.Mesh && Name == rhs.Name;
//      return (Mesh == rhs.Mesh && BoneMap == rhs.BoneMap);
   }

   bool AvatarAttachment::operator != (const AvatarAttachment& rhs) const
   {
      return !(*this == rhs);
   }

   void AvatarAttachment::setAvatarPolygon(unsigned long idx, bool select)
   {
      if (AvatarPolygons.find(idx) == AvatarPolygons.end())
      {
         AvatarPolygons[idx] = select;
      } else
      {
//         AvatarPolygons[idx] = !AvatarPolygons[idx];
         AvatarPolygons[idx] = select;
      }
   }

   bool AvatarAttachment::isAttached() const
   {
      AttachmentBoneMap::const_iterator iter = BoneMap.begin();
      for ( ; iter != BoneMap.end() ; ++iter)
      {
         if (iter->second.IsAttached)
         {
            return true;
         }
      }

      return false;
   }
}
