/**
 * @file RexOgreMaterialeffect.cpp
 * @brief RexOgreMaterialeffect class implementation
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"

#include "rexogrematerialeffect.h"

void RexOgreAdditiveBlending::enable(Ogre::MaterialPtr material)
{
   //if (isEnabled(material))
   //   return;

   //mBlendType[material->getName()] = Ogre::SBT_REPLACE;
   //if (material->isTransparent())
   //   mBlendType[material->getName()] = Ogre::SBT_TRANSPARENT_ALPHA;

   material->setSceneBlending(Ogre::SBT_ADD);
   material->setDepthWriteEnabled(false);

   Ogre::SceneManager *scm = Ogre::Root::getSingleton().getSceneManager("SceneManager");   
   material->getTechnique(0)->getPass(0)->setFog(true, scm->getFogMode(), Ogre::ColourValue::Black,
      scm->getFogDensity(), scm->getFogStart(), scm->getFogEnd()); // We need to override fog on additive blended objects, or else they look funky

   //RexOgreMaterialeffect::enable(material);
}

void RexOgreAdditiveBlending::disable(Ogre::MaterialPtr material)
{
   if (!isEnabled(material))
      return;

   const std::string &materialName = material->getName();
   if (mBlendType.find(materialName) != mBlendType.end())
   {
      material->setSceneBlending(mBlendType[materialName]);

      if (mBlendType[materialName] != Ogre::SBT_TRANSPARENT_ALPHA)
      {
         material->setDepthWriteEnabled(true);
      }
   }

   material->getTechnique(0)->getPass(0)->setFog(false);

   RexOgreMaterialeffect::disable(material);
}


///////////////////////////////////////////////
//
//        Additive blending no depth check
//

void RexOgreAdditiveBlendingNoDepth::enable(Ogre::MaterialPtr material)
{
   //if (isEnabled(material))
   //   return;

   //mBlendType[material->getName()] = Ogre::SBT_REPLACE;
   //if (material->isTransparent())
   //   mBlendType[material->getName()] = Ogre::SBT_TRANSPARENT_ALPHA;

   material->setSceneBlending(Ogre::SBT_ADD);
   material->setDepthCheckEnabled(false);

   Ogre::SceneManager *scm = Ogre::Root::getSingleton().getSceneManager("SceneManager");
   material->getTechnique(0)->getPass(0)->setFog(true, scm->getFogMode(), Ogre::ColourValue::Black,
      scm->getFogDensity(), scm->getFogStart(), scm->getFogEnd()); // We need to override fog on additive blended objects, or else they look funky

   //RexOgreMaterialeffect::enable(material);
}

void RexOgreAdditiveBlendingNoDepth::disable(Ogre::MaterialPtr material)
{
   if (!isEnabled(material))
      return;

   const std::string &materialName = material->getName();
   if (mBlendType.find(materialName) != mBlendType.end())
   {
      material->setSceneBlending(mBlendType[materialName]);
   }
//   material->setSceneBlending(mBlendType);
   material->setDepthCheckEnabled(true);

   material->getTechnique(0)->getPass(0)->setFog(false);

   RexOgreMaterialeffect::disable(material);
}

