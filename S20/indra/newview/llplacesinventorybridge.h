/** 
 * @file llplacesinventorybridge.h
 * @brief Declaration of the Inventory-Folder-View-Bridge classes for Places Panel.
 *
 * $LicenseInfo:firstyear=2009&license=viewergpl$
 * 
 * Copyright (c) 2009-2010, Linden Research, Inc.
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

#ifndef LL_LLPLACESINVENTORYBRIDGE_H
#define LL_LLPLACESINVENTORYBRIDGE_H

#include "llinventorybridge.h"

class LLFolderViewFolder;

/**
 * Overridden version of the Inventory-Folder-View-Bridge for Places Panel (Landmarks Tab)
 */
class LLPlacesLandmarkBridge : public LLLandmarkBridge
{
	friend class LLPlacesInventoryBridgeBuilder;

public:
	/*virtual*/ void buildContextMenu(LLMenuGL& menu, U32 flags);

protected:
	LLPlacesLandmarkBridge(LLInventoryType::EType type, LLInventoryPanel* inventory, const LLUUID& uuid, U32 flags = 0x00)
		: LLLandmarkBridge(inventory, uuid, flags) {mInvType = type;}
};

/**
 * Overridden version of the Inventory-Folder-View-Bridge for Folders
 */
class LLPlacesFolderBridge : public LLFolderBridge
{
	friend class LLPlacesInventoryBridgeBuilder;

public:
	/*virtual*/ void buildContextMenu(LLMenuGL& menu, U32 flags);
	/*virtual*/ void performAction(LLFolderView* folder, LLInventoryModel* model, std::string action);

protected:
	LLPlacesFolderBridge(LLInventoryType::EType type, LLInventoryPanel* inventory, const LLUUID& uuid)
		: LLFolderBridge(inventory, uuid) {mInvType = type;}

	LLFolderViewFolder* getFolder();
};


/**
 * This class intended to override default InventoryBridgeBuilder for Inventory Panel.
 *
 * It builds Bridges for Landmarks and Folders in Places Landmarks Panel
 */
class LLPlacesInventoryBridgeBuilder : public LLInventoryFVBridgeBuilder
{
public:
	/*virtual*/ LLInvFVBridge* createBridge(
		LLAssetType::EType asset_type,
		LLAssetType::EType actual_asset_type,
		LLInventoryType::EType inv_type,
		LLInventoryPanel* inventory,
		const LLUUID& uuid,
		U32 flags = 0x00) const;
};

#endif // LL_LLPLACESINVENTORYBRIDGE_H
