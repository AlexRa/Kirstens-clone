/** 
 * @file rexogrescripteditor.cpp
 * @brief RexOgreScriptEditor class implementation
 *
 * Copyright (c) 2008, Ludocraft, Ltd.
 * 
 */

// Includes
#include "llviewerprecompiledheaders.h"

// file include
#include "rexogrescripteditor.h"

#include "lluictrlfactory.h"
#include "llinventory.h"
#include "llogre.h"
#include "llogreassetloader.h"
#include "rexogrematerial.h"
#include "rexparticlescript.h"
#include "llassettype.h"
#include "llinventorytype.h"
#include "llviewermenufile.h"		//upload_new_resource_from_buffer
#include "llview.h"
#include "llcombobox.h"
#include "lltexteditor.h"
#include "rexogretexture.h"
#include "llviewerimagelist.h"
#include "llinventory.h"
#include "llviewerinventory.h"
#include "llinventorymodel.h"
#include "llviewercontrol.h"

// Statics
RexOgreScriptEditor::editor_map_t RexOgreScriptEditor::sInstances;


RexOgreScriptEditor::RexOgreScriptEditor(const std::string& name,
										const LLRect& rect,
										const std::string& script_name,
										const LLUUID& item_uuid,
										const LLUUID& asset_uuid,
										const LLAssetType::EType& asset_type,
										const BOOL& raw):
										LLFloater(name, rect, "Ogre Script Editor", TRUE, 340, 415),
										mScriptName(script_name),
										mItemUUID(item_uuid),
										mAssetUUID(asset_uuid),
										mAssetType(asset_type),
										mRawEdit(raw)
{
	// Build floater
	if(mRawEdit)
		LLUICtrlFactory::getInstance()->buildFloater(this, "floater_rex_ogre_script_raw_editor.xml");
	else
		LLUICtrlFactory::getInstance()->buildFloater(this, "floater_rex_ogre_script_editor.xml");

	// Init callbacks
	childSetAction("save_button", onClickSave, this);
	childSetAction("cancel_button", onClickCancel, this);
	childSetCommitCallback("name_editor", onCommitNameEditor, this);
	childSetCommitCallback("parameter_list", onCommitList, this);
	childSetFocusChangedCallback("name_editor", onFocusLostNameEditor, this);

	// Set title
	if(mAssetType == LLAssetType::AT_MATERIAL)
		mTitle = "Material Script Editor: ";		
	else if(mAssetType == LLAssetType::AT_PARTICLE_SCRIPT)
		mTitle = "Particle Script Editor: ";

	mTitle.append(mScriptName);
	setTitle(mTitle);
	
	// Build UI
	if(!mRawEdit)
	{
		// Build parameter list
		if(getMaterialString(mAssetUUID))
		{
			mMaterialMap = parseMaterial(mAssetUUID);
			buildParameterList();
		}
		else
		{
			LLCtrlListInterface *list = childGetListInterface("parameter_list");
			if (!list) return;

			list->addSimpleElement("Script parsing failed.");
			list->addSimpleElement("Try re-opening the script.");
		}
	}
	else
	{
		// Build just text editor
		buildRawEditUI();
	}

	childSetValue("name_editor", mScriptName);
	childDisable("save_button");
	childEnable("cancel_button");

	if(mItemUUID.notNull())
	{
		sInstances[mItemUUID] = this;
	}

	if (!getHost())
	{
		LLRect curRect = getRect();
		translate(rect.mLeft - curRect.mLeft, rect.mTop - curRect.mTop);
	}
}

// virtual
RexOgreScriptEditor::~RexOgreScriptEditor()
{	
	// Save position of the floater
	gSavedSettings.setRect("RexOgreScriptEditorRect", getRect());

	// Erase instance
	if (mItemUUID.notNull())
	{
		sInstances.erase(mItemUUID);
	}
}

// static
RexOgreScriptEditor* RexOgreScriptEditor::find(const LLUUID& item_uuid)
{
	RexOgreScriptEditor* instance = NULL;
	editor_map_t::iterator found_it = RexOgreScriptEditor::sInstances.find(item_uuid);
	if(found_it != RexOgreScriptEditor::sInstances.end())
	{
		instance = found_it->second;
	}
	return instance;
}

// static
RexOgreScriptEditor* RexOgreScriptEditor::show(const LLUUID& item_uuid, BOOL take_focus)
{
	RexOgreScriptEditor* instance = RexOgreScriptEditor::find(item_uuid);
	if(instance)
	{
		if (LLFloater::getFloaterHost() && LLFloater::getFloaterHost() != instance->getHost())
		{
			LLFloater::getFloaterHost()->addFloater(instance, TRUE);
		}
		instance->open();  /*Flawfinder: ignore*/
		if (take_focus)
		{
			instance->setFocus(TRUE);
		}
	}

	return instance;
}

