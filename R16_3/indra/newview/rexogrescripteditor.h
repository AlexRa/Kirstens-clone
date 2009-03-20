/** 
 * @file rexogrescripteditor.h
 * @brief RexOgreScriptEditor class definition
 * 
 * Copyright (c) 2008, Ludocraft, Ltd.
 */

#ifndef LL_REXOGRESCRIPTEDITOR_H
#define LL_REXOGRESCRIPTEDITOR_H

#include "llfloater.h"
#include <OGRE/Ogre.h>
#include <map>

class RexOgreScriptEditor: public LLFloater
{
public:
	RexOgreScriptEditor(const std::string& name,
					const LLRect& rect,
					const std::string& script_name,
					const LLUUID& item_uuid,
					const LLUUID& asset_uuid,
					const LLAssetType::EType& asset_type,
					const BOOL& raw);
	
	virtual ~RexOgreScriptEditor();

	static RexOgreScriptEditor* find(const LLUUID& item_uuid);
	static RexOgreScriptEditor* show(const LLUUID& item_uuid, BOOL take_focus = TRUE);
	
	BOOL getParticleString(const LLUUID &asset_uuid);
	BOOL getMaterialString(const LLUUID &asset_uuid);
	std::map<LLString, LLString> parseMaterial(const LLUUID &asset_uuid);
	Ogre::MaterialPtr createMaterial(LLUUID asset_uuid, std::map<LLString, LLString> material_map);

protected:		
	static void onClickSave(void* userdata);
	static void onClickCancel(void* userdata);
	static void onCommitList(LLUICtrl* ctrl, void* userdata);
	static void onCommitNameEditor(LLUICtrl* ctrl, void* userdata);
	static void onCommitParameterEditor(LLUICtrl* ctrl, void* userdata);
	static void onFocusLostNameEditor(LLFocusableElement* elem, void* userdata);

	void checkNewScriptName();
	void buildRawEditUI();
	void buildParameterControls (const LLString &param_name);
	void buildParameterList();

	BOOL handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
								 EDragAndDropType cargo_type,
								 void* cargo_data,
								 EAcceptance* accept,
								 LLString& tooltip_msg);

protected:
	LLString						mTitle;
	LLString						mScriptName;
	LLString						mParticleScript;
	LLString						mMaterialScript;
	LLUUID							mItemUUID;
	LLUUID							mAssetUUID;
	BOOL							mMaterialValid;
	BOOL							mRawEdit;
	LLAssetType::EType				mAssetType;
	std::map<LLString, LLString>	mMaterialMap;
	std::vector<LLLineEditor*>		mUUIDEditors;
	std::vector<LLLineEditor*>		mVPParamEditors;
	std::vector<LLLineEditor*>		mFPParamEditors;
	std::vector<LLUICtrl*>			mUIElementList;
	
private:
	typedef std::map<LLUUID, RexOgreScriptEditor*> editor_map_t;
	static editor_map_t sInstances;
};

#endif  // LL_REXOGRESCRIPTEDITOR_H
