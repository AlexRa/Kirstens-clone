/**
 * @file   llviewercontrollistener.cpp
 * @author Brad Kittenbrink
 * @date   2009-07-09
 * @brief  Implementation for llviewercontrollistener.
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

#include "llviewerprecompiledheaders.h"

#include "llviewercontrollistener.h"

#include "llviewercontrol.h"

LLViewerControlListener gSavedSettingsListener;

LLViewerControlListener::LLViewerControlListener()
	: LLEventAPI("LLViewerControl",
                 "LLViewerControl listener: set, toggle or set default for various controls",
                 "group")
{
	add("Global",
        "Set gSavedSettings control [\"key\"] to value [\"value\"]",
        boost::bind(&LLViewerControlListener::set, &gSavedSettings, _1));
	add("PerAccount",
        "Set gSavedPerAccountSettings control [\"key\"] to value [\"value\"]",
        boost::bind(&LLViewerControlListener::set, &gSavedPerAccountSettings, _1));
	add("Warning",
        "Set gWarningSettings control [\"key\"] to value [\"value\"]",
        boost::bind(&LLViewerControlListener::set, &gWarningSettings, _1));
	add("Crash",
        "Set gCrashSettings control [\"key\"] to value [\"value\"]",
        boost::bind(&LLViewerControlListener::set, &gCrashSettings, _1));

#if 0
	add(/*"toggleControl",*/ "Global", boost::bind(&LLViewerControlListener::toggleControl, &gSavedSettings, _1));
	add(/*"toggleControl",*/ "PerAccount", boost::bind(&LLViewerControlListener::toggleControl, &gSavedPerAccountSettings, _1));
	add(/*"toggleControl",*/ "Warning", boost::bind(&LLViewerControlListener::toggleControl, &gWarningSettings, _1));
	add(/*"toggleControl",*/ "Crash", boost::bind(&LLViewerControlListener::toggleControl, &gCrashSettings, _1));

	add(/*"setDefault",*/ "Global", boost::bind(&LLViewerControlListener::setDefault, &gSavedSettings, _1));
	add(/*"setDefault",*/ "PerAccount", boost::bind(&LLViewerControlListener::setDefault, &gSavedPerAccountSettings, _1));
	add(/*"setDefault",*/ "Warning", boost::bind(&LLViewerControlListener::setDefault, &gWarningSettings, _1));
	add(/*"setDefault",*/ "Crash", boost::bind(&LLViewerControlListener::setDefault, &gCrashSettings, _1));
#endif // 0
}

//static
void LLViewerControlListener::set(LLControlGroup * controls, LLSD const & event_data)
{
	if(event_data.has("key"))
	{
		std::string key(event_data["key"]);

		if(controls->controlExists(key))
		{
			controls->setUntypedValue(key, event_data["value"]);
		}
		else
		{
			llwarns << "requested unknown control: \"" << key << '\"' << llendl;
		}
	}
}

//static
void LLViewerControlListener::toggleControl(LLControlGroup * controls, LLSD const & event_data)
{
	if(event_data.has("key"))
	{
		std::string key(event_data["key"]);

		if(controls->controlExists(key))
		{
			LLControlVariable * control = controls->getControl(key);
			if(control->isType(TYPE_BOOLEAN))
			{
				control->set(!control->get().asBoolean());
			}
			else
			{
				llwarns << "requested toggle of non-boolean control: \"" << key << "\", type is " << control->type() << llendl;
			}
		}
		else
		{
			llwarns << "requested unknown control: \"" << key << '\"' << llendl;
		}
	}
}

//static
void LLViewerControlListener::setDefault(LLControlGroup * controls, LLSD const & event_data)
{
	if(event_data.has("key"))
	{
		std::string key(event_data["key"]);

		if(controls->controlExists(key))
		{
			LLControlVariable * control = controls->getControl(key);
			control->resetToDefault();
		}
		else
		{
			llwarns << "requested unknown control: \"" << key << '\"' << llendl;
		}
	}
}