void RexOgreScriptEditor::buildParameterList()
{
	LLCtrlListInterface *list = childGetListInterface("parameter_list");
	if(!list) return;
	
	S32 item_count = 0;
	std::map<LLString, LLString>::iterator iter;

	for(iter = mMaterialMap.begin(); iter != mMaterialMap.end(); ++iter)
	{
		LLString name;
		LLString name2;
		LLString type;
		LLString value;

		// Get the parameter type
		// 1) If the iter->second is valid uuid or iter->first end with '#' then it's a texture unit
		// 2) If iter->first ends with '@' then it's a vp
		// 3) If iter->first ends with '*' then it's a fp		
		LLString::size_type TU_found = iter->first.find_last_of('#');
		LLString::size_type VP_found = iter->first.find_last_of('@');
		LLString::size_type FP_found = iter->first.find_last_of('*');

		if(LLUUID::validate(iter->second) || TU_found != iter->first.npos)
		{
			type = "tex2d";
			name = iter->first;
			name2 = name.substr(0, name.find_last_of('#'));
			
			// Collect textures from inventory
			LLViewerInventoryCategory::cat_array_t cats;
			LLViewerInventoryItem::item_array_t items;
			LLIsType isTypeTexture(LLAssetType::AT_TEXTURE);

			gInventory.collectDescendentsIf(LLUUID::null,
									cats,
									items,
									LLInventoryModel::INCLUDE_TRASH,
									isTypeTexture);
			
			//...and find the name of the currently used texture
			// Show name if it has been fetched
			BOOL found = FALSE;
			for (S32 i = 0; i < items.count(); i++)
			{
				LLInventoryItem* itemp = items[i];
				if(itemp->getAssetUUID().asString() == iter->second)
				{
					if(itemp->getName().size() > 0)
					{
						value = itemp->getName();
						found = TRUE;
					}
				}
			}
			if(!found)
			{
				// Show just UUID
				value = iter->second;				
			}
		}

		if(VP_found != iter->first.npos)
		{	
			type = "float4(VP)";
			name = iter->first;
			name2 = name.substr(0, name.find_last_of('@'));
			value = iter->second;
		}

		if(FP_found != iter->first.npos)
		{
			type = "float4(FP)";
			name = iter->first;
			name2 = name.substr(0, name.find_last_of('*'));
			value = iter->second;
		}

		//Build the list
		LLSD row;
		LLString style = "NORMAL";
		
		row["id"] = name;

		row["columns"][0]["column"] = "name";
		row["columns"][0]["value"] = name2;
		row["columns"][0]["font-style"] = style;

		row["columns"][1]["column"] = "type";
		row["columns"][1]["value"] = type;
		row["columns"][1]["font-style"] = style;

		row["columns"][2]["column"] = "value";
		row["columns"][2]["value"] = value;
		row["columns"][2]["font-style"] = style;

		list->addElement(row);
		item_count++;
	}

	list->sortByColumn("type", FALSE);
}

// static
void RexOgreScriptEditor::onCommitList(LLUICtrl* ctrl, void* userdata)
{
	RexOgreScriptEditor* self = (RexOgreScriptEditor*)userdata;

	LLCtrlSelectionInterface* iface = self->childGetSelectionInterface("parameter_list");
	if (!iface) return;
	
	LLString param_name = iface->getSelectedValue();
	self->buildParameterControls(param_name);
}

// static
void RexOgreScriptEditor::onClickSave(void *userdata)
{
	RexOgreScriptEditor* self = (RexOgreScriptEditor*) userdata;
	
	std::map<LLString, LLString>::iterator iter;
	
	if(self->mRawEdit)
	{
		// Raw editing
		// New name and script text for the new ogre script modified from the original
		LLString new_script_name = self->childGetText("name_editor");
		LLString new_script = self->childGetValue("raw_editor").asString();
		S32 script_size = new_script.size();
		const S32 buf_size = 65536;
		U8 buffer[buf_size];
		
		// Copy string to buffer
		for(S32 i = 0; i < script_size; i++)
		{
			buffer[i] = new_script[i];
		}

		// Upload
		if(self->mAssetType == LLAssetType::AT_MATERIAL)
		{
			// Material script
			upload_new_resource_from_buffer(buffer, script_size, LLAssetType::AT_MATERIAL,
				new_script_name, "New material script", 0, LLAssetType::AT_MATERIAL,
				LLInventoryType::IT_OGRESCRIPT);
		}
		else if(self->mAssetType == LLAssetType::AT_PARTICLE_SCRIPT)
		{
			// Particle script
			upload_new_resource_from_buffer(buffer, script_size, LLAssetType::AT_PARTICLE_SCRIPT,
				new_script_name, "New particle script", 0, LLAssetType::AT_PARTICLE_SCRIPT,
				LLInventoryType::IT_OGRESCRIPT);		
		}
	}
	else
	{
		// Parameter editing
        // Switch to Ogre GL context to be sure
        LLOgreRenderer::getPointer()->setOgreContext();
        try
        {
			// New material
            Ogre::MaterialPtr NewMatPtr = self->createMaterial(self->mAssetUUID, self->mMaterialMap);

            if(!NewMatPtr.isNull() /*&& self->mMaterialValid*/)
            {
                // New name
                LLString new_script_name = self->childGetText("name_editor");
                Ogre::MaterialSerializer serializer;

                // Material as string	
                serializer.queueForExport(NewMatPtr, true, true);
                LLString materialString = serializer.getQueuedAsString();
                serializer.clearQueue();

                S32 script_size = materialString.size();
                const S32 buf_size = 65536;
                U8 buffer[buf_size];

                for(S32 i = 0; i < script_size; i++)
                {
                    buffer[i] = materialString[i];
                }

                // Upload
                upload_new_resource_from_buffer(buffer, script_size, LLAssetType::AT_MATERIAL,
					new_script_name, "New material script", 0, LLAssetType::AT_MATERIAL,
					LLInventoryType::IT_OGRESCRIPT);

                // Remove cloned material now, it is no longer needed
                std::string newMatName = NewMatPtr->getName();
                NewMatPtr.setNull();
                Ogre::MaterialManager::getSingleton().remove(newMatName);
            }
            else
            {
				llwarns << "New material could not be created!" << llendl;
            }
        }
        catch (Ogre::Exception e)
        {
        }

        // Now switch back to SL context
        LLOgreRenderer::getPointer()->setSLContext();
	}
}

