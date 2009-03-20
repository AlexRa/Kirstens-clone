// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "DynamicAnimationManager.h"
#include "XmlManager.h"

   //const Ogre::String DynamicAnimationManager::DYNAMICANIMATION_RESOURCE_GROUP_NAME = "DynamicAnimation";
template<> Rex::DynamicAnimationManager *Ogre::Singleton<Rex::DynamicAnimationManager>::ms_Singleton = 0;
std::vector<Ogre::Animation*> Rex::AnimatedEntity::BaseAnimations;

namespace Rex
{
   DynamicAnimationManager *DynamicAnimationManager::getSingletonPtr()
   {
       return ms_Singleton;
   }

   DynamicAnimationManager &DynamicAnimationManager::getSingleton()
   {  
       assert(ms_Singleton);  
       return(*ms_Singleton);
   }

   DynamicAnimationManager::DynamicAnimationManager()
   {
      ScriptPattern.push_back("*.dad.xml");
      
      mResourceType = "DynamicAnimation";

      // low because we shouldn't load any resources
      mLoadOrder = 0.0f;

      // this is how we register the ResourceManager with OGRE
      Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

      // Low, as it will reference other resources
      // Xml files need to be loaded before these scripts
      LoadOrder = 20.0f;

      // Register this manager
      Ogre::ResourceGroupManager::getSingleton()._registerScriptLoader(this);
   }

   DynamicAnimationManager::~DynamicAnimationManager()
   {
      // unregister manager
      Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
      Ogre::ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);

      size_t n;
      for (n=0 ; n<AnimatedEntities.size() ; ++n)
         AnimatedEntities[n].BaseAnimations.clear();

