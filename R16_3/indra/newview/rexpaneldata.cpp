/** 
 * @file rexpaneldata.cpp
 * @brief RexPanelData class implementation
 *
 * Copyright (c) 2008 LudoCraft
 */

// Includes
#include "llviewerprecompiledheaders.h"

// file include
#include "rexpaneldata.h"

#include <utility> // for std::pair<>

#include "llselectmgr.h"
#include "llvovolume.h"
#include "llviewergenericmessage.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llfocusmgr.h"

#include "audioengine.h"
#include "llagent.h"          // gAgent

LLUUID mCurrentASPID;

// Externs
extern LLDispatcher gGenericDispatcher;
extern LLAudioEngine* gAudiop;
extern LLAgent gAgent;

RexDataDispatchHandler RexPanelData::smDispatchRexData;

const LLString RexDataDispatchHandler::key("RexData");
char const PARAMETER_SEPARATOR = '\n';
char const VALUE_SEPARATOR = ' ';

std::map<LLUUID, std::string> RexPanelData::sQueuedRexData;

///////////////////////////////////////////////////////////////////////
//	RexDataDispatchHandler
//	Handler for RexData generic messages
///////////////////////////////////////////////////////////////////////

bool RexDataDispatchHandler::operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
		const LLDispatchHandler::sparam_t& string)
{
	U32 size = string.size();
	LLUUID uuid(string[0]);
	LLString data;
	
	if(size < 2)
	{
		llwarns << "Received too short RexData message"<<llendl;
		return false;
	}
	else if(size == 2)
	{		
		data = string[1];
	}
	else
	{	
		for(U32 i = 1; i < size; i++)
		{
			data.append(string[i]);
		}	
	}

	LLViewerObject *objectp = gObjectList.findObject(uuid);
	
	if(objectp)
	{
        if (objectp->getPCode() != LL_PCODE_VOLUME)
            return false;
		
		LLVector3d obj_pos_global = objectp->getPositionGlobal();
		LLVOVolume *volobjp = (LLVOVolume *)objectp;
		volobjp->setRexData(data);
		
		RexPanelData::processRexParameterMap(parseRexData(data), obj_pos_global);
	}
	else
	{
		llwarns << "RexData for object " << uuid << " received before object, queuing" << llendl;
        RexPanelData::queueRexData(uuid, data);
	}

	return true;
}

std::map<LLString, LLString> RexDataDispatchHandler::parseRexData(LLString data)
{
	std::map<LLString, LLString> parameter_map;
	std::map<LLString, LLString>::iterator map_it;

	// Trim the data
	LLString::trim(data);
	LLString::toLower(data);

	LLString::size_type i = 0, j = 0;
	
	// Separate the parameter names and values from the dataline
	while(j != LLString::npos && j < data.length())
	{
		j = data.find(PARAMETER_SEPARATOR, i);
		LLString dataline = data.substr(i, j == LLString::npos ? j : j - i);
		LLString::trim(dataline);

		if(!dataline.empty())
		{		
			LLString::size_type k = 0, l = 0;
			
			l = dataline.find(VALUE_SEPARATOR, k);
			LLString parameter = dataline.substr(k, l == LLString::npos ? l : l - k);
			LLString value;
			
			// If value exists, store it
			if(l != LLString::npos)
			{
				value = dataline.substr(l, LLString::npos);
				LLString::trim(value);
			}
			// If not, the parameter is used to identify the type of the RexData
			else
			{
				value = parameter;
				parameter = "__type";
			}

			LLString::trim(parameter);

			if(!parameter.empty())
			{
				parameter_map.insert(std::pair<LLString, LLString>(parameter, value));
			}
		}
		i = j + 1;
	}
	return parameter_map;
}

///////////////////////////////////////////////////////////////////////
//	RexPanelData
///////////////////////////////////////////////////////////////////////

RexPanelData::RexPanelData(const std::string& name)
	:	LLPanel(name)
{
	setMouseOpaque(FALSE);
}