// static
void RexOgreScriptEditor::onClickCancel(void *userdata)
{
	RexOgreScriptEditor* self = (RexOgreScriptEditor*) userdata;
	self->close();
}

// static
void RexOgreScriptEditor::onFocusLostNameEditor(LLFocusableElement* elem, void* userdata)
{
    RexOgreScriptEditor* self = (RexOgreScriptEditor*)userdata;
	self->checkNewScriptName();
}

// static
void RexOgreScriptEditor::onCommitNameEditor(LLUICtrl* ctrl, void* userdata)
{
    RexOgreScriptEditor* self = (RexOgreScriptEditor*)userdata;
	self->checkNewScriptName();
}

//static
void RexOgreScriptEditor::onCommitParameterEditor(LLUICtrl* ctrl, void* userdata)
{
	// Save changes on commit
	LLLineEditor *editor = (LLLineEditor*)ctrl;
	RexOgreScriptEditor *self = (RexOgreScriptEditor*)userdata;
	
	LLString param_name = editor->getName();
	LLString param_value = editor->getValue().asString();
	std::map<LLString, LLString>::iterator iter;
	
	// If the param_value is valid uuid then it's a tu texture name
	if(LLUUID::validate(param_value))
	{
		// Update param value
		iter = self->mMaterialMap.find(param_name);
		if(iter != self->mMaterialMap.end())
		{
			iter->second = param_value;
			if(param_value != iter->second)
				iter->second = param_value;
		}		
	}
	
	LLString::size_type VP_found = param_name.find_last_of('@');
	if(VP_found != param_name.npos)
	{
		
		// Get values from the fragment and vertex program parameter value line editors (4 editors for one value)
		//for(U16 i = 0; i < self->mVPParamEditors.size(); i += 4)
		//{
			LLLineEditor *editor1 = self->mVPParamEditors[0];
			LLLineEditor *editor2 = self->mVPParamEditors[1];
			LLLineEditor *editor3 = self->mVPParamEditors[2];
			LLLineEditor *editor4 = self->mVPParamEditors[3];
			
			//Check that the values belong to the same parameter
			if(editor1->getName() == editor2->getName() &&
				editor1->getName() == editor3->getName() &&
				editor1->getName() == editor4->getName())
			{
				// Get values
				std::stringstream vector_string;														
				vector_string << editor1->getValue().asString() << " " << editor2->getValue().asString()
					<< " " << editor3 ->getValue().asString() << " " << editor4->getValue().asString();

				// Insert to the material map
				iter = self->mMaterialMap.find(param_name);
				if(iter != self->mMaterialMap.end())
					iter->second = vector_string.str();
			}
		//}
	}
	
	LLString::size_type FP_found = param_name.find_last_of('*');
	if(FP_found != param_name.npos)
	{
		// Get values from the fragment program parameter value line editors (4 editors for one value)

		LLLineEditor *editor1 = self->mFPParamEditors[0];
		LLLineEditor *editor2 = self->mFPParamEditors[1];
		LLLineEditor *editor3 = self->mFPParamEditors[2];
		LLLineEditor *editor4 = self->mFPParamEditors[3];

		//Check that the values belong to the same parameter
		if(editor1->getName() == editor2->getName() &&
			editor1->getName() == editor3->getName() &&
			editor1->getName() == editor4->getName())
		{
			// Get values
			std::stringstream vector_string;														
			vector_string << editor1->getValue().asString() << " " << editor2->getValue().asString()
				<< " " << editor3 ->getValue().asString() << " " << editor4->getValue().asString();

			// Insert to the material map
			iter = self->mMaterialMap.find(param_name);
			if(iter != self->mMaterialMap.end())
				iter->second = vector_string.str();
		}
	}

	// Update list
	LLCtrlListInterface *list = self->childGetListInterface("parameter_list");
	if (!list) return;
	
	list->clearRows();
	self->buildParameterList();
}

