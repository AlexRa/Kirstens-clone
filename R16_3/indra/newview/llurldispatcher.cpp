/**
 * @file llurldispatcher.cpp
 * @brief Central registry for all URL handlers
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2008, Linden Research, Inc.
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
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
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

#include "llurldispatcher.h"

// viewer includes
#include "llagent.h"			// teleportViaLocation()
#include "llcommandhandler.h"
#include "llfloaterurldisplay.h"
#include "llfloaterdirectory.h"
#include "llfloaterhtml.h"
#include "llfloaterworldmap.h"
#include "llfloaterhtmlhelp.h"
#include "llpanellogin.h"
#include "llstartup.h"			// gStartupState
#include "llurlsimstring.h"
#include "llviewerwindow.h"		// alertXml()
#include "llweb.h"
#include "llworldmap.h"

// library includes
#include "llsd.h"

const std::string URL3DX_OL_HELP_PREFIX		= "openlife://app.";
const std::string URL3DX_OL_PREFIX			= "ol://";
const std::string URL3DX_OPENLIFE_PREFIX	= "openlife://";
const std::string URL3DX_URL3DX_PREFIX		= "http://url3DX.com/openlife/";

const std::string URL3DX_APP_TOKEN = "app/";

class LLURLDispatcherImpl
{
public:
	static bool isURL3DX(const std::string& url);

	static bool isURL3DXCommand(const std::string& url);

	static bool dispatch(const std::string& url, bool from_external_browser);
		// returns true if handled or explicitly blocked.

	static bool dispatchRightClick(const std::string& url);

private:
	static bool dispatchCore(const std::string& url, 
		bool from_external_browser, bool right_mouse);
		// handles both left and right click

	static bool dispatchHelp(const std::string& url, BOOL right_mouse);
		// Handles sl://app.floater.html.help by showing Help floater.
		// Returns true if handled.

	static bool dispatchApp(const std::string& url, bool from_external_browser, BOOL right_mouse);
		// Handles openlife:///app/agent/<agent_id>/about and similar
		// by showing panel in Search floater.
		// Returns true if handled or explicitly blocked.

	static bool dispatchRegion(const std::string& url, BOOL right_mouse);
		// handles openlife://Kai/123/45/67/
		// Returns true if handled.

	static void regionHandleCallback(U64 handle, const std::string& url,
		const LLUUID& snapshot_id, bool teleport);
		// Called by LLWorldMap when a location has been resolved to a
	    // region name

	static void regionNameCallback(U64 handle, const std::string& url,
		const LLUUID& snapshot_id, bool teleport);
		// Called by LLWorldMap when a region name has been resolved to a
		// location in-world, used by places-panel display.

	static bool matchPrefix(const std::string& url, const std::string& prefix);
	
	static std::string stripProtocol(const std::string& url);

	friend class LLTeleportHandler;
};

// static
bool LLURLDispatcherImpl::isURL3DX(const std::string& url)
{
	if (matchPrefix(url, URL3DX_OL_HELP_PREFIX)) return true;
	if (matchPrefix(url, URL3DX_OL_PREFIX)) return true;
	if (matchPrefix(url, URL3DX_OPENLIFE_PREFIX)) return true;
	if (matchPrefix(url, URL3DX_URL3DX_PREFIX)) return true;
	return false;
}

// static
bool LLURLDispatcherImpl::isURL3DXCommand(const std::string& url)
{ 
	if (matchPrefix(url, URL3DX_OL_PREFIX + URL3DX_APP_TOKEN)
		|| matchPrefix(url, URL3DX_OPENLIFE_PREFIX + "/" + URL3DX_APP_TOKEN)
		|| matchPrefix(url, URL3DX_URL3DX_PREFIX + URL3DX_APP_TOKEN) )
	{
		return true;
	}
	return false;
}

// static
bool LLURLDispatcherImpl::dispatchCore(const std::string& url, bool from_external_browser, bool right_mouse)
{
	if (url.empty()) return false;
	if (dispatchHelp(url, right_mouse)) return true;
	if (dispatchApp(url, from_external_browser, right_mouse)) return true;
	if (dispatchRegion(url, right_mouse)) return true;

	/*
	// Inform the user we can't handle this
	std::map<std::string, std::string> args;
	args["[URL3DX]"] = url;
	gViewerWindow->alertXml("BadURL", args);
	*/
	
	return false;
}

// static
bool LLURLDispatcherImpl::dispatch(const std::string& url, bool from_external_browser)
{
	llinfos << "url: " << url << llendl;
	const bool right_click = false;
	return dispatchCore(url, from_external_browser, right_click);
}

