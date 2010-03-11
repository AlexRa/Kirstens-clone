/** 
 * @file llvoavatar.cpp
 * @brief Implementation of LLVOAvatar class which is a derivation fo LLViewerObject
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2010, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"
#include "llvoavatardefines.h"
#include "llviewercontrol.h" // gSavedSettings

const S32 LLVOAvatarDefines::SCRATCH_TEX_WIDTH = 512;
const S32 LLVOAvatarDefines::SCRATCH_TEX_HEIGHT = 512;
const S32 LLVOAvatarDefines::IMPOSTOR_PERIOD = 2;

using namespace LLVOAvatarDefines;

/*********************************************************************************
 * Edit this function to add/remove/change textures and mesh definitions for avatars.
 */

LLVOAvatarDictionary::Textures::Textures()
{
	addEntry(TEX_HEAD_BODYPAINT,              new TextureEntry("head_bodypaint",   TRUE,  BAKED_NUM_INDICES, "",                          WT_SKIN));
	addEntry(TEX_UPPER_SHIRT,                 new TextureEntry("upper_shirt",      TRUE,  BAKED_NUM_INDICES, "UIImgDefaultShirtUUID",     WT_SHIRT));
	addEntry(TEX_LOWER_PANTS,                 new TextureEntry("lower_pants",      TRUE,  BAKED_NUM_INDICES, "UIImgDefaultPantsUUID",     WT_PANTS));
	addEntry(TEX_EYES_IRIS,                   new TextureEntry("eyes_iris",        TRUE,  BAKED_NUM_INDICES, "UIImgDefaultEyesUUID",      WT_EYES));
	addEntry(TEX_HAIR,                        new TextureEntry("hair_grain",       TRUE,  BAKED_NUM_INDICES, "UIImgDefaultHairUUID",      WT_HAIR));
	addEntry(TEX_UPPER_BODYPAINT,             new TextureEntry("upper_bodypaint",  TRUE,  BAKED_NUM_INDICES, "",                          WT_SKIN));
	addEntry(TEX_LOWER_BODYPAINT,             new TextureEntry("lower_bodypaint",  TRUE,  BAKED_NUM_INDICES, "",                          WT_SKIN));
	addEntry(TEX_LOWER_SHOES,                 new TextureEntry("lower_shoes",      TRUE,  BAKED_NUM_INDICES, "UIImgDefaultShoesUUID",     WT_SHOES));
	addEntry(TEX_LOWER_SOCKS,                 new TextureEntry("lower_socks",      TRUE,  BAKED_NUM_INDICES, "UIImgDefaultSocksUUID",     WT_SOCKS));
	addEntry(TEX_UPPER_JACKET,                new TextureEntry("upper_jacket",     TRUE,  BAKED_NUM_INDICES, "UIImgDefaultJacketUUID",    WT_JACKET));
	addEntry(TEX_LOWER_JACKET,                new TextureEntry("lower_jacket",     TRUE,  BAKED_NUM_INDICES, "UIImgDefaultJacketUUID",    WT_JACKET));
	addEntry(TEX_UPPER_GLOVES,                new TextureEntry("upper_gloves",     TRUE,  BAKED_NUM_INDICES, "UIImgDefaultGlovesUUID",    WT_GLOVES));
	addEntry(TEX_UPPER_UNDERSHIRT,            new TextureEntry("upper_undershirt", TRUE,  BAKED_NUM_INDICES, "UIImgDefaultUnderwearUUID", WT_UNDERSHIRT));
	addEntry(TEX_LOWER_UNDERPANTS,            new TextureEntry("lower_underpants", TRUE,  BAKED_NUM_INDICES, "UIImgDefaultUnderwearUUID", WT_UNDERPANTS));
	addEntry(TEX_SKIRT,                       new TextureEntry("skirt",            TRUE,  BAKED_NUM_INDICES, "UIImgDefaultSkirtUUID",     WT_SKIRT));

	addEntry(TEX_LOWER_ALPHA,                 new TextureEntry("lower_alpha",      TRUE,  BAKED_NUM_INDICES, "UIImgDefaultAlphaUUID",     WT_ALPHA));
	addEntry(TEX_UPPER_ALPHA,                 new TextureEntry("upper_alpha",      TRUE,  BAKED_NUM_INDICES, "UIImgDefaultAlphaUUID",     WT_ALPHA));
	addEntry(TEX_HEAD_ALPHA,                  new TextureEntry("head_alpha",       TRUE,  BAKED_NUM_INDICES, "UIImgDefaultAlphaUUID",     WT_ALPHA));
	addEntry(TEX_EYES_ALPHA,                  new TextureEntry("eyes_alpha",       TRUE,  BAKED_NUM_INDICES, "UIImgDefaultAlphaUUID",     WT_ALPHA));
	addEntry(TEX_HAIR_ALPHA,                  new TextureEntry("hair_alpha",       TRUE,  BAKED_NUM_INDICES, "UIImgDefaultAlphaUUID",     WT_ALPHA));

	addEntry(TEX_HEAD_TATTOO,                 new TextureEntry("head_tattoo",      TRUE,  BAKED_NUM_INDICES, "",     WT_TATTOO));
	addEntry(TEX_UPPER_TATTOO,                new TextureEntry("upper_tattoo",     TRUE,  BAKED_NUM_INDICES, "",     WT_TATTOO));
	addEntry(TEX_LOWER_TATTOO,                new TextureEntry("lower_tattoo",     TRUE,  BAKED_NUM_INDICES, "",     WT_TATTOO));

	addEntry(TEX_HEAD_BAKED,                  new TextureEntry("head-baked",       FALSE, BAKED_HEAD));
	addEntry(TEX_UPPER_BAKED,                 new TextureEntry("upper-baked",      FALSE, BAKED_UPPER));
	addEntry(TEX_LOWER_BAKED,                 new TextureEntry("lower-baked",      FALSE, BAKED_LOWER));
	addEntry(TEX_EYES_BAKED,                  new TextureEntry("eyes-baked",       FALSE, BAKED_EYES));
	addEntry(TEX_HAIR_BAKED,                  new TextureEntry("hair-baked",       FALSE, BAKED_HAIR));
	addEntry(TEX_SKIRT_BAKED,                 new TextureEntry("skirt-baked",      FALSE, BAKED_SKIRT));
}