BOOL RexOgreScriptEditor::getParticleString(const LLUUID &asset_uuid)
{
	LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();
	if(!assetLoader) return FALSE;
	
	if(!assetLoader->areParticleScriptsLoaded(asset_uuid))
    {
		assetLoader->loadAsset(asset_uuid, LLAssetType::AT_PARTICLE_SCRIPT, 0);
	}

	RexParticleScript* particle = assetLoader->getParticleScript(asset_uuid);

	if(particle)
	{
		LLString particleString = particle->getParticleString();

		// Find tabulator symbols from the string and replace them with three spaces
		char const TAB = '\t';
		LLString::size_type i = 0, j = 0;
			
		LLString::trim(particleString);

		while(j != LLString::npos)
		{
			j = particleString.find(TAB, i);
			if(j != LLString::npos)
			{
				particleString.replace(j, 1, " ");
				particleString.insert(j+1, 2, ' ');
			}
			i = j;
		}

		mParticleScript = particleString;
		return TRUE;
	}

	else return FALSE;
}

BOOL RexOgreScriptEditor::getMaterialString(const LLUUID &asset_uuid)
{
	// Get asset loader
	LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();
	if (!assetLoader) return FALSE;

	if(!assetLoader->isMaterialLoaded(asset_uuid))
	{
		// Material not loaded, load now
		assetLoader->loadAsset(asset_uuid, LLAssetType::AT_MATERIAL, 0);
	}
	
	RexOgreMaterial* material = assetLoader->getMaterial(asset_uuid);
	if(!material) return FALSE;
	
	Ogre::MaterialPtr MatPtr = material->getMaterial();
	if(!MatPtr.isNull())
	{
		Ogre::MaterialSerializer serializer;
		serializer.queueForExport(MatPtr, true, true);
		LLString materialString = serializer.getQueuedAsString();
		serializer.clearQueue();
			
		// Find tabulator symbols from the string and replace them with three spaces
		char const TAB = '\t';
		LLString::size_type i = 0, j = 0;
		
		LLString::trim(materialString);

		while(j != LLString::npos)
		{
			j = materialString.find(TAB, i);
			if(j != LLString::npos)
			{
				materialString.replace(j, 1, " ");
				materialString.insert(j+1, 2, ' ');
			}

			i = j;
		}

		mMaterialScript = materialString;
		return TRUE;
	}

	return FALSE;
}

void RexOgreScriptEditor::checkNewScriptName()
{
	// Name of the new script must differ from the original
	LLString new_name = childGetText("name_editor");
	if(new_name == mScriptName)
	{
		childDisable("save_button");
	}
	else
	{
		childEnable("save_button");
	}
}

