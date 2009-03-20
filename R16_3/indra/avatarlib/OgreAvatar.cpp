// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "OgreAvatar.h"
#include "DynamicAnimationManager.h"
#include "DynamicAnimation.h"
#include "XmlManager.h"
#include "XmlFile.h"
#include "AvatarSerializer.h"
#include "ClothManager.h"
#include <OGRE/Ogre.h>
#include <OGRE/OgreTagPoint.h>


namespace Rex
{
   const std::string OgreAvatar::MESH_RESOURCEGROUP = "AvatarMesh";
   const std::string OgreAvatar::TEXTURE0_RESOURCEGROUP = "AvatarTextureFace";
   const std::string OgreAvatar::TEXTURE1_RESOURCEGROUP = "AvatarTextureBody";

   OgreAvatar::OgreAvatar() : Avatar(), Entity(0), SceneNode(0), DefaultHeight(0)
   {
   }

   OgreAvatar::OgreAvatar(const std::string &uid) : Avatar(uid), Entity(0), SceneNode(0), DefaultHeight(0)
   {

   }
   OgreAvatar::OgreAvatar(Ogre::Entity *ent, Ogre::SceneNode *node, Ogre::SceneManager *sceneManager) : Avatar(), Entity(ent), SceneNode(node), SceneManager(sceneManager), DefaultHeight(0)
   {
      if (ent != 0 && node != 0)
      {
         setMesh(ent->getMesh()->getName());

         setPhysicalAppearance(Entity, SceneNode);
      }
   }

   OgreAvatar::~OgreAvatar()
   {
      clear();
   }

   void OgreAvatar::clear(bool deleteAttachments)
   {
      removeAllAttachments();

      Entity = 0;
      SceneNode = 0;
      PolygonsToRemove.clear();

      if (CloneMesh.isNull() == false)
      {  
         Ogre::ResourcePtr castPtr = static_cast<Ogre::ResourcePtr>(CloneMesh);
         Ogre::MeshManager::getSingleton().remove(castPtr);
         CloneMesh.setNull();
      }

      Avatar::clear(deleteAttachments);
//      OgreAttachmentList.clear();
   }

   void OgreAvatar::clearAttachments()
   {
      removeAllAttachments();
      PolygonsToRemove.clear();
      Avatar::clearAttachments();
   }

   Ogre::MeshPtr OgreAvatar::cloneMesh(const Ogre::MeshPtr &mesh)
   {
      // First lets see if we need to clone the mesh
      bool needClone = false;

      size_t n;
      for (n=0 ; n<AttachmentList.size() ; ++n)
      {
         if (AttachmentList[n]->AvatarPolygons.empty() == false)
         {
            needClone = true;
            break;
         }
      }

      if (!needClone)
         return mesh;

      
      // Clone the mesh, so we can remove some polygons from it without affecting all avatars using the same mesh
      Ogre::String meshName = mesh->getName() + Uid + "_clone";
      Ogre::LogManager::getSingleton().logMessage("Cloning mesh" + meshName);

      Ogre::MeshPtr clonedMesh = Ogre::MeshManager::getSingleton().getByName(meshName);
      if (clonedMesh.isNull())
      {
         try
         {
            clonedMesh = mesh->clone(meshName);
            CloneMesh = clonedMesh;
            setMesh(meshName);

            Ogre::LogManager::getSingleton().logMessage("Mesh cloned" + meshName);
         } catch (Ogre::Exception)
         {
            Ogre::LogManager::getSingleton().logMessage("Failed to clone mesh, using the original mesh.");
            clonedMesh = mesh;
         }
      } else
      {
         setMesh(meshName);
         Ogre::LogManager::getSingleton().logMessage("Cloned mesh exists! " + meshName);
      }

      return clonedMesh;
   }

   const Ogre::Entity *OgreAvatar::getEntity() const
   {
      return Entity;
   }

   Ogre::Entity *OgreAvatar::getEntity()
   {
      return Entity;
   }

   const Ogre::SceneNode *OgreAvatar::getSceneNode() const
   {
      return SceneNode;
   }

   Ogre::SceneNode *OgreAvatar::getSceneNode()
   {
      return SceneNode;
   }

   void OgreAvatar::setHeight(Ogre::Real height)
   {
      Avatar::setHeight((float)height);

      calculateAndApplySize();
   }

   void OgreAvatar::setWeight(Ogre::Real weight)
   {
      Avatar::setWeight((float)weight);

      calculateAndApplySize();
   }


   void OgreAvatar::setMesh(const std::string &mesh)
   {
      Avatar::setMesh(mesh);
   }