      for (n=0 ; n<ClonedAnimations.size() ; ++n)
         SAFE_DELETE(ClonedAnimations[n]);
   }

   bool DynamicAnimationManager::addAnimationToEntity(Ogre::Entity *entity, const Ogre::String &name, bool replace)
   {
      assert(entity != NULL);
      assert(name.empty() == false);

      if (!entity || !entity->hasSkeleton())
         return false;

      DynamicAnimationPtr animation = static_cast<DynamicAnimationPtr>(ResourceManager::getByName(name));
      if (animation.isNull())
         return false;


      
      AnimatedEntity animEnt;
      AnimatedEntity *animEntPtr = &animEnt;
      animEnt.Entity = entity;
      animEnt.AnimationMap[name] = animation;
//      animEnt.Animations.push_back(animation);
      Ogre::SkeletonInstance *skeleton = entity->getSkeleton();


      // See if the entity already contains some dynamic animations
      std::vector<AnimatedEntity>::iterator foundAnim = std::find(AnimatedEntities.begin(), AnimatedEntities.end(), animEnt);
      if (foundAnim != AnimatedEntities.end())
      {
         animEntPtr = &*foundAnim;

         bool found = false;
         if (replace || animEntPtr->AnimationMap.find(name) == animEntPtr->AnimationMap.end())
         {
            animEntPtr->AnimationMap[name] = animation;
//            found = true;
//            break;
         }
//         size_t n;
//         for (n=0 ; n<animEntPtr->Animations.size() ; ++n)
//         {
//            if (animEntPtr->Animations[n]->getName().compare(name) == 0)
//            {
//               found = true;
//               break;
//            }
//         }

//         if (found == false)
//         {
//            animEntPtr->AnimationMap[name] = animation;
//            animEntPtr->Animations.push_back(animation);
//         }  
      }
//      return true;
      size_t n;
      for (n=0 ; n<animation->getBaseAnimations().size() ; ++n)
      {
         Ogre::Animation* baseAnimation;
         Ogre::String name = animation->getBaseAnimations()[n];
         Ogre::Animation *animation = 0;
         try
         {
            animation = skeleton->getAnimation(name);
         } catch (Ogre::Exception e) { continue; }

//         assert(animation);
         if (!animation)
            continue;

         //// See if this animation is already on entity
         //if (foundAnim != AnimatedEntities.end())
         //{
         //   size_t k;
         //   for (k=0 ; k<foundAnim->BaseAnimations.size() ; ++k)
         //   {
         //      if (name.compare(foundAnim->BaseAnimations[k]->getName()) == 0)
         //         continue; // Animation already on list, skip it
         //   }
         //}
         // Add animation
         bool found = false;
         size_t n;
         for (n=0 ; n<AnimatedEntity::BaseAnimations.size() ; ++n)
         {
            if (AnimatedEntity::BaseAnimations[n]->getName().compare(name) == 0)
            {
               found = true;
               break;
            }
         }
         if (!found)
         {
         baseAnimation = animation->clone(name);
         ClonedAnimations.push_back(baseAnimation);

         AnimatedEntity::BaseAnimations.push_back(baseAnimation);

         //animEntPtr->BaseAnimations.push_back(baseAnimation);
         }

      }
      if (foundAnim == AnimatedEntities.end())
         AnimatedEntities.push_back(*animEntPtr);
      

      return true;
   }

   bool DynamicAnimationManager::removeEntity(Ogre::Entity *entity)
   {
      assert(entity != 0);

      AnimatedEntity anim;
      anim.Entity = entity;

      std::vector<AnimatedEntity>::iterator found = std::find(AnimatedEntities.begin(), AnimatedEntities.end(), anim);
      if (found != AnimatedEntities.end())
      {
         AnimatedEntities.erase(found);

         return true;
      }

      return false;
   }


   DynamicAnimationPtr DynamicAnimationManager::load(const Ogre::String &name, const Ogre::String &group)
   {
      DynamicAnimationPtr anim = getByName(name);

       if (anim.isNull())
           anim = create(name, group);

       return anim;
   }

   //! Loads dynamic animation definitions from xml file.
   void DynamicAnimationManager::loadDynamicAnimationDefs(const Ogre::String &filename)
   {
   //   Ogre::LogManager::getSingleton().logMessage("Parsing dynamic animations...");
      XmlFilePtr xml;
      try
      {
         xml = XmlManager::getSingleton().load(filename, XmlManager::XMLDATA_RESOURCE_GROUP_NAME);
      } catch(...)
      {
         Ogre::LogManager::getSingleton().logMessage("Failed to parse file: " + filename + ".");
         return;
      }
      assert(xml.isNull() == false);
      if (xml.isNull())
      {
         Ogre::LogManager::getSingleton().logMessage("Failed to parse file: " + filename + ".");
         return;
      }

      XmlDocument *doc = xml->getDocument();
      assert(doc);
      XmlElement *root = doc->RootElement();
      if (root && root->ValueStr().compare("dynamic_animation_defs") == 0)
      {
         XmlElement *dynamicanimation_element = root->FirstChildElement();
         XmlElement *nextAnimationElement = dynamicanimation_element;
         while(nextAnimationElement)
         {
            dynamicanimation_element = nextAnimationElement;
            nextAnimationElement = nextAnimationElement->NextSiblingElement();

            const Ogre::String *name = dynamicanimation_element->Attribute(Ogre::String("name"));
            if (!name)
               continue;

            DynamicAnimationPtr animation = static_cast<DynamicAnimationPtr>(ResourceManager::create(*name, XmlManager::XMLDATA_RESOURCE_GROUP_NAME));
            if (animation->load(dynamicanimation_element) == false)
            {
               Ogre::LogManager::getSingleton().logMessage("Error encountered while loading: " + filename + ".");
               continue;
            }
         }
      }

      xml->unload();
   }

   Ogre::Real DynamicAnimationManager::projectVector(const Ogre::Real x1, const Ogre::Real x2, const Ogre::Real y1, const Ogre::Real y2)
   {
      Ogre::Real range1 = Ogre::Math::Abs(x1) + Ogre::Math::Abs(x2);
      Ogre::Real range2 = Ogre::Math::Abs(y1) + Ogre::Math::Abs(y2);
      if (Ogre::Math::RealEqual(range1, 0.0))
         return 0.0;

      return ((Ogre::Math::Abs(x1) * range2) / range1);
   }
   
   void DynamicAnimationManager::setAnimationPosition(const Ogre::String &name, float pos)
   {
      assert(name.empty() == false);
      assert(pos >= 0 && pos <= 1);

      if (pos < 0 || pos > 1)
      {
         Ogre::LogManager::getSingletonPtr()->logMessage("Dynamic animation offset out of range.");
         return;
      }

      size_t e;
      for (e=0 ; e<AnimatedEntities.size() ; ++e)
      {
         AnimatedEntities[e].update(name, pos);
      }
      return;
   }

   void DynamicAnimationManager::exportAnimations(XmlNode *root)
   {
      assert(root);
      DynamicAnimationManager::ResourceMapIterator iterator = DynamicAnimationManager::getSingleton().getResourceIterator();

      while (iterator.hasMoreElements())
      {
         DynamicAnimationPtr animation = static_cast<DynamicAnimationPtr>(iterator.peekNextValue());
         animation->exportTo(root);

         iterator.moveNext();
      }
   }

   const AnimatedEntity *DynamicAnimationManager::getAnimatedEntity(const Ogre::Entity *entity) const
   {
      AnimatedEntity toFind;
      toFind.Entity = const_cast<Ogre::Entity*>(entity);
      std::vector<AnimatedEntity>::const_iterator iter = std::find(AnimatedEntities.begin(), AnimatedEntities.end(), toFind);
      if (iter != AnimatedEntities.end())
      {
         return (&*iter);
      }

      return 0;
   }

   Ogre::Resource *DynamicAnimationManager::createImpl(const Ogre::String &name, Ogre::ResourceHandle handle, 
            const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader, 
            const Ogre::NameValuePairList *createParams)
   {
      return new DynamicAnimation(this, name, handle, group, isManual, loader);
   }


   const Ogre::StringVector &DynamicAnimationManager::getScriptPatterns() const
   {
      return ScriptPattern;
   }

   Ogre::Real DynamicAnimationManager::getLoadingOrder() const
   {
      return LoadOrder;
   }

   void DynamicAnimationManager::parseScript(Ogre::DataStreamPtr &stream, const Ogre::String &groupName)
   {
      if (groupName.compare(XmlManager::XMLDATA_RESOURCE_GROUP_NAME) == 0)
         loadDynamicAnimationDefs(stream->getName());
   }
} // namespace Rex