std::map<LLString, LLString> RexOgreScriptEditor::parseMaterial(const LLUUID &asset_uuid)
{
	std::map<LLString, LLString> parameter_map;

	// Get asset loader
	LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();
	if(!assetLoader) return parameter_map;

	if(!assetLoader->isMaterialLoaded(asset_uuid))
	{
		// Material not loaded, load now
		assetLoader->loadAsset(asset_uuid, LLAssetType::AT_MATERIAL, 0);
	}

	RexOgreMaterial* material = assetLoader->getMaterial(asset_uuid);
	if(!material) return parameter_map;
	
	Ogre::MaterialPtr MatPtr = material->getMaterial();
	if(!MatPtr.isNull())
	{
		// Material
		Ogre::Material::TechniqueIterator tIter = MatPtr->getTechniqueIterator();
		int techNum = 0;
		while(tIter.hasMoreElements())
		{
			// Technique
			Ogre::Technique *tech = tIter.getNext();
			Ogre::Technique::PassIterator pIter = tech->getPassIterator();
			int passNum = 0;
			while(pIter.hasMoreElements())
			{
				// Pass
				Ogre::Pass *pass = pIter.getNext();
				if(pass)
				{
					if(pass->hasVertexProgram())
					{
						// Vertex program
						const Ogre::GpuProgramPtr verProg = pass->getVertexProgram();
						if(!verProg.isNull())
						{
							Ogre::GpuProgramParametersSharedPtr verPtr = pass->getVertexProgramParameters();
							if(verPtr->hasNamedParameters())
							{
								// Named parameters (constants)
								Ogre::GpuConstantDefinitionIterator mapIter = verPtr->getConstantDefinitionIterator();									
								int constNum = 0;
								while(mapIter.hasMoreElements())
								{
									std::string paramName = mapIter.peekNextKey();
									const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();
									// Filter names that end with '[0]'
									std::string::size_type found = paramName.find_last_of("[0]");
									if(found == paramName.npos)
									{	
										// Ignore auto parameters
										BOOL is_auto_param = FALSE;
										Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = verPtr->getAutoConstantIterator();
										while(autoConstIter.hasMoreElements())
										{
											Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
											if(autoConstEnt.physicalIndex == paramDef.physicalIndex)
											{
												is_auto_param = TRUE;
												break;
											}
										}
										
										if(!is_auto_param)
										{
											if(paramDef.isFloat())
											{
												size_t count = paramDef.elementSize * paramDef.arraySize;
												std::vector<float> paramValue;
												std::vector<float>::iterator iter;
												paramValue.resize(count, 0);
												verPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());
												
												std::stringstream vector_string;													
												for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
												{
													vector_string << *iter << " ";
												}
												
												// Insert vertex program values into the map
												// add to "@" to the end of the parameter name in order to identify VP parameters
												paramName.append("@");
												parameter_map.insert(std::pair<LLString, LLString>(paramName, vector_string.str()));
											}
										}
									}

									++constNum;
								}
							}
						}
					}
					
					if(pass->hasFragmentProgram())
					{
						// Fragment program
						const Ogre::GpuProgramPtr fragProg = pass->getFragmentProgram();
						if(!fragProg.isNull())
						{
							Ogre::GpuProgramParametersSharedPtr fragPtr = pass->getFragmentProgramParameters();
							if(!fragPtr.isNull())
							{
								if(fragPtr->hasNamedParameters())
								{
									// Named parameters (constants)
									Ogre::GpuConstantDefinitionIterator mapIter = fragPtr->getConstantDefinitionIterator();
									int constNum = 0;
									while(mapIter.hasMoreElements())
									{
										std::string paramName = mapIter.peekNextKey();
										const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();
										// Filter names that end with '[0]'
										std::string::size_type found = paramName.find_last_of("[0]");
										if(found == paramName.npos)
										{
											// Ignore auto parameters
											BOOL is_auto_param = FALSE;
											Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = fragPtr->getAutoConstantIterator();
											while(autoConstIter.hasMoreElements())
											{
												Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
												if(autoConstEnt.physicalIndex == paramDef.physicalIndex)
												{
													is_auto_param = TRUE;
													break;
												}
											}
											
											if(!is_auto_param)
											{
												if(paramDef.isFloat())
												{
													size_t count = paramDef.elementSize * paramDef.arraySize;
													std::vector<float> paramValue;
													std::vector<float>::iterator iter;
													paramValue.resize(count, 0);
													fragPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());
													
													std::stringstream vector_string;							
													for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
													{
														vector_string << *iter << " ";
													}
													
													// Insert fragment program values into the map
													// add to "*" to the end of the parameter name in order to identify FP parameters
													paramName.append("*");
													parameter_map.insert(std::pair<LLString, LLString>(paramName, vector_string.str()));
												}
											}
										}

										++constNum;
									}
								}
							}
						}
					}
					
					Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
					int tuNum = 0;
					while(texIter.hasMoreElements())
					{
						// Texture units
						const Ogre::TextureUnitState *tu = texIter.getNext();
						
						// Insert texture unit values into the map
						// Don't insert tu's with empty texture names (i.e. shadowMap)
						// add to "#" to the end of the parameter name in order to identify FP parameters
						if(tu->getTextureName().size() > 0)
						{
							LLString tu_name;
							tu_name = tu->getName().c_str();
							tu_name.append("#");
							parameter_map.insert(std::pair<LLString, LLString>(tu_name, tu->getTextureName().c_str()));
						}

						++tuNum;
					}
						
					++passNum;
				}
			}
				
			++techNum;
		}
	}

	return parameter_map;
}