   void OgreAvatar::setPhysicalAppearance(Ogre::Entity *entity, Ogre::SceneNode *node, bool attachNow)
   {
      assert(entity);
      assert(node);
      if (!entity || !node)
         return;

      if (Entity)
      {
         removeAllAttachments();
      }


//      if (SceneNode)
//         SceneNode->removeAndDestroyAllChildren();

      SceneNode = node;

//      if (Entity && SceneManager)
//         SceneManager->destroyEntity(Entity);

      Entity = entity;

      calculateAndApplySize();

      // See if avatar contains materials, and if it doesn't, create them
      size_t n;
      for (n=0 ; n<Entity->getNumSubEntities() ; ++n)
      {
         Ogre::String materialName = Entity->getSubEntity((unsigned int)n)->getMaterialName();
         Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(materialName);
         if (material.isNull())
         {
             Entity->getSubEntity((unsigned int)n)->getMaterial()->clone(materialName);
             Entity->getSubEntity((unsigned int)n)->setMaterialName(materialName); // reset material
         }
      }
      for (n=0 ; n<MaterialList.size() ; ++n)
      {
         if (n < Entity->getNumSubEntities())
         {
            Entity->getSubEntity((unsigned int)n)->setMaterialName(MaterialList[n]);
         }
      }

      // Set materials
      if (MaterialList.empty())
      {
         unsigned int i;
         for (i=0 ; i<Entity->getNumSubEntities() ; ++i)
         {
            MaterialList.push_back(Entity->getSubEntity(i)->getMaterialName());
         }
      }

      if (TextureList.find(Avatar::TEXTURE_FACE) != TextureList.end())
      {
         setTexture(Avatar::TEXTURE_FACE, TextureList[Avatar::TEXTURE_FACE]);
      }

      if (TextureList.find(Avatar::TEXTURE_BODY) != TextureList.end())
      {
         setTexture(Avatar::TEXTURE_BODY, TextureList[Avatar::TEXTURE_BODY]);
      }

      Ogre::LogManager::getSingleton().logMessage("Attaching meshes.");
      if (attachNow)
      {
         attachAll();

         Ogre::LogManager::getSingleton().logMessage("Removing unwanted polygons from avatar.");
         //AvatarAttachment::PolygonMap::const_iterator iter = PolygonsToRemove.begin();
         //for ( ; iter != PolygonsToRemove.end() ; ++iter)
         //{
        // if (iter->second)
        // {
         if (CloneMesh.isNull() == false) // Make sure we remove polygons only from a cloned mesh
         {
            Ogre::MeshPtr mesh = Entity->getMesh();
            unsigned short n;
            for (n=0 ; n<1 ; ++n)
            {
               Ogre::SubMesh *submesh = mesh->getSubMesh(n);
               Ogre::IndexData *data = submesh->indexData;

               Ogre::HardwareIndexBufferSharedPtr ibuf = data->indexBuffer;

               unsigned long* lIdx = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
               unsigned short* pIdx = reinterpret_cast<unsigned short*>(lIdx);
               bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

               size_t n;
               for (n=0 ; n<data->indexCount ; n += 3)
               {
                  if (PolygonsToRemove.find(pIdx[n]) != PolygonsToRemove.end() ||
                      PolygonsToRemove.find(pIdx[n+1]) != PolygonsToRemove.end() ||
                      PolygonsToRemove.find(pIdx[n+2]) != PolygonsToRemove.end())
                  {
                     if (n + 3 < data->indexCount)
                     {
                        size_t i;
                        for (i = n ; i<data->indexCount-3 ; ++i)
                        {
                           if (use32bitindexes)
                              lIdx[i] = lIdx[i+3];
                           else
                              pIdx[i] = pIdx[i+3];
                        }
                     }
                     data->indexCount -= 3;
                     n -= 3;
                     //unsigned short temp = pIdx[n+1];
                     //pIdx[n+1] =  pIdx[n+2];
                     //pIdx[n+2] = temp;
                  }
               }
               ibuf->unlock();
            }
         }
      }

      Ogre::LogManager::getSingleton().logMessage("Deforming avatar mesh.");

      addDynamicAnimations();
      // Calculate&apply effects of master modifiers on bones & morphs now
      applyMasterModifiers();
   }

   AvatarAttachment *OgreAvatar::addAttachment(AvatarAttachment *attachment)
   {
      assert(attachment->Mesh.empty() == false);

      if (attachment->Mesh.empty())
         return 0;

      std::vector<AvatarAttachment*>::iterator iter = AttachmentList.begin();
      for ( ; iter != AttachmentList.end() ; ++iter)
      {
         if (**iter == *attachment)
         {
            Rex::AvatarAttachment *oldAttachment = *iter;
            AttachmentList.erase(iter);
            AttachmentList.push_back(oldAttachment);

            return oldAttachment;
         }
      }

      if (std::find(AttachmentList.begin(), AttachmentList.end(), attachment) == AttachmentList.end())
      {
         AttachmentList.push_back(attachment);
      }

      return attachment;
   }