// virtual
RexPanelData::~RexPanelData()
{
	// Children all cleaned up by default view destructor.
}

// virtual
BOOL RexPanelData::postBuild()
{	
    this->childSetAction("button_set", onClickSetRexData, this);
	
	childSetEnabled("RexEditor_rexdata", TRUE);
	childEnable("button_set");

	refresh();

	return TRUE;
}

// virtual
void RexPanelData::clearCtrls()
{
	LLPanel::clearCtrls();

	childDisable("button_set");
    childDisable("RexEditor_rexdata");
}

// virtual
void RexPanelData::refresh()
{	
	LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getFirstObject();
	
	if(!objectp)
	{
		//forfeit focus
		if (gFocusMgr.childHasKeyboardFocus(this))
		{
			gFocusMgr.setKeyboardFocus(NULL, NULL);
		}

		// Disable all text input fields
		clearCtrls();

		return;
	}

	mObject = objectp;

	// Verify that we've selected only a single volume object and it is editable
	BOOL editable = objectp->permModify();
	BOOL single_volume = objectp->getPCode() == LL_PCODE_VOLUME && LLSelectMgr::getInstance()->selectionAllPCode( LL_PCODE_VOLUME )
		&& LLSelectMgr::getInstance()->getSelection()->getObjectCount() == 1;
	if ((!editable) || (!single_volume))
	{
		clearCtrls();
		return;
	}

	LLVOVolume *volobjp = (LLVOVolume *)objectp;

	// Get the actual values
	childEnable("button_set");
	childEnable("RexEditor_rexdata");
	childSetValue("RexEditor_rexdata", volobjp->getRexData());
}

// static
void RexPanelData::initClass()
{
	gGenericDispatcher.addHandler(smDispatchRexData.getKey(), &smDispatchRexData);
	llinfos<<"RexDataHandler added"<<llendl;
}

// static
void RexPanelData::onClickSetRexData(void* userdata)
{
	RexPanelData* self = (RexPanelData*) userdata;
	
	LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getFirstObject();

	if(objectp)
	{
		LLVOVolume *volobjp = (LLVOVolume *)objectp;

		std::vector<std::string> strings;
		LLUUID obj_id = objectp->getID();
		LLString data = self->childGetValue("RexEditor_rexdata").asString();
		LLString ID_as_string = obj_id.asString();
		
		// Push the UUID to the strings vector
		strings.push_back(ID_as_string);
		
		if(data.size() > 200)
		{
			// If data string's size is more than 200, generate substrings
			for(LLString::size_type i = 0; i < data.size(); i += 200)
			{
				LLString data_substr = data.substr(i, 200);
				// Push the data substring to the strings vector
				strings.push_back(data_substr);
			}
		}
		else
		{
			// No substrings needed; just one data string
			strings.push_back(data);
		}
		
		// Verify that we've selected only a single volume object and it is editable
		BOOL editable = objectp->permModify();
		BOOL single_volume = objectp->getPCode() == LL_PCODE_VOLUME && LLSelectMgr::getInstance()->selectionAllPCode(LL_PCODE_VOLUME)
			&& LLSelectMgr::getInstance()->getSelection()->getObjectCount() == 1;
			
		if (editable && single_volume)
		{
			//Don't send identical data
			if(data != volobjp->getRexData())
			{
				send_generic_message("RexData", strings);
			}
		}
	}
}

// static
void RexPanelData::queueRexData(const LLUUID& id, const std::string& src)
{
    sQueuedRexData[id] = src;
}

// static
bool RexPanelData::getQueuedRexData(const LLUUID& id, std::string& dest)
{
    std::map<LLUUID, std::string>::iterator i = sQueuedRexData.find(id);
    if (i == sQueuedRexData.end())
        return false;
    
    dest = i->second;
    sQueuedRexData.erase(i);
    return true;
  
    dest = i->second;
    sQueuedRexData.erase(i);
    return true;
}