Ogre::MaterialPtr RexOgreScriptEditor::createMaterial(LLUUID asset_uuid, std::map<LLString, LLString> material_map)
{	
	std::map<LLString, LLString>::const_iterator iter;
	
	// Get asset loader
	LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();
	if (!assetLoader) 
	{
		Ogre::MaterialPtr nullPtr;
		nullPtr.setNull();
		return nullPtr;
	}

	if(!assetLoader->isMaterialLoaded(asset_uuid))
	{
		// Material not loaded, load now
		assetLoader->loadAsset(asset_uuid, LLAssetType::AT_MATERIAL, 0);
	}

	RexOgreMaterial* material = assetLoader->getMaterial(asset_uuid);
	if (!material) 
	{
		Ogre::MaterialPtr nullPtr;
		nullPtr.setNull();
		return nullPtr;
	}

	// Make clone from the original and uset that for the new material
	Ogre::MaterialPtr MatPtr = material->getMaterial();
	Ogre::MaterialPtr MatPtrClone = MatPtr->clone("MatPtrClone");
	if(!MatPtrClone.isNull())
	{
		Ogre::Material::TechniqueIterator tIter = MatPtrClone->getTechniqueIterator();
		int techNum = 0;
		while(tIter.hasMoreElements())
		{
			Ogre::Technique *tech = tIter.getNext();
			Ogre::Technique::PassIterator pIter = tech->getPassIterator();
			int passNum = 0;
			while(pIter.hasMoreElements())
			{
				Ogre::Pass *pass = pIter.getNext();
				if(pass)
				{
					if(pass->hasVertexProgram())
					{
						// Vertex program
						const Ogre::GpuProgramPtr verProg = pass->getVertexProgram();
						if(!verProg.isNull())
						{
							Ogre::GpuProgramParametersSharedPtr verPtr = pass->getVertexProgramParameters();
							if(verPtr->hasNamedParameters())
							{
								// Named parameters (constants)
								Ogre::GpuConstantDefinitionIterator mapIter = verPtr->getConstantDefinitionIterator();
								int constNum = 0;
								while(mapIter.hasMoreElements())
								{
									std::string paramName = mapIter.peekNextKey();
									std::string paramNameAppended = paramName.append("@");
									const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();
									
									// Filter names that end with '[0]'
									std::string::size_type found = paramName.find_last_of("[0]");
									if(found == paramName.npos)
									{
										if(paramDef.isFloat())
										{
											size_t count = paramDef.elementSize * paramDef.arraySize;
											std::vector<float> newParamValue;
											std::vector<float>::iterator it;
											newParamValue.resize(count, 0);
											
											// Find the corresponding value from the material map
											iter = material_map.find(paramNameAppended);
											if(iter != material_map.end())
											{
												LLString new_value_as_string = iter->second;
												LLString::trim(new_value_as_string);
	
												// fill the float vector with new values
												it = newParamValue.begin();
												LLString::size_type i = 0, j = 0;
												while(j != LLString::npos)
												{
													j = new_value_as_string.find(' ', i);
													LLString new_value_substr = new_value_as_string.substr(i, j == LLString::npos ? j : j - i);
													
													if(!new_value_substr.empty())
													{
														*it = atof(new_value_substr.c_str());
														++it;
													}

													i = j + 1;
												}
												
												// Set the new value											
												Ogre::Vector4 vector(newParamValue[0], newParamValue[1], newParamValue[2], newParamValue[3]);
												verPtr->_writeRawConstant(paramDef.physicalIndex, vector);
											}
										}
									}

									++constNum;
								}						
							}
						}
					}

					if(pass->hasFragmentProgram())
					{
						// Fragment program
						const Ogre::GpuProgramPtr fragProg = pass->getFragmentProgram();
						if(!fragProg.isNull())
						{
							Ogre::GpuProgramParametersSharedPtr fragPtr = pass->getFragmentProgramParameters();
							if(!fragPtr.isNull())
							{
								if(fragPtr->hasNamedParameters())
								{
									// Named parameters (constants)
									Ogre::GpuConstantDefinitionIterator mapIter = fragPtr->getConstantDefinitionIterator();
									int constNum = 0;
									while(mapIter.hasMoreElements())
									{
										std::string paramName = mapIter.peekNextKey();
										std::string paramNameAppended = paramName.append("*");
										const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

										// Filter names that end with '[0]'
										std::string::size_type found = paramName.find_last_of("[0]");
										if(found == paramName.npos)
										{
											if(paramDef.isFloat())
											{
												size_t count = paramDef.elementSize * paramDef.arraySize;
												std::vector<float> newParamValue;
												std::vector<float>::iterator it;
												newParamValue.resize(count, 0);
												
												// Find the corresponding value from the material map
												iter = material_map.find(paramNameAppended);
												if(iter != material_map.end())
												{
													LLString new_value_as_string = iter->second;
													LLString::trim(new_value_as_string);
														// fill the float vector with new values
													it = newParamValue.begin();
													LLString::size_type i = 0, j = 0;
													while(j != LLString::npos)
													{
														j = new_value_as_string.find(' ', i);
														LLString new_value_substr = new_value_as_string.substr(i, j == LLString::npos ? j : j - i);
														
														if(!new_value_substr.empty())
														{
															*it = atof(new_value_substr.c_str());
															++it;
														}
														i = j + 1;
													}

													// Set the new value												
													Ogre::Vector4 vector(newParamValue[0], newParamValue[1], newParamValue[2], newParamValue[3]);
													fragPtr->_writeRawConstant(paramDef.physicalIndex, vector);
												}
											}
										}

										++constNum;
									}
								}
							}
						}
					}
					
					Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
					int tuNum = 0;
					while(texIter.hasMoreElements())
					{
						// Texture units
						Ogre::TextureUnitState *tu = texIter.getNext();
						// Replace the texture name (uuid) with the new one
						LLString tu_name = tu->getName().c_str();
						tu_name.append("#");
						iter = material_map.find(tu_name);
						if(iter != material_map.end())
						{
							LLString new_texture_name = iter->second;
							// If new texture is UUID-based one, make sure the corresponding RexOgreTexture gets created,
							// because we may not be able to load it later if load fails now
							if (LLUUID::validate(new_texture_name))
							{
								LLUUID imageID(new_texture_name);
								if (imageID != LLUUID::null)
								{
									LLViewerImage* image = gImageList.getImage(imageID);
									if (image)
									{
										image->getOgreTexture();
									}
								}
							}
						
							tu->setTextureName(iter->second);
						}

						++tuNum;
					}

					++passNum;
				}
			}

			++techNum;
		}

		return MatPtrClone;
	}
	
	MatPtrClone.setNull();
	return MatPtrClone;
}

