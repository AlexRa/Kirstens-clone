// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "AvatarAttachmentSerializer.h"
#include "RexException.h"
#include "XmlManager.h"
#include "XmlFile.h"

namespace Rex
{
   AvatarAttachmentSerializer::AvatarAttachmentSerializer()
   {
   }

   AvatarAttachmentSerializer::~AvatarAttachmentSerializer()
   {
   }

   void AvatarAttachmentSerializer::exportAttachment(const AvatarAttachment *attachment, const std::string &filename)
   {
      assert(attachment);
      assert(filename.empty() == false);

      static const char *skeleton = 
         "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>"
         "<!-- Attachment data -->"
         "<attachment>"
         "</attachment>";

      Rex::XmlDocument doc;
      doc.Parse(skeleton);
      Rex::XmlElement *root = doc.RootElement();

      exportAttachment(attachment, root);
      
      bool result = doc.SaveFile(filename);
      if (!result)
      {
         std::string message("Failed to save to file: ");
         message.append(filename);
#if LL_WINDOWS
         MessageBoxA( NULL, message.c_str(), "Error while writing to file.", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#endif
      }
   }

   void AvatarAttachmentSerializer::exportAttachment(const AvatarAttachment *attachment, XmlElement *root, bool onlyAttached)
   {
      assert(attachment);
      assert(root);

      TiXmlElement name("name");
      name.SetAttribute("value", attachment->Name);
      root->InsertEndChild(name);

      TiXmlElement item("mesh");
      item.SetAttribute("name", attachment->Mesh);
      item.SetAttribute("linkskeleton", attachment->LinkSkeleton);
      if (attachment->ClothName.empty() == false)
      {
         item.SetAttribute("cloth", attachment->ClothName);
      }
      root->InsertEndChild(item);

      TiXmlElement categoryItem("category");
      categoryItem.SetAttribute("name", attachment->Category.Name);
      size_t n;
      for (n=0 ; n<attachment->Category.Bones.size() ; ++n)
      {
         TiXmlElement itemBone("bone");
         itemBone.SetAttribute("name", attachment->Category.Bones[n]);

         categoryItem.InsertEndChild(itemBone);
      }
      root->InsertEndChild(categoryItem);

      TiXmlElement avatarItem("avatar");
      //Ogre::String meshName;
      //if (AvatarEntity)
      //{
      //   meshName = AvatarEntity->getMesh()->getName();
      //}
      //avatarItem.SetAttribute("name", meshName);
      avatarItem.SetAttribute("name", attachment->AvatarMesh);
      AttachmentBoneMap::const_iterator iter = attachment->BoneMap.begin();
      for ( ; iter != attachment->BoneMap.end() ; ++iter)
      {
         if (onlyAttached && iter->second.IsAttached == false)
            continue;

         TiXmlElement itemBone("bone");
         itemBone.SetAttribute("name", iter->first);
         Rex::XmlFile::setAttribute(&itemBone, "offset", iter->second.BoneOffset.Translation);
         Rex::XmlFile::setAttribute(&itemBone, "rotation", iter->second.BoneOffset.Orientation);
         Rex::XmlFile::setAttribute(&itemBone, "scale", iter->second.BoneOffset.Scale);

         avatarItem.InsertEndChild(itemBone);
      }

      AvatarAttachment::PolygonMap::const_iterator i = attachment->AvatarPolygons.begin();
      for ( ; i != attachment->AvatarPolygons.end() ; ++i)
      {
         if (i->second)
         {
            TiXmlElement itemPolygon("avatar_polygon");
            itemPolygon.SetAttribute("idx", i->first);

            avatarItem.InsertEndChild(itemPolygon);
         }
      }
      root->InsertEndChild(avatarItem);


   }

   void AvatarAttachmentSerializer::importAttachment(const std::string &filename, AvatarAttachment *attachment)
   {
      assert(filename.empty() == false);
      assert(attachment);

      XmlFilePtr file;
      file = XmlManager::getSingleton().load(filename, AvatarAttachment::AttachmentResourceGroup);

      XmlDocument *doc = file->getDocument();
      assert(doc);
      if (doc)
      {
         const Rex::XmlElement *root_element = doc->RootElement();
         if (root_element)
         {
            importAttachment(root_element, attachment);
         } else
         {
            REX_EXCEPT(RexException::ERR_INTERNAL_ERROR, "Failed to parse xml file.", attachment->Name);
         }
      }
   }

   void AvatarAttachmentSerializer::importAttachment(const XmlElement *root, AvatarAttachment *attachment)
   {
      assert(root);
      assert(attachment);

      const Rex::XmlElement *attachment_element_name = root->FirstChildElement("name");
      if (attachment_element_name)
      {
         const std::string *attachment_name = attachment_element_name->Attribute(std::string("value"));
         if (attachment_name)
         {
            attachment->Name = *attachment_name;
         }
      }

      const Rex::XmlElement *attachment_element = root->FirstChildElement("mesh");
      if (!attachment_element)
      {
         REX_EXCEPT(RexException::ERR_INTERNAL_ERROR, "Missing element 'mesh'.", attachment->Name);
      }

      const std::string *name = attachment_element->Attribute(std::string("name"));
      if (name)
      {
         int linkSkeletons = 0;
         attachment_element->QueryIntAttribute("linkskeleton", &linkSkeletons);
         const std::string *cloth = attachment_element->Attribute(std::string("cloth"));

         attachment->Mesh = *name;
         attachment->LinkSkeleton = (linkSkeletons != 0);
         if (cloth)
         {
            attachment->ClothName = *cloth;
         }

         const Rex::XmlElement *category_element = root->FirstChildElement("category");
         if (category_element)
         {
            const std::string *name = category_element->Attribute(std::string("name"));
            if (name)
            {
               attachment->Category.Name = *name;
               const Rex::XmlElement *bone_element = category_element->FirstChildElement("bone");
               while (bone_element)
               {
                  const Rex::XmlElement *current = bone_element;
                  bone_element = bone_element->NextSiblingElement("bone");

                  const std::string *bone = current->Attribute(std::string("name"));
                  if (bone)
                  {
                     attachment->Category.Bones.push_back(*bone);
                  }
               }
            }
         }

         const Rex::XmlElement *avatar_element = root->FirstChildElement("avatar");
         if (avatar_element)
         {
            const Rex::XmlElement *bone_element = avatar_element->FirstChildElement("bone");
            const std::string *name = avatar_element->Attribute(std::string("name"));
            if (name)
            {
               attachment->AvatarMesh = *name;
            }
            while (bone_element)
            {
               const Rex::XmlElement *current = bone_element;
               bone_element = bone_element->NextSiblingElement("bone");

               const std::string *name = current->Attribute(std::string("name"));
               if (!name)
                  continue;

               Ogre::Vector3 offset = Ogre::Vector3::ZERO;
               Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
               Ogre::Vector3 scale = Ogre::Vector3::UNIT_SCALE;
               Rex::XmlFile::queryVector3Attribute(current, "offset", &offset);
               Rex::XmlFile::queryQuaternionAttribute(current, "rotation", &rotation);
               Rex::XmlFile::queryVector3Attribute(current, "scale", &scale);

               attachment->BoneMap[*name].BoneOffset.Translation = offset;
               attachment->BoneMap[*name].BoneOffset.Orientation = rotation;
               attachment->BoneMap[*name].BoneOffset.Scale = scale;

               attachment->BoneMap[*name].Entity = 0;
               attachment->BoneMap[*name].Node = 0;
               attachment->BoneMap[*name].IsAttached = false;
            }

            const Rex::XmlElement *polygon_element = avatar_element->FirstChildElement("avatar_polygon");
            while (polygon_element)
            {
               const Rex::XmlElement *current = polygon_element;
               polygon_element = polygon_element->NextSiblingElement("avatar_polygon");

               int index = 0;
               if (current->QueryIntAttribute(std::string("idx"), &index) == TIXML_SUCCESS)
               {
                  attachment->setAvatarPolygon(index);
               }
            }
         }
      } else
      {
         Ogre::LogManager::getSingleton().logMessage("Warning: missing mesh name in attachment: " + attachment->Name);
      }
   }
}
