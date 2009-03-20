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

#include "llviewerprecompiledheaders.h"

#include "dxchatcmd.h"

#include <deque>

#include "audioengine.h" 
#include "indra_constants.h"
#include "lscript_byteformat.h"
#include "mean_collision_data.h"
#include "llfloaterbump.h"
#include "llassetstorage.h"
#include "llcachename.h"
#include "llchat.h"
#include "lldbstrings.h"
#include "lleconomy.h"
#include "llfilepicker.h"
#include "llfocusmgr.h"
#include "llfollowcamparams.h"
#include "llinstantmessage.h"
#include "llquantize.h"
#include "llregionflags.h"
#include "llregionhandle.h"
#include "llsdserialize.h"
#include "llstring.h"
#include "llteleportflags.h"
#include "lltracker.h"
#include "lltransactionflags.h"
#include "llxfermanager.h"
#include "message.h"
#include "sound_ids.h"
#include "lltimer.h"
#include "llmd5.h"
#include "llagent.h"
#include "llcallingcard.h"
#include "llconsole.h"
#include "llvieweraudio.h"
#include "llviewercontrol.h"
#include "lldrawpool.h"
#include "llfirstuse.h"
#include "llfloateractivespeakers.h"
#include "llfloaterbuycurrency.h"
#include "llfloaterbuyland.h"
#include "llfloaterchat.h"
#include "llfloatergroupinfo.h"
#include "llfloaterimagepreview.h"
#include "llfloaterland.h"
#include "llfloaterregioninfo.h"
#include "llfloaterlandholdings.h"
#include "llfloatermap.h"
#include "llurldispatcher.h"
#include "llfloatermute.h"
#include "llfloaterpostcard.h"
#include "llfloaterpreference.h"
#include "llfollowcam.h"
#include "llgroupnotify.h"
#include "llhudeffect.h"
#include "llhudeffecttrail.h"
#include "llhudmanager.h"
#include "llimpanel.h"
#include "llinventorymodel.h"
#include "llinventoryview.h"
#include "llmenugl.h"
#include "llmutelist.h"
#include "llnetmap.h"
#include "llnotify.h"
#include "llpanelgrouplandmoney.h"
#include "llselectmgr.h"
#include "llstartup.h"
#include "llsky.h"
#include "llstatenums.h"
#include "llstatusbar.h"
#include "llimview.h"
#include "lltool.h"
#include "lltoolbar.h"
#include "lltoolmgr.h"
#include "lltrans.h"
#include "llui.h"			// for make_ui_sound
#include "lluploaddialog.h"
#include "llviewercamera.h"
#include "llviewergenericmessage.h"
#include "llviewerinventory.h"
#include "llviewermenu.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelmgr.h"
#include "llviewerpartsource.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewertexteditor.h"
#include "llviewerthrottle.h"
#include "llviewerwindow.h"
#include "llvlmanager.h"
#include "llvoavatar.h"
#include "llvotextbubble.h"
#include "llweb.h"
#include "llworld.h"
#include "pipeline.h"
#include "llappviewer.h"
#include "llfloaterworldmap.h"
#include "llkeythrottle.h"
#include "llviewerdisplay.h"
#include "llkeythrottle.h"

#include <boost/tokenizer.hpp>

#if LL_WINDOWS // For Windows specific error handler
#include "llwindebug.h"	// For the invalid message handler
#endif



void trigger_test(LLMessageSystem *msg, void **user_data)
{
	LLChat		chat;
	std::string		mesg;
	std::string		from_name;
	U8			source_temp;
	U8			type_temp;
	U8			audible_temp;

	LLUUID		from_id;

	LLViewerObject*	chatter;

	msg->getString("ChatData", "FromName", from_name);
	chat.mFromName = from_name;

	msg->getUUID("ChatData", "SourceID", from_id);
	chat.mFromID = from_id;

	msg->getU8Fast(_PREHASH_ChatData, _PREHASH_SourceType, source_temp);
	chat.mSourceType = (EChatSourceType)source_temp;

	msg->getU8("ChatData", "ChatType", type_temp);
	chat.mChatType = (EChatType)type_temp;

	msg->getU8Fast(_PREHASH_ChatData, _PREHASH_Audible, audible_temp);
	chat.mAudible = (EChatAudible)audible_temp;
    chatter = gObjectList.findObject(from_id);
	// BOOL is_audible = (CHAT_AUDIBLE_FULLY == chat.mAudible);

	if(chatter)
	{

        if (chat.mSourceType == CHAT_SOURCE_OBJECT 
			&& chat.mChatType != CHAT_TYPE_DEBUG_MSG)
		 {

			 llinfos << "trigger test chat dxchatcmd module!" << llendl;

		 }
	}
	
	
}	
    