   void OgreAvatar::removeAttachmentByCategory(AvatarAttachment *attachment)
   {
      std::vector<AvatarAttachment*>::iterator iter = std::find(AttachmentList.begin(), AttachmentList.end(), attachment);
      if (iter != AttachmentList.end())
      {
         size_t index = std::distance(AttachmentList.begin(), iter);
         size_t n;
         for (n=0 ; n<attachment->Category.Bones.size() ; ++n)
         {
            removeAttachment(index, attachment->Category.Bones[n]);
         }
      }
   }

   void OgreAvatar::removeAttachment(AvatarAttachment *attachment)
   {
      if (!Entity)
         return;

      size_t n;
      for (n=0 ; n<AttachmentList.size() ; ++n)
      {
         if (AttachmentList[n] == attachment)
         {
            AttachmentBoneMap::const_iterator iter = attachment->BoneMap.begin();
            for ( ; iter != attachment->BoneMap.end() ; ++iter)
            {
               removeAttachment(n, iter->first);
            }
            break;
         }
      }
   }

   void OgreAvatar::removeAttachment(size_t index, const std::string &bone)
   {
      if (index < 0 || index >= AttachmentList.size())
         return;
      

      OgreAvatarAttachment &attachment = static_cast<OgreAvatarAttachment&>(*AttachmentList[index]);


      if (attachment.ClothInstance.isNull() == false)
      {
         std::vector<ClothPtr>::iterator iter = std::find(PhysicsCloth.begin(), PhysicsCloth.end(), attachment.ClothInstance);
         if (iter != PhysicsCloth.end())
         {
            iter->setNull();
            PhysicsCloth.erase(iter);
         }

         // Remove unused cloth
         Ogre::ResourcePtr castPtr = static_cast<Ogre::ResourcePtr>(attachment.ClothInstance);
         ClothManager::getSingleton().remove(castPtr);
         attachment.ClothInstance.setNull();
      }

      if (bone.empty())
      {
         AttachmentBoneMap::iterator iter = attachment.BoneMap.begin();
         for ( ; iter != attachment.BoneMap.end() ; ++iter)
         {
            if (iter->second.IsAttached == true)
            {
               iter->second.IsAttached = false;

               if (Entity->hasSkeleton())
               {
                  Ogre::SkeletonInstance *skeleton = Entity->getSkeleton();
                  Ogre::Bone *bone = 0;
                  try
                  {
                     bone = skeleton->getBone(iter->first);
                  } catch (Ogre::Exception e) { continue; }

                  if (bone)
                  {
                     if (iter->second.Entity->sharesSkeletonInstance())
                        iter->second.Entity->stopSharingSkeletonInstance();

                     Entity->detachObjectFromBone(iter->second.Entity);
                     SceneManager->destroyEntity(iter->second.Entity);
                     iter->second.Entity = 0;
                  } else
                  {
                     try
                     {
                        if (iter->second.Entity->sharesSkeletonInstance())
                           iter->second.Entity->stopSharingSkeletonInstance();

                        static_cast<Ogre::SceneNode*>(iter->second.Node)->removeAndDestroyAllChildren();
                        SceneManager->destroyEntity(iter->second.Entity);
                        iter->second.Entity = 0;
                        iter->second.Node = 0;
                     } catch (Ogre::Exception) { }
                  }
               } else
               {
                  try
                  {
                     if (iter->second.Entity->sharesSkeletonInstance())
                        iter->second.Entity->stopSharingSkeletonInstance();

                     static_cast<Ogre::SceneNode*>(iter->second.Node)->removeAndDestroyAllChildren();
                     SceneManager->destroyEntity(iter->second.Entity);
                     iter->second.Node = 0;
                     iter->second.Entity = 0;
                  } catch (Ogre::Exception) { }
               }
            }
         }
      } else
      {
         AttachmentBoneMap::iterator iter = attachment.BoneMap.find(bone);
         if (iter != attachment.BoneMap.end())
         {
            if (iter->second.IsAttached == true)
            {
               if (iter->second.Entity->sharesSkeletonInstance())
               {
                   iter->second.Entity->stopSharingSkeletonInstance();
               }

               iter->second.IsAttached = false;
               if (Entity->hasSkeleton())
               {
                  Ogre::SkeletonInstance *skeleton = Entity->getSkeleton();
                  Ogre::Bone *bone = 0;
                  try
                  {
                     bone = skeleton->getBone(iter->first);
                  } catch (Ogre::Exception e){ }

                  if (bone)
                  {
                     Entity->detachObjectFromBone(iter->second.Entity);
                     SceneManager->destroyEntity(iter->second.Entity);
                     iter->second.Entity = 0;
                  } else
                  {
                     try
                     {
                        static_cast<Ogre::SceneNode*>(iter->second.Node)->removeAndDestroyAllChildren();
                        SceneManager->destroyEntity(iter->second.Entity);
                        iter->second.Entity = 0;
                        iter->second.Node = 0;
                     } catch (Ogre::Exception) { }
                  }
               } else
               {
                  try
                  {
                     static_cast<Ogre::SceneNode*>(iter->second.Node)->removeAndDestroyAllChildren();
                     SceneManager->destroyEntity(iter->second.Entity);
                     iter->second.Entity = 0;
                     iter->second.Node = 0;
                  } catch (Ogre::Exception) { }
               }
            }
         }
      }
   }

