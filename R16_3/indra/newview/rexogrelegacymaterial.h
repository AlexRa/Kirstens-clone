/** 
 * @file rexogrelegacymaterial.h
 * @brief RexOgreLegacyMaterial class header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#ifndef REXOGRELEGACYMATERIAL_H					
#define REXOGRELEGACYMATERIAL_H

#include "llimage.h"
#include "llprimitive.h"
#include "rexogrematerialeffect.h"

#include <OGRE/Ogre.h>
#include <list>
#include <map>

//! Contains functions for making Ogre materials out of SLViewer textures/images
/*! Doesn't handle compressed textures (SLViewer does support them, but doesn't seem to use them yet),
   Also doesn't handle textures that contain mipmaps, but sl doesn't seem to support those either yet
    the resulting Ogre texture may be upside down or mirrored or something.
    Only does RGB and RGBA textures for now.
    Occasionally there is an RGBA texture from SL that seems to have RGB size??
    Sometimes old material has same name as old material, bug or weirdness with random number generator?
    Need a better way to name textures and materials. 

    \todo Need to test the Ogre textures are rightside up...
*/
class RexOgreLegacyMaterial
{
public:
   //! constructor
   /*!
       \param name Name of the material, should be UUID of the texture it's based on
   */
    RexOgreLegacyMaterial(const std::string &name);

   //! destructor
   ~RexOgreLegacyMaterial();

   //! Returns the name of this material
   const std::string &getName() const;

   //! Returns true if material was created succesfully, false otherwise
   bool createdSuccesfully() const;

   //! Set new texture for this material, its variations & clones
   void setTexture(const Ogre::TexturePtr texture);

   //! Returns Ogre material
   Ogre::MaterialPtr &getMaterial() { return mMaterial; } 

   //! Register a clone of this material, to be updated automatically when the texture of parent changes.
   /*! Clones are checked for null reference: those are automatically erased from clone list, so manual
    *  removal is not required
	*/
   void addClone(const std::string& name);

   //! Set material static, so it will always use the same texture
   void setStatic(bool isStatic);

   //! Sets texture from main version of the material to the other variations (alpha, fullbright etc.)
   void updateVariationsTexture();

   //! Sets texture on first texture unit. Creates it if it didn't exist. Returns texture unit pointer.
   Ogre::TextureUnitState* setFirstTextureUnit(Ogre::MaterialPtr material, const Ogre::String& texName);

   //! Returns true is material is static, false otherwise
   bool getStatic() const;

   //! Init class (fixed effect map)
   static void initClass();

   //! Generates unique material name. Returns empty string if unique name could not be generated.
   static std::string generateUniqueMaterialName();

   //! Returns amount of textures created this frame, clears statistics for next frame
   static int getTextureUpdateCount();

   //! Get suffix of fixed material variation (f.ex. additive blending...)
   static const std::string& getFixedMaterialSuffix(FixedOgreMaterial material);

   //! Set fixed type for the material (f.ex. additive blending...)
   // void setFixedMaterialType(FixedOgreMaterial material);

   static const std::string sDefaultMaterialName;

   typedef std::map<FixedOgreMaterial, RexOgreMaterialeffect*> FixedMaterialMap;

protected:
   //! Create fixed Ogre material effects
   void createFixedEffects();

   //! Update shadowing on material variations, based on whether or not it is wanted and whether texture has alpha
   void updateShadowReceipt(bool hasAlpha);

   //! Creates a new material
   /*!
      \return true if created succesfully, false otherwise
   */
   bool createMaterial();

   //! Destroy ogre material
   bool destroyOgreMaterial();

   //! Ogre material
   Ogre::MaterialPtr mMaterial;
   //! Alpha variation of the material
   Ogre::MaterialPtr mAlphaMaterial;
   //! Solid alpha variation of material
   Ogre::MaterialPtr mSolidAlphaMaterial;
   //! Fullbright variation of the material
   Ogre::MaterialPtr mFullbrightMaterial;
   //! Fullbright alpha variation of the material
   Ogre::MaterialPtr mFullbrightAlphaMaterial;
   //! Fullbright solid alpha variation of the material
   Ogre::MaterialPtr mFullbrightSolidAlphaMaterial;
   //! Additive blended variation of material
   Ogre::MaterialPtr mAdditiveMaterial;
   //! Additive blended depthcheck-off variation of material
   Ogre::MaterialPtr mAdditiveNoDepthMaterial;
   //! Fullbright additive blended variation of material
   Ogre::MaterialPtr mFullbrightAdditiveMaterial;
   //! Fullbright additive blended depthcheck-off variation of material
   Ogre::MaterialPtr mFullbrightAdditiveNoDepthMaterial;
   //! Two sided
   Ogre::MaterialPtr mMaterialTwoSided;
   //! Two sided fb
   Ogre::MaterialPtr mMaterialTwoSidedFullbright;

   //! Material name
   std::string mName;

   //! List of clones
   std::list<std::string> mCloneList;

   //! Static material, no (texture) updates should be done
   bool mStatic;

   //! Continuous id for unique material name generator
   static U32 sUniqueMaterialId;

   //! Texture update count
   static int sTextureUpdateCount;

   //! Default effect
   static RexOgreMaterialeffect sDefaultMaterialEffect;
   static RexOgreAdditiveBlending sAdditiveBlending;
   static RexOgreAdditiveBlendingNoDepth sAdditiveBlendingNoDepth;

   //! Map of ogre material effects
   static FixedMaterialMap sFixedMaterialEffects;
};

#endif // REXOGRELEGACYMATERIAL_H