LLVOAvatarDictionary::BakedTextures::BakedTextures()
{
	// Baked textures
	addEntry(BAKED_HEAD,       new BakedEntry(TEX_HEAD_BAKED,  
											  "head", "a4b9dc38-e13b-4df9-b284-751efb0566ff", 
											  3, TEX_HEAD_BODYPAINT, TEX_HEAD_TATTOO, TEX_HEAD_ALPHA,
											  5, WT_SHAPE, WT_SKIN, WT_HAIR, WT_TATTOO, WT_ALPHA));

	addEntry(BAKED_UPPER,      new BakedEntry(TEX_UPPER_BAKED, 
											  "upper_body", "5943ff64-d26c-4a90-a8c0-d61f56bd98d4", 
											  7, TEX_UPPER_SHIRT,TEX_UPPER_BODYPAINT, TEX_UPPER_JACKET,
											  TEX_UPPER_GLOVES, TEX_UPPER_UNDERSHIRT, TEX_UPPER_TATTOO, TEX_UPPER_ALPHA,
											  8, WT_SHAPE, WT_SKIN,	WT_SHIRT, WT_JACKET, WT_GLOVES, WT_UNDERSHIRT, WT_TATTOO, WT_ALPHA));											  

	addEntry(BAKED_LOWER,      new BakedEntry(TEX_LOWER_BAKED, 
											  "lower_body", "2944ee70-90a7-425d-a5fb-d749c782ed7d",
											  8, TEX_LOWER_PANTS,TEX_LOWER_BODYPAINT,TEX_LOWER_SHOES, TEX_LOWER_SOCKS,
											  TEX_LOWER_JACKET, TEX_LOWER_UNDERPANTS, TEX_LOWER_TATTOO, TEX_LOWER_ALPHA,
											  9, WT_SHAPE, WT_SKIN,	WT_PANTS, WT_SHOES,	 WT_SOCKS,  WT_JACKET, WT_UNDERPANTS, WT_TATTOO, WT_ALPHA));

	addEntry(BAKED_EYES,       new BakedEntry(TEX_EYES_BAKED,  
											  "eyes", "27b1bc0f-979f-4b13-95fe-b981c2ba9788",
											  2, TEX_EYES_IRIS, TEX_EYES_ALPHA,
											  2, WT_EYES, WT_ALPHA));

	addEntry(BAKED_SKIRT,      new BakedEntry(TEX_SKIRT_BAKED,
											  "skirt", "03e7e8cb-1368-483b-b6f3-74850838ba63", 
											  1, TEX_SKIRT,
											  1, WT_SKIRT));

	addEntry(BAKED_HAIR,       new BakedEntry(TEX_HAIR_BAKED,
											  "hair", "a60e85a9-74e8-48d8-8a2d-8129f28d9b61", 
											  2, TEX_HAIR, TEX_HAIR_ALPHA,
											  2, WT_HAIR, WT_ALPHA));
}

LLVOAvatarDictionary::Meshes::Meshes()
{
	// Meshes
	addEntry(MESH_ID_HAIR,             new MeshEntry(BAKED_HAIR,  "hairMesh",         6, LLViewerJoint::PN_4));
	addEntry(MESH_ID_HEAD,             new MeshEntry(BAKED_HEAD,  "headMesh",         5, LLViewerJoint::PN_5));
	addEntry(MESH_ID_EYELASH,          new MeshEntry(BAKED_HEAD,  "eyelashMesh",      1, LLViewerJoint::PN_0)); // no baked mesh associated currently
	addEntry(MESH_ID_UPPER_BODY,       new MeshEntry(BAKED_UPPER, "upperBodyMesh",    5, LLViewerJoint::PN_1));
	addEntry(MESH_ID_LOWER_BODY,       new MeshEntry(BAKED_LOWER, "lowerBodyMesh",    5, LLViewerJoint::PN_2));
	addEntry(MESH_ID_EYEBALL_LEFT,     new MeshEntry(BAKED_EYES,  "eyeBallLeftMesh",  2, LLViewerJoint::PN_3));
	addEntry(MESH_ID_EYEBALL_RIGHT,    new MeshEntry(BAKED_EYES,  "eyeBallRightMesh", 2, LLViewerJoint::PN_3));
	addEntry(MESH_ID_SKIRT,            new MeshEntry(BAKED_SKIRT, "skirtMesh",        5, LLViewerJoint::PN_5));
}