   void OgreAvatar::attachByCategory(AvatarAttachment *attachment)
   {
      std::vector<AvatarAttachment*>::iterator iter = std::find(AttachmentList.begin(), AttachmentList.end(), attachment);
      if (iter != AttachmentList.end())
      {
         size_t index = std::distance(AttachmentList.begin(), iter);
         size_t n;
         for (n=0 ; n<attachment->Category.Bones.size() ; ++n)
         {
            attach(index, attachment->Category.Bones[n]);
         }
      }
   }

   void OgreAvatar::attach(size_t index, const std::string &bone)
   {
      if (index < 0 || index >= AttachmentList.size() || Entity == 0)
         return;

      OgreAvatarAttachment &attachment = static_cast<OgreAvatarAttachment&>(*AttachmentList[index]);
      
      if (attachment.Mesh.empty())
         return;
      
      std::string entityName = attachment.Name;
      entityName.append(bone);
      entityName.append("_entity");
      entityName.append(Ogre::StringConverter::toString(index));
      entityName.append(Uid);

      Ogre::LogManager::getSingleton().logMessage("Attaching mesh: " + attachment.Name);

      Ogre::Entity *entity;
      if (SceneManager->hasEntity(entityName))
         return;

      Ogre::MeshPtr originalMesh;
      try
      {
         originalMesh = Ogre::MeshManager::getSingleton().load(attachment.Mesh, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
      } catch (Ogre::Exception e)
      {
         Ogre::LogManager::getSingleton().logMessage("Failed to load mesh: " + attachment.Mesh);
         Ogre::MeshManager::getSingleton().remove(attachment.Mesh);
         return;
      }
      Ogre::MeshPtr mesh = originalMesh;

      if (attachment.ClothName.empty() == false)
      {
         Ogre::String cloneName = entityName + "_mesh";
         mesh = Ogre::MeshManager::getSingleton().getByName(cloneName);
         if (mesh.isNull())
            mesh = originalMesh->clone(cloneName);
      }

      if (mesh->hasSkeleton() == true && mesh->getSkeleton().isNull() == true)
      {
         if (attachment.LinkSkeleton == true)
         {
            Ogre::SkeletonPtr skel = Ogre::SkeletonManager::getSingleton().getByName(getSkeleton());
            if (skel.isNull() == false)
            {
               Ogre::LogManager::getSingleton().logMessage("Setting linked skeleton: " + skel->getName());
               mesh->_notifySkeleton(skel); // We need to use notifySkeleton here, as setSkeletonName() tries to (re)load the skeleton from file
            }
         } else
         {
            // Fyi this rather senseless piece of code is needed, otherwise when creating the entity the skeleton
            // can't be found and the viewer will crash
            Ogre::SkeletonPtr skeleton = Ogre::SkeletonManager::getSingleton().getByName(mesh->getSkeletonName());
            if (skeleton.isNull() == false)
            {
               mesh->_notifySkeleton(skeleton);
            }
         }
      }

      try
      {
//         entity = SceneManager->createEntity(entityName, attachment.Mesh);
         entity = SceneManager->createEntity(entityName, mesh->getName());
      } catch (Ogre::Exception e)
      {
         Ogre::LogManager::getSingleton().logMessage("Failed to create entity: " + entityName);
         //  no handling
         return;
      }

      if (attachment.LinkSkeleton == true && Entity->hasSkeleton() && entity->hasSkeleton() &&
         Entity->getSkeleton()->getName() == entity->getSkeleton()->getName())
      {
         entity->shareSkeletonInstanceWith(Entity);
      }

      Ogre::LogManager::getSingleton().logMessage("Creating cloth.");
      if (attachment.ClothName.empty() == false)
      {
         ClothPtr cloth;
         cloth = Rex::ClothManager::getSingleton().getByName(attachment.ClothName);
         if (cloth.isNull())
         {
            try
            {
               cloth = Rex::ClothManager::getSingleton().load(attachment.ClothName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            } catch (Ogre::Exception e)
            {
               Ogre::LogManager::getSingleton().logMessage("Failed to load cloth: " + attachment.ClothName + ".");
            }
         }
//            cloth->clone(entityName + "_cloth");
         attachment.ClothInstance = cloth->clone(entityName + "_cloth");
         attachment.ClothInstance->setMesh(mesh);
         attachment.ClothInstance->setParentNode(SceneNode);
         attachment.ClothInstance->startSimulation();
         attachment.ClothInstance->reform();
         PhysicsCloth.push_back(attachment.ClothInstance);

//            cloth->setMesh(mesh);
//            cloth->setParentNode(SceneNode);
//            cloth->startSimulation();
//            PhysicsCloth.push_back(cloth);
         
      }


      AttachmentBoneMap::iterator iter = attachment.BoneMap.find(bone);
      if (iter == attachment.BoneMap.end())
      {
         attachment.BoneMap[bone].IsAttached = false;
         attachment.BoneMap[bone].BoneOffset.Orientation = Ogre::Quaternion::IDENTITY;
         attachment.BoneMap[bone].BoneOffset.Translation = Ogre::Vector3::ZERO;
         attachment.BoneMap[bone].BoneOffset.Scale = Ogre::Vector3::UNIT_SCALE;
         attachment.BoneMap[bone].Node = 0;

         iter = attachment.BoneMap.find(bone);
      }
         if (iter->second.IsAttached == false)
         {
            if (Entity->hasSkeleton() == true)
            {
               Ogre::SkeletonInstance *skeleton = Entity->getSkeleton();
               Ogre::Bone *bone = 0;
               try
               {
                  bone = skeleton->getBone(iter->first);
               } catch(Ogre::Exception) {}

               if (bone)
               {
                  try
                  {
                     Ogre::TagPoint *tag = Entity->attachObjectToBone(iter->first, entity, iter->second.BoneOffset.Orientation, iter->second.BoneOffset.Translation);
                     tag->setScale(iter->second.BoneOffset.Scale);
                     iter->second.IsAttached = true;
                     iter->second.Entity = entity;
                     iter->second.Node = tag;
                  //   Ogre::LogManager::getSingleton().logMessage("Attaching to bone " + iter->first + "offset: " + Ogre::StringConverter::toString(iter->second.Translation));
                  } catch(Ogre::Exception e)
                  {
                     SceneManager->destroyEntity(entity);
                  }
               } else
               {
                  Ogre::SceneNode *node = SceneNode->createChildSceneNode(iter->second.BoneOffset.Translation, iter->second.BoneOffset.Orientation);
                  node->setScale(iter->second.BoneOffset.Scale);
                  iter->second.Node = node;
                  node->attachObject(entity);
                  iter->second.IsAttached = true;
                  iter->second.Entity = entity;
               }
            } else
            {
               Ogre::SceneNode *node = SceneNode->createChildSceneNode(iter->second.BoneOffset.Translation, iter->second.BoneOffset.Orientation);
               node->setScale(iter->second.BoneOffset.Scale);
               iter->second.Node = node;
               node->attachObject(entity);
               iter->second.IsAttached = true;
               iter->second.Entity = entity;
         //   SceneManager->destroyEntity(entity);
               
            }
         }
      //} else
      //{
      //   SceneManager->destroyEntity(entity);
      //   // Attach to scenenode
      ////   SceneNode->createChildSceneNode()->attachObject(entity);
      ////   attachment.IsAttached[iter->first] = true;
      //}

      std::vector<std::string> materials;
      unsigned int n;
      for (n=0 ; n<entity->getNumSubEntities() ; ++n)
         materials.push_back(entity->getSubEntity(n)->getMaterialName());

      attachment.MaterialList = materials;

      // Add polygons to remove from attachment to avatar
      AvatarAttachment::PolygonMap::const_iterator i = attachment.AvatarPolygons.begin();
      for ( ; i != attachment.AvatarPolygons.end() ; ++i)
      {
         if (i->second)
         {
            PolygonsToRemove[i->first] = true;
         }
      }
   }


   void OgreAvatar::calculateAndApplySize()
   {
      if (Entity)
      {
         DefaultHeight = Entity->getBoundingBox().getSize().y;
      }
      return;

      // This probably doesn't work well, so let's just skip it
      float height = getHeight();
      if (height > 0.0f && Entity && SceneNode)
      {
         Ogre::Vector3 scale = SceneNode->getScale();
         scale.y = height / DefaultHeight;

         SceneNode->setScale(scale);
      }

      //! \todo handle weight somehow?
   }

   
   void OgreAvatar::removeAllAttachments()
   {
//      assert(Entity);
      if (!Entity)
         return;

      // Remove attachment from old entity
      size_t n;
      for (n=0 ; n<AttachmentList.size() ; ++n)
      {
         AttachmentBoneMap::const_iterator iter = AttachmentList[n]->BoneMap.begin();
         for ( ; iter != AttachmentList[n]->BoneMap.end() ; ++iter)
         {
            removeAttachment(n, iter->first);
         }
      }
   }
   
   const OgreAvatarAttachment *OgreAvatar::getAttachment(size_t index) const
   {
      assert(index >= 0 && index <  getNumAttachments());
      return static_cast<const OgreAvatarAttachment*>(AttachmentList[index]);
   }
   
   void OgreAvatar::attachAll()
   {
      assert(Entity);

      size_t n;
      for (n=0 ; n<AttachmentList.size() ; ++n)
      {
         AttachmentBoneMap::const_iterator iter = AttachmentList[n]->BoneMap.begin(); 
         for ( ; iter != AttachmentList[n]->BoneMap.end() ; ++iter)
         {
            if (iter->second.IsAttached == false)
            {
               attach(n, iter->first);
            }
         }
      }
   }

   OgreAvatar *OgreAvatar::create(const std::string &uid, Ogre::SceneManager *sceneManager)
   {
      assert(sceneManager);
      
      OgreAvatar* avatar = new OgreAvatar(uid);
      avatar->SceneManager = sceneManager;

      return avatar;
   }

   OgreAvatar *OgreAvatar::create(const Ogre::String &name, const std::string &uid, Ogre::SceneManager *sceneManager)
   {
      assert(name.empty() == false);

      OgreAvatar* avatar = create(uid, sceneManager);
      if (!avatar || name.empty() || !sceneManager)
         return 0;
      //assert(sceneManager);

      //if (name.empty() || !sceneManager)
      //   return 0;

      //OgreAvatar* avatar = new OgreAvatar(uid);
      //avatar->SceneManager = sceneManager;
      XmlFilePtr xml;
      Ogre::LogManager::getSingleton().logMessage("Creating new avatar...");
      try
      {
         xml = XmlManager::getSingleton().load(name, XmlManager::XMLDATA_RESOURCE_GROUP_NAME);
       //  Ogre::LogManager::getSingleton().logMessage("Failed to load avatar from " + name + ".");
      } catch(Ogre::Exception e)
      {
         delete avatar;
         return 0;
      }
      if (xml.isNull())
      {
         delete avatar;
         return 0;
      }

      AvatarSerializer serializer;
      if (serializer.importAvatar(xml->getDocument(), avatar) == false)
      {
         Ogre::LogManager::getSingleton().logMessage("Failed Serializing new avatar...");
         delete avatar;
         return 0;
      }

      Ogre::LogManager::getSingleton().logMessage("Avatar created.");


      return avatar;
   }

   void OgreAvatar::setTexture(TEXTURE position, const std::string &name)
   {
      Avatar::setTexture(position, name);
      
      applyTexture(position, name);
   }

   void OgreAvatar::setMaterial(size_t index, const std::string &name)
   {
      Avatar::setMaterial(index, name);

      if (Entity)
      {
         if (index < Entity->getNumSubEntities())
         {
            try
            {
               Entity->getSubEntity(static_cast<unsigned int>(index))->setMaterialName(name);
            } catch (Ogre::Exception e) { /* Doesn't matter if material doesn't exist */ }
         }
      }
   }

   const std::string &OgreAvatar::getMaterial(size_t index) const
   {
      if (index < MaterialList.size())
         return MaterialList[index];

      if (Entity)
      {
         if (index < Entity->getNumSubEntities())
         {
            return (Entity->getSubEntity(static_cast<unsigned int>(index))->getMaterialName());
         }
      }

      REX_EXCEPT(RexException::ERR_ITEM_NOT_FOUND, 
			"material not found",
			"OgreAvatar::getMaterial");
   }

   void OgreAvatar::applyTexture(TEXTURE position, const std::string &name)
   {
      Ogre::MaterialPtr material = getMaterialFromTexturePosition(position);
      if (material.isNull() == false)
      {
         Ogre::Technique *technique = material->getTechnique(0);
         if (!technique)
            technique = material->createTechnique();

         Ogre::Pass *pass = technique->getPass(0);
         if (!pass)
            pass = technique->createPass();

          Ogre::TextureUnitState *tu = 0;
         if (pass->getNumTextureUnitStates() == 0)
            tu = pass->createTextureUnitState();
         else
            tu = pass->getTextureUnitState(0);

         tu->setTextureName(name);
      }
   }

   Ogre::MaterialPtr OgreAvatar::getMaterialFromTexturePosition(TEXTURE position)
   {
      if (Entity)
      {
         switch (position)
         {
         case TEXTURE_FACE:
            {
               if (Entity->getNumSubEntities() > 1)
                  return (Entity->getSubEntity(1)->getMaterial());
            }
            break;

         case TEXTURE_BODY:
            {
               if (Entity->getNumSubEntities() > 0)
                  return (Entity->getSubEntity(0)->getMaterial());
            }
            break;
         }
      }

      return Ogre::MaterialPtr();
   }

   void OgreAvatar::getMorphModifiersFromMesh()
   {
      // Clear existing modifiers
      MorphModifiers.clear();

      Ogre::Entity *entity = getEntity();
      if (entity)
      {
         // Add all pose animations
         Ogre::MeshPtr mesh = entity->getMesh();
         size_t numAnims = mesh->getNumAnimations();
         unsigned i;
         for (i = 0; i < numAnims; ++i)
         {
            Ogre::Animation* anim = mesh->getAnimation(i);
            Ogre::Animation::VertexTrackIterator it = anim->getVertexTrackIterator();
            bool isPose = false;
            
            while (it.hasMoreElements())
            {
               Ogre::VertexAnimationTrack* vat = it.getNext();
               if (vat->getAnimationType() == Ogre::VAT_POSE) isPose = true;
            }

            if (isPose)
            {
               MorphModifier newMorph;
               newMorph.InternalName = anim->getName();
               newMorph.Influence = 0.0f;
       
               Ogre::String humanReadableName = newMorph.InternalName;
               // Strip away "Morph_" from the beginning for nicer human-readable name
               if (humanReadableName.find("Morph_") == 0)
               {
                    humanReadableName = humanReadableName.substr(6, humanReadableName.length() - 6);
               } 

               // No need to apply it as it's 0 influence in the beginning, only store it  
               Avatar::setMorphModifier(humanReadableName, newMorph); 
            }         
         }
      } 
   }

   void OgreAvatar::applyTextureModifier(const TextureModifier& texture)
   {
       if (!Entity) return;

       if (texture.Influence >= texture.Threshold)
           applyTexture((TEXTURE)texture.SubmeshIndex, texture.TextureName);
   }

   void OgreAvatar::applyMorphModifier(const MorphModifier& morph)
   {          
       if (!Entity) return;

       _applyMorphModifier(morph, Entity);

       // Apply morph to attachments too
       size_t n;
       for (n=0 ; n<AttachmentList.size() ; ++n)
       {
          AttachmentBoneMap::iterator i = AttachmentList[n]->BoneMap.begin();
          for ( ; i != AttachmentList[n]->BoneMap.end() ; ++i)
          {
             if (i->second.Entity)
             {
                _applyMorphModifier(morph, i->second.Entity);
             }
          }
       }
   }

   void OgreAvatar::_applyMorphModifier(const MorphModifier& morph, Ogre::Entity *entity)
   {
      if (entity->getMesh()->hasAnimation(morph.InternalName) == false)
         return;

      try
      {
         Ogre::AnimationState* anim = entity->getAnimationState(morph.InternalName);

         float influence = morph.Influence;
         if (influence > 0.0f)
         {
            // Clamp to very near 1, but do not go actually at 1 or it goes back to 0
            if (influence > 0.99999f) influence = 0.99999f;

            anim->setEnabled(true);
            anim->setTimePosition(influence * anim->getLength());
         }
         else
         {
            anim->setEnabled(false);
         }
      } catch (Ogre::Exception e) {}
   }

   void OgreAvatar::applyMasterModifiers()
   {
      calculateMasterModifiers();

      unsigned n;
      for (n=0 ; n<AnimationModifiersVector.size() ; ++n)
      {
         std::string name = AnimationModifiersVector[n]->InternalName;
         DynamicAnimationPtr animation = DynamicAnimationManager::getSingleton().getByName(name);
         if (animation.isNull() == false)
         {
            animation->apply(AnimationModifiersVector[n]->Position);
         }
      }

      {
         MorphModifierMap::const_iterator i = MorphModifiers.begin();
         while (i != MorphModifiers.end())
         {
            applyMorphModifier(i->second);
            i++;
         }
      }

      {
         TextureModifierMap::const_iterator i = TextureModifiers.begin();
         while (i != TextureModifiers.end())
         {
            applyTextureModifier(i->second);
            i++;
         }
      }
   }

   void OgreAvatar::addDynamicAnimations()
   {
      size_t n;

      for (n=0 ; n<AnimationModifiersVector.size() ; ++n)
      {
         std::string name = AnimationModifiersVector[n]->InternalName;
         DynamicAnimationPtr animation = DynamicAnimationManager::getSingleton().getByName(name);
         if (animation.isNull() == false)
         {
            DynamicAnimationManager::getSingleton().addAnimationToEntity(Entity, name);
         
            animation->apply(AnimationModifiersVector[n]->Position);
         }
      }
   }

   void OgreAvatar::applyBoneConstraints()
   {
      if (!Entity) return;

      // If no constraints, don't go through skeleton
      if (!BoneConstraints.size()) return;

      Ogre::SkeletonInstance* skeleton = Entity->getSkeleton();
      if (!skeleton) return;

      // Apply constraints hierarchically, starting from root bone
      applyBoneConstraints(skeleton, skeleton->getRootBone());
   }

   void OgreAvatar::applyBoneConstraints(Ogre::SkeletonInstance* skeleton, Ogre::Bone* bone)
   {
      if (!bone) return;

      BoneConstraintMap::const_iterator i = BoneConstraints.find(bone->getName());
      if (i != BoneConstraints.end())
      {
         const BoneConstraintVector& v = i->second;

         bool boneChanged = false;

         // Find bone from the original source skeleton as well, so we can get the original initial state
         const Ogre::SkeletonPtr& origSkeleton = Entity->getMesh()->getSkeleton();
         if (origSkeleton.isNull()) return;
         Ogre::Bone* origBone = 0;
         try
         {  
            origBone = origSkeleton->getBone(bone->getName());
         } catch (...)
         {
            return;
         }

         unsigned j;
         for (j = 0; j < v.size(); ++j)
         {
            Ogre::Bone* targetBone = 0;
            try
            {
               targetBone = skeleton->getBone(v[j].Target);
            } 
            catch (...)
            {
               continue;
            }
            
            switch (v[j].Type)
            {
            case CONSTRAINT_POSITION:
               {
                  // Find out position of target bone in our frame
                  Ogre::Vector3 targetPos = targetBone->getWorldPosition();
                  if (bone->getParent())
                  {
                     const Ogre::Matrix4& parentDerived = bone->getParent()->_getFullTransform();
                     Ogre::Matrix4 parentInverse = parentDerived.inverseAffine();
                     targetPos = parentInverse * targetPos;
                  }
                  if (bone->getPosition() != targetPos)
                  {
                     bone->setPosition(targetPos);
                     bone->setInitialState();
                     boneChanged = true;
                  }
               }
               break;

            case CONSTRAINT_LOOKAT:
               {
                  // Find out position of target bone in our frame
                  Ogre::Vector3 targetPos = targetBone->getWorldPosition();
                  if (bone->getParent())
                  {
                     const Ogre::Matrix4& parentDerived = bone->getParent()->_getFullTransform();
                     Ogre::Matrix4 parentInverse = parentDerived.inverseAffine();
                     targetPos = parentInverse * targetPos;
                  }
                  Ogre::Vector3 dir = targetPos - bone->getPosition();
                  dir.normalise();
                  if (dir != Ogre::Vector3::ZERO)
                  {
                     // Adapted from Ogre::Camera.setDirection()
                     Ogre::Vector3 axes[3];
                     // Use the orientation from original skeleton so we have a stable initial state to work with
                     Ogre::Quaternion ori = origBone->getOrientation();
                     ori.ToAxes(axes);
                     Ogre::Quaternion rotQuat;
                     rotQuat = axes[0].getRotationTo(dir);
                     Ogre::Quaternion newOri = rotQuat * ori;
                     if (bone->getOrientation() != newOri)
                     {
                        bone->setOrientation(newOri);
                        bone->setInitialState();
                        boneChanged = true;
                     }
                  }
               }
               break;
            }
         }
        
         // Now update cached transform of this bone & all its children
         // This may be costly, so do it only when bone actually changed
         if (boneChanged)
         {
            bone->_update(true, false);
         }
      }

      unsigned numChildren = bone->numChildren();
      unsigned j;
      for (j = 0; j < numChildren; ++j)
      {
         applyBoneConstraints(skeleton, static_cast<Ogre::Bone*>(bone->getChild(j)));
      }
   }

   Ogre::Vector3 OgreAvatar::getBaseOffset()
   {
      Ogre::Vector3 offset = Ogre::Vector3::ZERO;

      const std::string& baseOffset = getProperty("baseoffset");
      if (!baseOffset.empty()) 
      {
          offset = Ogre::StringConverter::parseVector3(baseOffset);
      }

      const std::string& baseBoneName = getProperty("basebone");
      if (baseBoneName.empty()) 
          return offset;

      if (!Entity) 
         return offset;

      if (!Entity->hasSkeleton())
         return offset;

      Ogre::SkeletonInstance *skeleton = Entity->getSkeleton();
      try
      {
         Ogre::Node* bone = skeleton->getBone(baseBoneName);
         if (bone)
         {
            // Hacky and slow way to derive the initial position of the base bone. Do not use current position
            // because animations change it
            Ogre::Vector3 pos = bone->getInitialPosition();
            Ogre::Vector3 scale = bone->getInitialScale();
            Ogre::Quaternion orient = bone->getInitialOrientation();

            while (bone->getParent())
            {
               Ogre::Node* parent = bone->getParent();

               if (bone->getInheritOrientation())
               {
                  orient = parent->getInitialOrientation() * orient;
               }
               if (bone->getInheritScale())
               {
                  scale = parent->getInitialScale() * scale;
               }

               pos = parent->getInitialOrientation() * (parent->getInitialScale() * pos);
               pos += parent->getInitialPosition();

               bone = parent;
            }

            offset += pos;
         }
      }
      catch (Ogre::Exception) {}
    
      return offset;
   }
}