// static
bool LLURLDispatcherImpl::dispatchRightClick(const std::string& url)
{
	llinfos << "url: " << url << llendl;
	const bool from_external_browser = false;
	const bool right_click = true;
	return dispatchCore(url, from_external_browser, right_click);
}
// static
bool LLURLDispatcherImpl::dispatchHelp(const std::string& url, BOOL right_mouse)
{
#if LL_LIBXUL_ENABLED
	if (matchPrefix(url, URL3DX_OL_HELP_PREFIX))
	{
		gViewerHtmlHelp.show();
		return true;
	}
#endif
	return false;
}

// static
bool LLURLDispatcherImpl::dispatchApp(const std::string& url, 
									  bool from_external_browser,
									  BOOL right_mouse)
{
	if (!isURL3DX(url))
	{
		return false;
	}

	LLURI uri(url);
	LLSD pathArray = uri.pathArray();
	pathArray.erase(0); // erase "app"
	std::string cmd = pathArray.get(0);
	pathArray.erase(0); // erase "cmd"
	bool handled = LLCommandDispatcher::dispatch(
			cmd, from_external_browser, pathArray, uri.queryMap());
	return handled;
}

// static
bool LLURLDispatcherImpl::dispatchRegion(const std::string& url, BOOL right_mouse)
{
	if (!isURL3DX(url))
	{
		return false;
	}

	// Before we're logged in, need to update the startup screen
	// to tell the user where they are going.
	if (LLStartUp::getStartupState() < STATE_LOGIN_CLEANUP)
	{
		// Parse it and stash in globals, it will be dispatched in
		// STATE_CLEANUP.
		LLURLSimString::setString(url);
		// We're at the login screen, so make sure user can see
		// the login location box to know where they are going.
		
		LLPanelLogin::refreshLocation( true );
		return true;
	}

	std::string sim_string = stripProtocol(url);
	std::string region_name;
	S32 x = 128;
	S32 y = 128;
	S32 z = 0;
	LLURLSimString::parse(sim_string, &region_name, &x, &y, &z);

	LLFloaterURLDisplay* url_displayp = LLFloaterURLDisplay::getInstance(LLSD());
	url_displayp->setName(region_name);

	// Request a region handle by name
	LLWorldMap::getInstance()->sendNamedRegionRequest(region_name,
									  LLURLDispatcherImpl::regionNameCallback,
									  url,
									  false);	// don't teleport
	return true;
}

/*static*/
void LLURLDispatcherImpl::regionNameCallback(U64 region_handle, const std::string& url, const LLUUID& snapshot_id, bool teleport)
{
	std::string sim_string = stripProtocol(url);
	std::string region_name;
	S32 x = 128;
	S32 y = 128;
	S32 z = 0;
	LLURLSimString::parse(sim_string, &region_name, &x, &y, &z);

	LLVector3 local_pos;
	local_pos.mV[VX] = (F32)x;
	local_pos.mV[VY] = (F32)y;
	local_pos.mV[VZ] = (F32)z;

	
	// determine whether the point is in this region
	if ((x >= 0) && (x < REGION_WIDTH_UNITS) &&
		(y >= 0) && (y < REGION_WIDTH_UNITS))
	{
		// if so, we're done
		regionHandleCallback(region_handle, url, snapshot_id, teleport);
	}

	else
	{
		// otherwise find the new region from the location
		
		// add the position to get the new region
		LLVector3d global_pos = from_region_handle(region_handle) + LLVector3d(local_pos);

		U64 new_region_handle = to_region_handle(global_pos);
		LLWorldMap::getInstance()->sendHandleRegionRequest(new_region_handle,
										   LLURLDispatcherImpl::regionHandleCallback,
										   url, teleport);
	}
}

