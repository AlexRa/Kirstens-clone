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

#ifndef LL_DXCHATCMD_H
#define LL_DXCHATCMD_H

#include "llviewerprecompiledheaders.h"

#include "llchatbar.h"
#include "llinstantmessage.h"
#include "lltransactiontypes.h"
#include "lluuid.h"
#include "stdenums.h"

bool dxcmd(std::string revised_text, EChatType type);
//
// Forward declarations
//
class LLColor4;
class LLViewerObject;
class LLInventoryObject;
class LLInventoryItem;
class LLMessageSystem;
class LLViewerRegion;

void trigger_test(LLMessageSystem *mesgsys, void **user_data);


#endif
