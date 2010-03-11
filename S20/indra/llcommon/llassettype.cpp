/** 
 * @file llassettype.cpp
 * @brief Implementatino of LLAssetType functionality.
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

#include "linden_common.h"

#include "llassettype.h"
#include "lldictionary.h"
#include "llmemory.h"
#include "llsingleton.h"

///----------------------------------------------------------------------------
/// Class LLAssetType
///----------------------------------------------------------------------------
struct AssetEntry : public LLDictionaryEntry
{
	AssetEntry(const char *desc_name,
			   const char *type_name, 	// 8 character limit!
			   const char *human_name, 	// for decoding to human readable form; put any and as many printable characters you want in each one
			   bool can_link) 			// can you create a link to this type?
		:
		LLDictionaryEntry(desc_name),
		mTypeName(type_name),
		mHumanName(human_name),
		mCanLink(can_link)
	{
		llassert(strlen(mTypeName) <= 8);
	}

	const char *mTypeName;
	const char *mHumanName;
	bool mCanLink;
};

class LLAssetDictionary : public LLSingleton<LLAssetDictionary>,
						  public LLDictionary<LLAssetType::EType, AssetEntry>
{
public:
	LLAssetDictionary();
};

LLAssetDictionary::LLAssetDictionary()
{
	//       												   DESCRIPTION			TYPE NAME	HUMAN NAME			CAN LINK?	
	//      												  |--------------------|-----------|-------------------|-----------|
	addEntry(LLAssetType::AT_TEXTURE, 			new AssetEntry("TEXTURE",			"texture",	"texture",			FALSE));
	addEntry(LLAssetType::AT_SOUND, 			new AssetEntry("SOUND",				"sound",	"sound",			FALSE));
	addEntry(LLAssetType::AT_CALLINGCARD, 		new AssetEntry("CALLINGCARD",		"callcard",	"calling card",		FALSE));
	addEntry(LLAssetType::AT_LANDMARK, 			new AssetEntry("LANDMARK",			"landmark",	"landmark",			FALSE));
	addEntry(LLAssetType::AT_SCRIPT, 			new AssetEntry("SCRIPT",			"script",	"legacy script",	FALSE));
	addEntry(LLAssetType::AT_CLOTHING, 			new AssetEntry("CLOTHING",			"clothing",	"clothing",			TRUE));
	addEntry(LLAssetType::AT_OBJECT, 			new AssetEntry("OBJECT",			"object",	"object",			TRUE));
	addEntry(LLAssetType::AT_NOTECARD, 			new AssetEntry("NOTECARD",			"notecard",	"note card",		FALSE));
	addEntry(LLAssetType::AT_CATEGORY, 			new AssetEntry("CATEGORY",			"category",	"folder",			TRUE));
	addEntry(LLAssetType::AT_LSL_TEXT, 			new AssetEntry("LSL_TEXT",			"lsltext",	"lsl2 script",		FALSE));
	addEntry(LLAssetType::AT_LSL_BYTECODE, 		new AssetEntry("LSL_BYTECODE",		"lslbyte",	"lsl bytecode",		FALSE));
	addEntry(LLAssetType::AT_TEXTURE_TGA, 		new AssetEntry("TEXTURE_TGA",		"txtr_tga",	"tga texture",		FALSE));
	addEntry(LLAssetType::AT_BODYPART, 			new AssetEntry("BODYPART",			"bodypart",	"body part",		TRUE));
	addEntry(LLAssetType::AT_SOUND_WAV, 		new AssetEntry("SOUND_WAV",			"snd_wav",	"sound",			FALSE));
	addEntry(LLAssetType::AT_IMAGE_TGA, 		new AssetEntry("IMAGE_TGA",			"img_tga",	"targa image",		FALSE));
	addEntry(LLAssetType::AT_IMAGE_JPEG, 		new AssetEntry("IMAGE_JPEG",		"jpeg",		"jpeg image",		FALSE));
	addEntry(LLAssetType::AT_ANIMATION, 		new AssetEntry("ANIMATION",			"animatn",	"animation",		FALSE));
	addEntry(LLAssetType::AT_GESTURE, 			new AssetEntry("GESTURE",			"gesture",	"gesture",			TRUE));
	addEntry(LLAssetType::AT_SIMSTATE, 			new AssetEntry("SIMSTATE",			"simstate",	"simstate",			FALSE));

	addEntry(LLAssetType::AT_LINK, 				new AssetEntry("LINK",				"link",		"symbolic link",	FALSE));
	addEntry(LLAssetType::AT_LINK_FOLDER, 		new AssetEntry("FOLDER_LINK",		"link_f", 	"symbolic folder link", FALSE));

	addEntry(LLAssetType::AT_NONE, 				new AssetEntry("NONE",				"-1",		NULL,		  		FALSE));
};

// static
LLAssetType::EType LLAssetType::getType(const std::string& desc_name)
{
	std::string s = desc_name;
	LLStringUtil::toUpper(s);
	return LLAssetDictionary::getInstance()->lookup(s);
}

// static
const std::string &LLAssetType::getDesc(LLAssetType::EType asset_type)
{
	const AssetEntry *entry = LLAssetDictionary::getInstance()->lookup(asset_type);
	if (entry)
	{
		return entry->mName;
	}
	else
	{
		return badLookup();
	}
}

// static
const char *LLAssetType::lookup(LLAssetType::EType asset_type)
{
	const LLAssetDictionary *dict = LLAssetDictionary::getInstance();
	const AssetEntry *entry = dict->lookup(asset_type);
	if (entry)
	{
		return entry->mTypeName;
	}
	else
	{
		return badLookup().c_str();
	}
}

// static
LLAssetType::EType LLAssetType::lookup(const char* name)
{
	return lookup(ll_safe_string(name));
}

// static
LLAssetType::EType LLAssetType::lookup(const std::string& type_name)
{
	const LLAssetDictionary *dict = LLAssetDictionary::getInstance();
	for (LLAssetDictionary::const_iterator iter = dict->begin();
		 iter != dict->end();
		 iter++)
	{
		const AssetEntry *entry = iter->second;
		if (type_name == entry->mTypeName)
		{
			return iter->first;
		}
	}
	return AT_NONE;
}

// static
const char *LLAssetType::lookupHumanReadable(LLAssetType::EType asset_type)
{
	const LLAssetDictionary *dict = LLAssetDictionary::getInstance();
	const AssetEntry *entry = dict->lookup(asset_type);
	if (entry)
	{
		return entry->mHumanName;
	}
	else
	{
		return badLookup().c_str();
	}
}

// static
LLAssetType::EType LLAssetType::lookupHumanReadable(const char* name)
{
	return lookupHumanReadable(ll_safe_string(name));
}

// static
LLAssetType::EType LLAssetType::lookupHumanReadable(const std::string& readable_name)
{
	const LLAssetDictionary *dict = LLAssetDictionary::getInstance();
	for (LLAssetDictionary::const_iterator iter = dict->begin();
		 iter != dict->end();
		 iter++)
	{
		const AssetEntry *entry = iter->second;
		if (entry->mHumanName && (readable_name == entry->mHumanName))
		{
			return iter->first;
		}
	}
	return AT_NONE;
}

// static
bool LLAssetType::lookupCanLink(EType asset_type)
{
	const LLAssetDictionary *dict = LLAssetDictionary::getInstance();
	const AssetEntry *entry = dict->lookup(asset_type);
	if (entry)
	{
		return entry->mCanLink;
	}
	return false;
}

// static
// Not adding this to dictionary since we probably will only have these two types
bool LLAssetType::lookupIsLinkType(EType asset_type)
{
	if (asset_type == AT_LINK || asset_type == AT_LINK_FOLDER)
	{
		return true;
	}
	return false;
}

// static
const std::string &LLAssetType::badLookup()
{
	static const std::string sBadLookup = "llassettype_bad_lookup";
	return sBadLookup;

}