// static
BOOL RexPanelData::processRexParameterMap(std::map<LLString, LLString> parameter_map, const LLVector3d& obj_pos_global)
{
	std::map<LLString, LLString>::iterator it;
	it = parameter_map.find("__type");
	
	if(it != parameter_map.end())
	{	
		LLString type = it->second;
		if(type == "rexsoundloop")
		{
			LLUUID asset_uuid = LLUUID::null;
			BOOL uuid_set = FALSE;
			BOOL loop = FALSE;
			F32 gain = 1.0f;
			F32 radius = 0;
			//F32 duration = 0; ignore for now

			// Get uuid
			it = parameter_map.find("assetuuid");
			if(it != parameter_map.end())
			{
				asset_uuid.set(it->second);
				uuid_set = TRUE;	
			}
			
			// Get radius
			it = parameter_map.find("radius");
			if(it != parameter_map.end())
			{
				radius = atof(it->second.c_str());
			}

			// Get loop boolean
			it = parameter_map.find("loop");
			if(it != parameter_map.end())
			{
				LLString::size_type found = it->second.find("true", 0);
				if(found != LLString::npos) loop = TRUE;
				else loop = FALSE;
			}
			
			// Get gain
			it = parameter_map.find("gain");
			if(it != parameter_map.end())
			{
				gain = atof(it->second.c_str());
			}
			
			/* Duration disabled for now
			// Get duration
			it = parameter_map.find("duration");
			if(it != parameter_map.end())
			{
				duration = atof(it->second.c_str());
			}
			*/

			if(uuid_set)
			{
				if(asset_uuid == LLUUID::null)
				{
					// Delete the audio source if uuid is null
					LLAudioSource *temp_asp = gAudiop->findAudioSource(mCurrentASPID);
					if(temp_asp)
					{
						gAudiop->cleanupAudioSource(temp_asp);
					}
				}
				else
				{
					// Add a new audio source
					if(gAudiop)
					{								
						LLUUID source_id;
						source_id.generate();

						LLAudioSource *asp = new LLAudioSource(source_id, gAgent.getID(), gain);
						gAudiop->addAudioSource(asp);

						asp->setRexSound(TRUE);
						asp->setRexSoundID(asset_uuid);
						
						/*if(duration > 0 && loop)
						{
							asp->defineDuration(duration);
							asp->setDuration(TRUE);
						}
						else
						{
							asp->defineDuration(0);
							asp->setDuration(FALSE);
						}*/
						
						if (radius == 0 && loop)
						{
							// Ambient sound loop
							asp->setAmbient(TRUE);
							asp->setRadius(FALSE);
							//asp->defineRadius(0);
							asp->setRexLoop(loop);
						}
						
						/*else if (radius == 0 && !loop)
						{
							//Ambient sound
							llinfos<<"Ambient sound created"<<llendl;
							asp->setAmbient(TRUE);
							asp->setRadius(FALSE);
							//asp->defineRadius(0);
							asp->setRexLoop(loop);
							asp->setRexPlayOnce(TRUE);
						}*/

						else if (radius > 0 && loop)
						{
							// Location based sound loop
							asp->setPositionGlobal(obj_pos_global);
							asp->setRadius(TRUE);
							asp->defineRadius(radius);
							asp->setRexLoop(loop);
						}
						else if(radius > 0 && !loop)
						{
							// Location based sound
							asp->setPositionGlobal(obj_pos_global);
							asp->setRadius(TRUE);
							asp->defineRadius(radius);
							asp->setRexLoop(loop);
							asp->setRexPlayOnce(TRUE);
						}
						else
						{
							llinfos<<"Unknow RexSoundLoop type"<<llendl;
						}

						mCurrentASPID = source_id;
					}					
				}
				return TRUE;
			}
			else
			{
				llinfos<<"Not enough parameters for RexSoundLoop!"<<llendl;
				return FALSE;
			}
		}
		return FALSE;
	}
	else
	{
		llinfos<<"Uknown RexData type"<<llendl;
		return FALSE;
	}
}