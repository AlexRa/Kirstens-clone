/**
 * @file rexogrelegacymaterial.cpp
 * @brief RexOgreLegacyMaterial class implementation
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"

#include <OGRE/Ogre.h>

#include "rexogrelegacymaterial.h"
#include "llogre.h"
#include "llobjectmanager.h"

const std::string RexOgreLegacyMaterial::sDefaultMaterialName = "NoTexture";
U32 RexOgreLegacyMaterial::sUniqueMaterialId = 0;
int RexOgreLegacyMaterial::sTextureUpdateCount = 0;

RexOgreMaterialeffect RexOgreLegacyMaterial::sDefaultMaterialEffect;
RexOgreAdditiveBlending RexOgreLegacyMaterial::sAdditiveBlending;
RexOgreAdditiveBlendingNoDepth RexOgreLegacyMaterial::sAdditiveBlendingNoDepth;   

RexOgreLegacyMaterial::FixedMaterialMap RexOgreLegacyMaterial::sFixedMaterialEffects;

void RexOgreLegacyMaterial::initClass()
{
   sFixedMaterialEffects[FIXEDMATERIAL_DEFAULT] = &sDefaultMaterialEffect;
   sFixedMaterialEffects[FIXEDMATERIAL_ADDITIVEBLENDING] = &sAdditiveBlending;
   sFixedMaterialEffects[FIXEDMATERIAL_ADDITIVEBLENDING_NODEPTHCHECK] = &sAdditiveBlendingNoDepth;
   sFixedMaterialEffects[FIXEDMATERIAL_COUNT] = &sDefaultMaterialEffect;
}

RexOgreLegacyMaterial::RexOgreLegacyMaterial(const std::string &name) : mStatic(false)
{
   mName = name;

   // If material creation failed (renderer not yet initted), revert to name of default untextured material
   if (!createMaterial()) 
      mName = sDefaultMaterialName;
}

//! destructor
RexOgreLegacyMaterial::~RexOgreLegacyMaterial()
{
   destroyOgreMaterial();
}

bool RexOgreLegacyMaterial::createMaterial()
{
   LLOgreRenderer *renderer = LLOgreRenderer::getPointer();

 //  llassert(mMaterial.isNull());
   if (mMaterial.isNull() == false)
   {
      llassert(mName == mMaterial->getName());
      return true; // material already exists
   }

   // Ogre may not have been yet initialised when some textures get loaded (preloaded images)
   if (!renderer->isInitialized())
      return false;

   renderer->setOgreContext();

//   if (mMaterial.isNull() == false)
//   {
//      llwarns << "Ogre material " << mName << " already exists, deleting." << llendl;
//   }

   //! \Hack ugliness for the case when material with same name exists. Should not happen 
   //! nowadays that we delete the old OgreMaterials
   while (!Ogre::MaterialManager::getSingleton().getByName(mName).isNull())
   {
	   llwarns << "Material with name " + mName + " already exists" << llendl;
	   mName = mName + "1";
   }

   const std::string& baseMaterialName = renderer->getBaseMaterialName();
   Ogre::MaterialPtr baseMaterial;
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName);
    
   //  Base material
   if (baseMaterial.isNull())
   {
       Ogre::TrackVertexColourType tvc = Ogre::TVC_DIFFUSE|Ogre::TVC_AMBIENT;
       mMaterial = Ogre::MaterialManager::getSingleton().create(mName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
       // Setup defaults for all versions
       mMaterial->getTechnique(0)->getPass(0)->setTextureAnisotropy(4);
       mMaterial->getTechnique(0)->getPass(0)->setVertexColourTracking(tvc);
       // Create a default image so that we always have a texture unit, nice for the description textures
       mMaterial->getTechnique(0)->getPass(0)->createTextureUnitState("notexture.png");
       // Disable shadows everywhere else but on the terrain until better shadowing implemented / bugs resolved :)
       mMaterial->setReceiveShadows(false);
   }
   else
   {
       mMaterial = baseMaterial->clone(mName);
   }

   // Alpha material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "alpha");
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mAlphaMaterial = mMaterial->clone(mName + "alpha");
       mAlphaMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
       mAlphaMaterial->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
       if (mAlphaMaterial->getTechnique(0)->getPass(0)->getNumTextureUnitStates())
          mAlphaMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE);
   }
   else
   {
       mAlphaMaterial = baseMaterial->clone(mName + "alpha");
   }

   // Solid alpha material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "salpha");
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mSolidAlphaMaterial = mMaterial->clone(mName + "salpha");
   }
   else
   {
       mSolidAlphaMaterial = baseMaterial->clone(mName + "salpha");
   }

   // Fullbright material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "fb");
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mFullbrightMaterial = mMaterial->clone(mName + "fb");
       mFullbrightMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
   }
   else
   {
       mFullbrightMaterial = baseMaterial->clone(mName + "fb");
   }

   // Fullbright alpha material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "fbalpha");
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mFullbrightAlphaMaterial = mMaterial->clone(mName + "fbalpha");
       mFullbrightAlphaMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
       mFullbrightAlphaMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
       mFullbrightAlphaMaterial->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
       if (mFullbrightAlphaMaterial->getTechnique(0)->getPass(0)->getNumTextureUnitStates())
           mFullbrightAlphaMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE);
   }
   else
   {
       mFullbrightAlphaMaterial = baseMaterial->clone(mName + "fbalpha");
   }

   // Fullbright solid alpha material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "fbsalpha");
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mFullbrightSolidAlphaMaterial = mMaterial->clone(mName + "fbsalpha");
       mFullbrightSolidAlphaMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
   }
   else
   {
       mFullbrightSolidAlphaMaterial = baseMaterial->clone(mName + "fbsalpha");
   }

   // Additive blended material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + sAdditiveBlending.getSuffix());
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mAdditiveMaterial = mMaterial->clone(mName + sAdditiveBlending.getSuffix());
       sAdditiveBlending.enable(mAdditiveMaterial);
   }
   else
   {
       mAdditiveMaterial = baseMaterial->clone(mName + sAdditiveBlending.getSuffix());
   }

   // Additive blended nodepth material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + sAdditiveBlendingNoDepth.getSuffix());
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mAdditiveNoDepthMaterial = mMaterial->clone(mName + sAdditiveBlendingNoDepth.getSuffix());
       sAdditiveBlendingNoDepth.enable(mAdditiveNoDepthMaterial);
   }
   else
   {
       mAdditiveNoDepthMaterial = baseMaterial->clone(mName + sAdditiveBlendingNoDepth.getSuffix());
   }

   // Fullbright additive blended material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "fb" + sAdditiveBlending.getSuffix());
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mFullbrightAdditiveMaterial = mMaterial->clone(mName + "fb" + sAdditiveBlending.getSuffix());
       mFullbrightAdditiveMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
       sAdditiveBlending.enable(mFullbrightAdditiveMaterial);
   }
   else
   {
       mFullbrightAdditiveMaterial = baseMaterial->clone(mName + "fb" + sAdditiveBlending.getSuffix());
   }

   // Fullbright additive blended nodepth material
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "fb" + sAdditiveBlendingNoDepth.getSuffix());
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mFullbrightAdditiveNoDepthMaterial = mMaterial->clone(mName + "fb" +sAdditiveBlendingNoDepth.getSuffix());
       mFullbrightAdditiveNoDepthMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
       sAdditiveBlendingNoDepth.enable(mFullbrightAdditiveNoDepthMaterial);
   }
   else
   {
       mFullbrightAdditiveNoDepthMaterial = baseMaterial->clone(mName + "fb" + sAdditiveBlendingNoDepth.getSuffix());
   }

   // Twosided
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "ts");
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mMaterialTwoSided = mMaterial->clone(mName + "ts");
       mMaterialTwoSided->setCullingMode(Ogre::CULL_NONE);
   }
   else
   {
       mMaterialTwoSided = baseMaterial->clone(mName + "ts");
   }

   // Twosided fullbright
   if (!baseMaterialName.empty())
       baseMaterial = Ogre::MaterialManager::getSingleton().getByName(baseMaterialName + "fbts");
   else
       baseMaterial.setNull();
   if (baseMaterial.isNull())
   {
       mMaterialTwoSidedFullbright = mMaterial->clone(mName + "fbts");
       mMaterialTwoSidedFullbright->setCullingMode(Ogre::CULL_NONE);
       mMaterialTwoSidedFullbright->getTechnique(0)->getPass(0)->setLightingEnabled(false);
   }
   else
   {
       mMaterialTwoSidedFullbright = baseMaterial->clone(mName + "fbts");
   }

   // Enable shadowing if someone necessarily wants it 
   updateShadowReceipt(false);

   renderer->setSLContext();
   return true;
}

//! Update shadowing on material variations, based on whether or not it is wanted and whether texture has alpha
void RexOgreLegacyMaterial::updateShadowReceipt(bool hasAlpha)
{
    if (mMaterial.isNull()) return;

    LLOgreRenderer *renderer = LLOgreRenderer::getPointer();

    // Hack: if basematerialname set, let it control shadow receipt totally
    if (!renderer->getBaseMaterialName().empty())
        return;

    bool useShadows = renderer->getUseObjectShadows();

    // Disable shadows always on alphablended textures
    if (hasAlpha) useShadows = false;

    mMaterial->setReceiveShadows(useShadows);
    mFullbrightMaterial->setReceiveShadows(useShadows); 
}

//! Returns material name

const std::string &RexOgreLegacyMaterial::getName() const
{
	return mName;
}

bool RexOgreLegacyMaterial::createdSuccesfully() const
{
   return !mMaterial.isNull();
}

Ogre::TextureUnitState* RexOgreLegacyMaterial::setFirstTextureUnit(Ogre::MaterialPtr material, const Ogre::String& texName)
{
    Ogre::Pass* passPtr = material->getTechnique(0)->getPass(0);
    Ogre::TextureUnitState* tu;

    if (!passPtr->getNumTextureUnitStates())
        tu = passPtr->createTextureUnitState(texName);
    else
    {
        tu = passPtr->getTextureUnitState(0);
        tu->setTextureName(texName);
    }

    return tu;
}

//! Set texture from main variation to others
void RexOgreLegacyMaterial::updateVariationsTexture()
{
   if (mMaterial.isNull()) return;

   Ogre::TextureUnitState* tu = mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0);
   if (!tu) return;

   const Ogre::String& texName = tu->getTextureName();

   Ogre::Real uScale = tu->getTextureUScale();
   Ogre::Real vScale = tu->getTextureVScale();

   setFirstTextureUnit(mAlphaMaterial, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mSolidAlphaMaterial, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mFullbrightMaterial, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mFullbrightAlphaMaterial, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mFullbrightSolidAlphaMaterial, texName)->setTextureScale(uScale, vScale);
   
   setFirstTextureUnit(mAdditiveMaterial, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mAdditiveNoDepthMaterial, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mFullbrightAdditiveMaterial, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mFullbrightAdditiveNoDepthMaterial, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mMaterialTwoSided, texName)->setTextureScale(uScale, vScale);
   setFirstTextureUnit(mMaterialTwoSidedFullbright, texName)->setTextureScale(uScale, vScale);

   Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(texName);
   if (!texture.isNull())
       updateShadowReceipt(texture->hasAlpha());
}

//! Set texture
void RexOgreLegacyMaterial::setTexture(const Ogre::TexturePtr texture)
{
   if (texture.isNull())
       return;

   const Ogre::String& texName = texture->getName();

   setFirstTextureUnit(mMaterial, texName);
   setFirstTextureUnit(mAlphaMaterial, texName);
   setFirstTextureUnit(mSolidAlphaMaterial, texName);
   setFirstTextureUnit(mFullbrightMaterial, texName);
   setFirstTextureUnit(mFullbrightAlphaMaterial, texName);
   setFirstTextureUnit(mFullbrightSolidAlphaMaterial, texName);

   setFirstTextureUnit(mAdditiveMaterial, texName);
   setFirstTextureUnit(mAdditiveNoDepthMaterial, texName);
   setFirstTextureUnit(mFullbrightAdditiveMaterial, texName);
   setFirstTextureUnit(mFullbrightAdditiveNoDepthMaterial, texName);
   setFirstTextureUnit(mMaterialTwoSided, texName);
   setFirstTextureUnit(mMaterialTwoSidedFullbright, texName);

   // Set new texture for clones
   std::list<std::string>::iterator i = mCloneList.begin();
   while (i != mCloneList.end())
   {
	   Ogre::MaterialPtr clone = Ogre::MaterialManager::getSingleton().getByName((*i));
	   if (clone.isNull() == false)
	   {
		   Ogre::Pass *pass = clone->getTechnique(0)->getPass(0);
		   if (pass->getNumTextureUnitStates())
		   {
			   pass->getTextureUnitState(0)->setTextureName(texName);
		   }
		   ++i;
	   }
	   else
	   {
		   // Clone has been destroyed
		   i = mCloneList.erase(i);
	   }
   }

   // Update texture blending mode based on whether it has alpha
   if (texture->hasAlpha())
   {
      mMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
      mMaterial->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
      mSolidAlphaMaterial->getTechnique(0)->getPass(0)->setAlphaRejectSettings(Ogre::CMPF_GREATER_EQUAL, 128);
 
      mFullbrightMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
      mFullbrightMaterial->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
      mFullbrightSolidAlphaMaterial->getTechnique(0)->getPass(0)->setAlphaRejectSettings(Ogre::CMPF_GREATER_EQUAL, 128);

      mMaterialTwoSided->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
      mMaterialTwoSided->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);

      mMaterialTwoSidedFullbright->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
      mMaterialTwoSidedFullbright->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);

   }
   else
   {
      mMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_REPLACE);
      mMaterial->getTechnique(0)->getPass(0)->setDepthWriteEnabled(true);
      mSolidAlphaMaterial->getTechnique(0)->getPass(0)->setAlphaRejectFunction(Ogre::CMPF_ALWAYS_PASS);

      mFullbrightMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_REPLACE);
      mFullbrightMaterial->getTechnique(0)->getPass(0)->setDepthWriteEnabled(true);
      mFullbrightSolidAlphaMaterial->getTechnique(0)->getPass(0)->setAlphaRejectFunction(Ogre::CMPF_ALWAYS_PASS);

      mMaterialTwoSided->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_REPLACE);
      mMaterialTwoSided->getTechnique(0)->getPass(0)->setDepthWriteEnabled(true);

      mMaterialTwoSidedFullbright->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_REPLACE);
      mMaterialTwoSidedFullbright->getTechnique(0)->getPass(0)->setDepthWriteEnabled(true);
   }

   // Re-enable effects

   // Setup effects in the additive blending versions
   sAdditiveBlending.enable(mAdditiveMaterial);
   sAdditiveBlending.enable(mFullbrightAdditiveMaterial);
   sAdditiveBlendingNoDepth.enable(mAdditiveNoDepthMaterial);
   sAdditiveBlendingNoDepth.enable(mFullbrightAdditiveNoDepthMaterial);

   // Set two-sided
   mMaterialTwoSided->setCullingMode(Ogre::CULL_NONE);
   mMaterialTwoSidedFullbright->setCullingMode(Ogre::CULL_NONE);

   updateShadowReceipt(texture->hasAlpha());

   LLObjectManager* objMgr = LLObjectManager::getInstance();
   if (objMgr)
       objMgr->materialModified(mMaterial.get());

   ++sTextureUpdateCount;
   return;
}

//! Destroy ogre material
bool RexOgreLegacyMaterial::destroyOgreMaterial()
{
   LLOgreRenderer *renderer = LLOgreRenderer::getPointer();
   if (!renderer->isInitialized())
      return false;

   if (mMaterial.isNull() == true)
      return true;

   renderer->setOgreContext();

   Ogre::ResourcePtr castPtr;

   castPtr = static_cast<Ogre::ResourcePtr>(mMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mAlphaMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mSolidAlphaMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mFullbrightMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mFullbrightAlphaMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mFullbrightSolidAlphaMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mAdditiveMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mAdditiveNoDepthMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mFullbrightAdditiveMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mFullbrightAdditiveNoDepthMaterial);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mMaterialTwoSided);
   Ogre::MaterialManager::getSingleton().remove(castPtr);

   castPtr = static_cast<Ogre::ResourcePtr>(mMaterialTwoSidedFullbright);
   Ogre::MaterialManager::getSingleton().remove(castPtr);
   
   renderer->setSLContext();

   return true;
}

std::string RexOgreLegacyMaterial::generateUniqueMaterialName()
{
   if (sUniqueMaterialId > 4000000)
      sUniqueMaterialId = 0;

   S32 n;
   for (n=0 ; n<1000000 ; ++n)
   {
       std::string name("REXMaterial_");
      std::stringstream ss;
      ss << sUniqueMaterialId;
      ss >> name;

      sUniqueMaterialId++;

      if (Ogre::MaterialManager::getSingleton().resourceExists(name) == false)
         return name;
   }

   return std::string("");
}

//! Return texture build count and zero it for next frame
int RexOgreLegacyMaterial::getTextureUpdateCount()
{
	int count = sTextureUpdateCount;
	sTextureUpdateCount = 0;
	return count;
}

//! Register a clone of this material for texture updates
void RexOgreLegacyMaterial::addClone(const std::string& name)
{
	mCloneList.push_back(name);
}

void RexOgreLegacyMaterial::setStatic(bool isStatic)
{
   mStatic = isStatic;
}

//! Returns true is material is static, false otherwise
bool RexOgreLegacyMaterial::getStatic() const
{
   return mStatic;
}

const std::string& RexOgreLegacyMaterial::getFixedMaterialSuffix(FixedOgreMaterial material)
{
    return sFixedMaterialEffects[material]->getSuffix();
}

//void RexOgreLegacyMaterial::setFixedMaterialType(FixedOgreMaterial material)
//{
//   if (mName == RexOgreLegacyMaterial::sDefaultMaterialName)
//       return;
//
//   // Disable all material effects
//   FixedMaterialMap::iterator iter = mFixedMaterialEffects.begin();
//   for ( ; iter != mFixedMaterialEffects.end() ; ++iter)
//   {
//      iter->second->disable(mMaterial);
//      iter->second->disable(mAlphaMaterial);
//      iter->second->disable(mSolidAlphaMaterial);
//      iter->second->disable(mFullbrightMaterial);
//      iter->second->disable(mFullbrightAlphaMaterial);
//      iter->second->disable(mFullbrightSolidAlphaMaterial);
//   }
//   mFixedMaterialEffects[material]->enable(mMaterial);
//   mFixedMaterialEffects[material]->enable(mAlphaMaterial);
//   mFixedMaterialEffects[material]->enable(mSolidAlphaMaterial);
//   mFixedMaterialEffects[material]->enable(mFullbrightMaterial);
//   mFixedMaterialEffects[material]->enable(mFullbrightAlphaMaterial);
//   mFixedMaterialEffects[material]->enable(mFullbrightSolidAlphaMaterial);
//}