/* static */
void LLURLDispatcherImpl::regionHandleCallback(U64 region_handle, const std::string& url, const LLUUID& snapshot_id, bool teleport)
{
	std::string sim_string = stripProtocol(url);
	std::string region_name;
	S32 x = 128;
	S32 y = 128;
	S32 z = 0;
	LLURLSimString::parse(sim_string, &region_name, &x, &y, &z);

	// remap x and y to local coordinates
	S32 local_x = x % REGION_WIDTH_UNITS;
	S32 local_y = y % REGION_WIDTH_UNITS;
	if (local_x < 0)
		local_x += REGION_WIDTH_UNITS;
	if (local_y < 0)
		local_y += REGION_WIDTH_UNITS;
	
	LLVector3 local_pos;
	local_pos.mV[VX] = (F32)local_x;
	local_pos.mV[VY] = (F32)local_y;
	local_pos.mV[VZ] = (F32)z;


	
	if (teleport)
	{
		LLVector3d global_pos = from_region_handle(region_handle);
		global_pos += LLVector3d(local_pos);
		gAgent.teleportViaLocation(global_pos);
		if(gFloaterWorldMap)
		{
			gFloaterWorldMap->trackLocation(global_pos);
		}
	}
	else
	{
		// display informational floater, allow user to click teleport btn
		LLFloaterURLDisplay* url_displayp = LLFloaterURLDisplay::getInstance(LLSD());


		url_displayp->displayParcelInfo(region_handle, local_pos);
		if(snapshot_id.notNull())
		{
			url_displayp->setSnapshotDisplay(snapshot_id);
		}
		std::string locationString = llformat("%s %d, %d, %d", region_name.c_str(), x, y, z);
		url_displayp->setLocationString(locationString);
	}
}

// static
bool LLURLDispatcherImpl::matchPrefix(const std::string& url, const std::string& prefix)
{
	std::string test_prefix = url.substr(0, prefix.length());
	LLStringUtil::toLower(test_prefix);
	return test_prefix == prefix;
}

// static
std::string LLURLDispatcherImpl::stripProtocol(const std::string& url)
{
	std::string stripped = url;
	if (matchPrefix(stripped, URL3DX_OL_HELP_PREFIX))
	{
		stripped.erase(0, URL3DX_OL_HELP_PREFIX.length());
	}
	else if (matchPrefix(stripped, URL3DX_OL_PREFIX))
	{
		stripped.erase(0, URL3DX_OL_PREFIX.length());
	}
	else if (matchPrefix(stripped, URL3DX_OPENLIFE_PREFIX))
	{
		stripped.erase(0, URL3DX_OPENLIFE_PREFIX.length());
	}
	else if (matchPrefix(stripped, URL3DX_URL3DX_PREFIX))
	{
		stripped.erase(0, URL3DX_URL3DX_PREFIX.length());
	}
	return stripped;
}

//---------------------------------------------------------------------------
// Teleportation links are handled here because they are tightly coupled
// to URL parsing and sim-fragment parsing
class LLTeleportHandler : public LLCommandHandler
{
public:
	// not allowed from outside the app
	LLTeleportHandler() : LLCommandHandler("teleport", false) { }

	bool handle(const LLSD& tokens, const LLSD& queryMap)
	{
		// construct a "normal" URL3DX, resolve the region to
		// a global position, and teleport to it
		if (tokens.size() < 1) return false;

		// Region names may be %20 escaped.
		std::string region_name = LLURLSimString::unescapeRegionName(tokens[0]);

		// build openlife://Kai/123/45/67 for use in callback
		std::string url = URL3DX_OPENLIFE_PREFIX;
		for (int i = 0; i < tokens.size(); ++i)
		{
			url += tokens[i].asString() + "/";
		}
		LLWorldMap::getInstance()->sendNamedRegionRequest(region_name,
			LLURLDispatcherImpl::regionHandleCallback,
			url,
			true);	// teleport
		return true;
	}
};
LLTeleportHandler gTeleportHandler;

//---------------------------------------------------------------------------

// static
bool LLURLDispatcher::isURL3DX(const std::string& url)
{
	return LLURLDispatcherImpl::isURL3DX(url);
}

// static
bool LLURLDispatcher::isURL3DXCommand(const std::string& url)
{
	return LLURLDispatcherImpl::isURL3DXCommand(url);
}

// static
bool LLURLDispatcher::dispatch(const std::string& url, bool from_external_browser)
{
	return LLURLDispatcherImpl::dispatch(url, from_external_browser);
}
// static
bool LLURLDispatcher::dispatchRightClick(const std::string& url)
{
	return LLURLDispatcherImpl::dispatchRightClick(url);
}

// static
bool LLURLDispatcher::dispatchFromTextEditor(const std::string& url)
{
	// text editors are by definition internal to our code
	const bool from_external_browser = false;
	return LLURLDispatcherImpl::dispatch(url, from_external_browser);
}

// static
std::string LLURLDispatcher::buildURL3DX(const std::string& regionname, S32 x, S32 y, S32 z)
{
	std::string url3DX = URL3DX_URL3DX_PREFIX + regionname + llformat("/%d/%d/%d",x,y,z); 
	url3DX = LLWeb::escapeURL( url3DX );
	return url3DX;
}