void RexOgreScriptEditor::buildRawEditUI()
{
	S32 left = 5;
	S32 right = getRect().getWidth() - 5;
	S32 top = getRect().getHeight() - 50;
	S32 bottom = top - 300;

	LLTextEditor* raw_editor = NULL;
	raw_editor = new LLTextEditor("raw_editor",
									LLRect(left, top, right, bottom), 
									5000,
									LLString::null,
									LLFontGL::sSansSerifSmall,
									TRUE);
	raw_editor->setFollowsAll();
	raw_editor->setWordWrap(TRUE);
	addChild(raw_editor);
	
	if(mAssetType == LLAssetType::AT_PARTICLE_SCRIPT)
	{
		if(getParticleString(mAssetUUID))
		{
			childSetValue("raw_editor", mParticleScript);
		}
		else
		{
			childSetValue("raw_editor", "Parsing particle script failed. Try re-opening the script.");
		}
	}
	else if(mAssetType == LLAssetType::AT_MATERIAL)
	{
		if(getMaterialString(mAssetUUID))
		{
			childSetValue("raw_editor", mMaterialScript);
		}
		else
		{
			childSetValue("raw_editor", "Parsing material script failed. Try re-opening the script.");
		}
	}
}

void RexOgreScriptEditor::buildParameterControls(const LLString &param_name)
{
	const S32 HPAD = 10;
	const S32 LEFT = HPAD;
	const S32 RIGHT = getRect().getWidth() - HPAD;
	const S32 TOP = getRect().getHeight() - LLFLOATER_HEADER_SIZE - 270;
	const S32 LINE = 20;
	
	std::map<LLString, LLString>::iterator it;

	LLTextBox* TUName = NULL;			// for tu names
	LLTextBox* VPParamName = NULL;		// for vertex program parameter names
	LLTextBox* FPParamName = NULL;		// for fragment program parameter names
	LLLineEditor* TUTextureName = NULL;	// for tu texture names (uuids)
	LLLineEditor* VPParamValue = NULL;	// for vertex program parameter values
	LLLineEditor* FPParamValue = NULL;	// for fragment program parameter values
	
	mVPParamEditors.clear();
	mFPParamEditors.clear();
	mUUIDEditors.clear();

	S32 y = TOP;
	
	// Clear the old UI elements
	for(U16 i = 0; i < mUIElementList.size(); i++)
	{
		delete mUIElementList[i];
		mUIElementList[i] = NULL;
	}
	
	it = mMaterialMap.find(param_name);
	if(it == mMaterialMap.end()) return;
	
	LLString::size_type TU_found = it->first.find_last_of('#');
	LLString uuid = it->second;

	// Create textbox and line editor for texture unit name and texture name
	// If the it->second is valid uuid or it->first end with '#' then it's a tu texture name
	if(LLUUID::validate(uuid) || TU_found != it->first.npos)
	{
		LLString text = it->first.substr(0, it->first.find_last_of('#'));
		TUName = new LLTextBox(it->first, 
								text,
								90,
								LLFontGL::sSansSerifSmall,
								TRUE);
		TUName->setRect(LLRect(LEFT, y-5, LEFT+90, y-LINE));
		TUName->setFollows(FOLLOWS_BOTTOM | FOLLOWS_LEFT);
		
		addChild(TUName);
		mUIElementList.push_back(TUName);
		
		// Line editor
		TUTextureName = new LLLineEditor(it->first,
							LLRect(LEFT+90, y, RIGHT, y-20),
							it->second,
							LLFontGL::sSansSerifSmall,
							36,	// max_length_bytes
							onCommitParameterEditor,//onCommitLine,
							0,//onKeyLine,
							0); //onFocusLostParameterEditor

		TUTextureName->setHandleEditKeysDirectly(true);
		TUTextureName->setFollows(FOLLOWS_BOTTOM | FOLLOWS_LEFT);

		addChild(TUTextureName);
		mUUIDEditors.push_back(TUTextureName);
		mUIElementList.push_back(TUTextureName);		
		childSetCommitCallback(it->first, onCommitParameterEditor, this);
	}

	// Create textboxs and line editors for vertex program parameters and values
	// If it->first ends with '@' then it's a vertex program parameter name
	LLString::size_type VP_found = it->first.find_last_of('@');
	if(VP_found != it->first.npos)
	{
		LLString text = it->first.substr(0, it->first.find_last_of('@'));
		VPParamName = new LLTextBox(it->first, 
								text,
								125,
								LLFontGL::sSansSerifSmall,
								TRUE);
		VPParamName->setRect(LLRect(LEFT, y-5, LEFT+90, y-LINE));
		VPParamName->setFollows(FOLLOWS_BOTTOM | FOLLOWS_LEFT);
		
		addChild(VPParamName);
		mUIElementList.push_back(VPParamName);

		// parse parameter values from it->second
		LLString data = it->second;
		LLString::trim(data);
		LLString::size_type i = 0, j = 0;
		U16 width = 0;
		U16 space = 0;
		BOOL first_time = TRUE;
		
		while(j != LLString::npos && j < data.length())
		{
			//values are separated with space
			j = data.find(' ', i);
			LLString value = data.substr(i, j == LLString::npos ? j : j - i);
			LLString::trim(data);
			if(!value.empty())
			{
				VPParamValue = new LLLineEditor(it->first,
									LLRect(LEFT+95+width+space, y, LEFT+145+width, y-LINE), 
									value,
									LLFontGL::sSansSerifSmall,
									15,	// max_length_bytes
									onCommitParameterEditor,	//onCommitLine,
									0,	//onKeyLine,
									0); //onFocusLostParameterEditor
				VPParamValue->setFollows(FOLLOWS_BOTTOM | FOLLOWS_LEFT);
				
				addChild(VPParamValue);
				mVPParamEditors.push_back(VPParamValue);
				mUIElementList.push_back(VPParamValue);
				childSetCommitCallback(it->first, onCommitParameterEditor, this);

				if(first_time)
				{
					space += 3;
					first_time = FALSE;
				}

				width += 50;
			}

			i = j + 1;
		}
	}

	// Create textboxs and line editors for fragment program parameters and values
	// If it->first ends with '*' then it's a fragment program parameter name
	LLString::size_type FP_found = it->first.find_last_of('*');
	if(FP_found != it->first.npos)
	{
		LLString text = it->first.substr(0, it->first.find_last_of('*'));
		FPParamName = new LLTextBox(it->first, 
							text,
							100,
							LLFontGL::sSansSerifSmall,
							TRUE);
		FPParamName->setRect(LLRect(LEFT, y-5, LEFT+100, y-LINE));
		FPParamName->setFollows(FOLLOWS_BOTTOM | FOLLOWS_LEFT);

		addChild(FPParamName);
		mUIElementList.push_back(FPParamName);
		
		// parse parameter values from it->second
		LLString data = it->second;
		LLString::trim(data);
		LLString::size_type i = 0, j = 0;
		U16 width = 0;
		U16 space = 0;
		BOOL first_time = TRUE;
		
		while(j != LLString::npos && j < data.length())
		{
			//values are separated with space
			j = data.find(' ', i);
			LLString value = data.substr(i, j == LLString::npos ? j : j - i);

			LLString::trim(value);
			if(!value.empty())
			{
				FPParamValue = new LLLineEditor(it->first,
									LLRect(LEFT+110+width+space, y, LEFT+160+width, y-LINE), 
									value,
									LLFontGL::sSansSerifSmall,
									15,	//max_length_bytes
									onCommitParameterEditor,	//onCommitLine,
									0,	//onKeyLine,
									0);	//onFocusLostParameterEditor
				FPParamValue->setFollows(FOLLOWS_BOTTOM | FOLLOWS_LEFT);
				
				addChild(FPParamValue);
				mFPParamEditors.push_back(FPParamValue);
				mUIElementList.push_back(FPParamValue);
				childSetCommitCallback(it->first, onCommitParameterEditor, this);
				
				if(first_time)
				{
					space += 3;
					first_time = FALSE;
				}

				width += 50;
			}

			i = j + 1;
		}
	}	
}

BOOL RexOgreScriptEditor::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									 EDragAndDropType cargo_type,
									 void* cargo_data,
									 EAcceptance* accept,
									 LLString& tooltip_msg)
{
	BOOL handled = FALSE;

    for (unsigned i = 0; i < mUUIDEditors.size(); ++i)
    {
        LLLineEditor* editor = mUUIDEditors[i];

        if (editor->getRect().pointInRect(x,y))
        {
            if ((cargo_type == DAD_TEXTURE) || (cargo_type == DAD_OGREASSET))
            {
                LLInventoryItem* item = (LLInventoryItem*)cargo_data;
                if (item->getType() == LLAssetType::AT_TEXTURE)
                {
                    *accept = ACCEPT_YES_SINGLE;
    
                    if (drop)
                    {
						editor->setValue(item->getAssetUUID());
						onCommitParameterEditor(editor, this);
                    }	
                }
            }
        }

        handled = TRUE;
    }

    return handled;
}