/*
*  Copyright (c) 2008-2009,Kirstenlee Openlife (Lee Quick)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Openlife / 3DX nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Kirstenlee Openlife ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Kirstenlee Openlife BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// KL this is a massive simplification of similar chat command systems.
// Same thing appears in restrained life and emerald viewer.
// For now keep it simple and expand on the theme in the future :)

#include "llviewerprecompiledheaders.h"

#include "dxchatcmd.h"

#include <deque>

#include "llchatbar.h"
#include "llagent.h"
#include "stdtypes.h"
#include "llviewerregion.h"
#include "llworld.h"
#include "lluuid.h"
#include "llviewercontrol.h"

#include "material_codes.h"
#include "llvolume.h"
#include "object_flags.h"
#include "llvolumemessage.h"
#include "llurldispatcher.h"
#include "llworldmap.h"

#include <iosfwd>

#include <float.h>

#include <boost/tokenizer.hpp>

#include "llchat.h"

#include "llfloaterchat.h"

#if LL_WINDOWS 
#include "llwindebug.h"
#endif


bool dxcmd(std::string revised_text, EChatType type)
{
    
        std::istringstream i(revised_text);
		std::string command;
		i >> command;
        LLStringUtil::toLower(command);
		if(command != "")
        {
            if(command == "###test")
            {
                llinfos << "test chat dxchatcmd!" << llendl;
				return false;
            }
            else if(command == "###platform")
            {
                llinfos << "rez a platform!" << llendl;
			    LLVector3 agentPos = gAgent.getPositionAgent()+(gAgent.getVelocity()*(F32)0.333);
				LLMessageSystem* msg = gMessageSystem;
				msg->newMessageFast(_PREHASH_ObjectAdd);
				msg->nextBlockFast(_PREHASH_AgentData);
				msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
				msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
				msg->nextBlockFast(_PREHASH_ObjectData);
				msg->addU8Fast(_PREHASH_PCode, LL_PCODE_VOLUME);
				msg->addU8Fast(_PREHASH_Material,	LL_MCODE_METAL);

				if(agentPos.mV[2] > 4096.0)msg->addU32Fast(_PREHASH_AddFlags, FLAGS_CREATE_SELECTED);
				else msg->addU32Fast(_PREHASH_AddFlags, 0);

				LLVolumeParams	volume_params;

				volume_params.setType( LL_PCODE_PROFILE_SQUARE, LL_PCODE_PATH_LINE );
				volume_params.setBeginAndEndS( 0.f, 1.f );
				volume_params.setBeginAndEndT( 0.f, 1.f );
				volume_params.setRatio	( 1, 1 );
				volume_params.setShear	( 0, 0 );

				LLVolumeMessage::packVolumeParams(&volume_params, msg);
				LLVector3 rezpos = agentPos - LLVector3(0.0f,0.0f,2.5f);
				msg->addVector3Fast(_PREHASH_Scale,			LLVector3(20.0f,20.0f,0.25f) );
				msg->addQuatFast(_PREHASH_Rotation,			LLQuaternion() );
				msg->addVector3Fast(_PREHASH_RayStart,		rezpos );
				msg->addVector3Fast(_PREHASH_RayEnd,			rezpos );
				msg->addU8Fast(_PREHASH_BypassRaycast,		(U8)1 );
				msg->addU8Fast(_PREHASH_RayEndIsIntersection, (U8)FALSE );
				msg->addU8Fast(_PREHASH_State, 0);
				msg->addUUIDFast(_PREHASH_RayTargetID,			LLUUID::null );
				msg->sendReliable(gAgent.getRegionHost());
				return false;
            }
        }
     return true;
    }
    
            



    