/*
 *
 *********************************************************************************/

LLVOAvatarDictionary::LLVOAvatarDictionary()
{
	createAssociations();
}

//virtual 
LLVOAvatarDictionary::~LLVOAvatarDictionary()
{
}

// Baked textures are composites of textures; for each such composited texture,
// map it to the baked texture.
void LLVOAvatarDictionary::createAssociations()
{
	for (BakedTextures::const_iterator iter = mBakedTextures.begin(); iter != mBakedTextures.end(); iter++)
	{
		const EBakedTextureIndex baked_index = (iter->first);
		const BakedEntry *dict = (iter->second);

		// For each texture that this baked texture index affects, associate those textures
		// with this baked texture index.
		for (texture_vec_t::const_iterator local_texture_iter = dict->mLocalTextures.begin();
			 local_texture_iter != dict->mLocalTextures.end();
			 local_texture_iter++)
		{
			const ETextureIndex local_texture_index = (ETextureIndex) *local_texture_iter;
			mTextures[local_texture_index]->mIsUsedByBakedTexture = true;
			mTextures[local_texture_index]->mBakedTextureIndex = baked_index;
		}
	}
		
}

LLVOAvatarDictionary::TextureEntry::TextureEntry(const std::string &name,
												 bool is_local_texture, 
												 EBakedTextureIndex baked_texture_index,
												 const std::string &default_image_name,
												 EWearableType wearable_type) :
	LLDictionaryEntry(name),
	mIsLocalTexture(is_local_texture),
	mIsBakedTexture(!is_local_texture),
	mIsUsedByBakedTexture(baked_texture_index != BAKED_NUM_INDICES),
	mBakedTextureIndex(baked_texture_index),
	mDefaultImageName(default_image_name),
	mWearableType(wearable_type)
{
}

LLVOAvatarDictionary::MeshEntry::MeshEntry(EBakedTextureIndex baked_index, 
										   const std::string &name, 
										   U8 level,
										   LLViewerJoint::PickName pick) :
	LLDictionaryEntry(name),
	mBakedID(baked_index),
	mLOD(level),
	mPickName(pick)
{
}
LLVOAvatarDictionary::BakedEntry::BakedEntry(ETextureIndex tex_index, 
											 const std::string &name, 
											 const std::string &hash_name,
											 U32 num_local_textures,
											 ... ) :
	LLDictionaryEntry(name),
	mWearablesHashID(LLUUID(hash_name)),
	mTextureIndex(tex_index)
{
	va_list argp;

	va_start(argp, num_local_textures);

	// Read in local textures
	for (U8 i=0; i < num_local_textures; i++)
	{
		ETextureIndex t = (ETextureIndex)va_arg(argp,int);
		mLocalTextures.push_back(t);
	}

	// Read in number of wearables
	const U32 num_wearables = (U32)va_arg(argp,int);
	// Read in wearables
	for (U8 i=0; i < num_wearables; i++)
	{
		EWearableType t = (EWearableType)va_arg(argp,int);
		mWearables.push_back(t);
	}
}

// static
ETextureIndex LLVOAvatarDictionary::bakedToLocalTextureIndex(EBakedTextureIndex index)
{
	return LLVOAvatarDictionary::getInstance()->getBakedTexture(index)->mTextureIndex;
}

//static 
EBakedTextureIndex LLVOAvatarDictionary::findBakedByRegionName(std::string name)
{
	U8 index = 0;
	while (index < BAKED_NUM_INDICES)
	{
		const BakedEntry *be = LLVOAvatarDictionary::getInstance()->getBakedTexture((EBakedTextureIndex) index);
		if (be && be->mName.compare(name) == 0)
		{
			// baked texture found
			return (EBakedTextureIndex) index;
		}
		index++;
	}
	// baked texture could not be found
	return BAKED_NUM_INDICES;
}

//static
const LLUUID LLVOAvatarDictionary::getDefaultTextureImageID(ETextureIndex index)
{
	const TextureEntry *texture_dict = getInstance()->getTexture(index);
	const std::string &default_image_name = texture_dict->mDefaultImageName;
	if (default_image_name == "")
	{
		return IMG_DEFAULT_AVATAR;
	}
	else
	{
		return LLUUID(gSavedSettings.getString(default_image_name));
	}
}

// static
EWearableType LLVOAvatarDictionary::getTEWearableType(ETextureIndex index )
{
	return getInstance()->getTexture(index)->mWearableType;
}

