/** 
 * @file llfloaterpostprocess.cpp
 * @brief LLFloaterPostProcess class definition
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2010, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 * 
 */

#include "llviewerprecompiledheaders.h"

#include "llfloaterpostprocess.h"

#include "llsliderctrl.h"
#include "llcheckboxctrl.h"
#include "llnotificationsutil.h"
#include "lluictrlfactory.h"
#include "llviewerdisplay.h"
#include "llpostprocess.h"
#include "llcombobox.h"
#include "lllineeditor.h"
#include "llviewerwindow.h"


LLFloaterPostProcess::LLFloaterPostProcess(const LLSD& key)
  : LLFloater(key)
{
	//LLUICtrlFactory::getInstance()->buildFloater(this, "floater_post_process.xml");
}

LLFloaterPostProcess::~LLFloaterPostProcess()
{


}
BOOL LLFloaterPostProcess::postBuild()
{
	/// Color Filter Callbacks
	childSetCommitCallback("wmiColorFilterToggle", &LLFloaterPostProcess::onBoolToggle, (char*)"enable_color_filter");
	//childSetCommitCallback("ColorFilterGamma", &LLFloaterPostProcess::onFloatControlMoved, &(gPostProcess->tweaks.gamma()));
	childSetCommitCallback("wmiColorFilterBrightness", &LLFloaterPostProcess::onFloatControlMoved, (char*)"brightness");
	childSetCommitCallback("wmiColorFilterSaturation", &LLFloaterPostProcess::onFloatControlMoved, (char*)"saturation");
	childSetCommitCallback("wmiColorFilterContrast", &LLFloaterPostProcess::onFloatControlMoved, (char*)"contrast");

	childSetCommitCallback("wmiColorFilterBaseR", &LLFloaterPostProcess::onColorControlRMoved, (char*)"contrast_base");
	childSetCommitCallback("wmiColorFilterBaseG", &LLFloaterPostProcess::onColorControlGMoved, (char*)"contrast_base");
	childSetCommitCallback("wmiColorFilterBaseB", &LLFloaterPostProcess::onColorControlBMoved, (char*)"contrast_base");
	childSetCommitCallback("wmiColorFilterBaseI", &LLFloaterPostProcess::onColorControlIMoved, (char*)"contrast_base");

	/// Night Vision Callbacks
	childSetCommitCallback("wmiNightVisionToggle", &LLFloaterPostProcess::onBoolToggle, (char*)"enable_night_vision");
	childSetCommitCallback("wmiNightVisionBrightMult", &LLFloaterPostProcess::onFloatControlMoved, (char*)"brightness_multiplier");
	childSetCommitCallback("wmiNightVisionNoiseSize", &LLFloaterPostProcess::onFloatControlMoved, (char*)"noise_size");
	childSetCommitCallback("wmiNightVisionNoiseStrength", &LLFloaterPostProcess::onFloatControlMoved, (char*)"noise_strength");

	/// Bloom Callbacks
	childSetCommitCallback("wmiBloomToggle", &LLFloaterPostProcess::onBoolToggle, (char*)"enable_bloom");
	childSetCommitCallback("wmiBloomExtract", &LLFloaterPostProcess::onFloatControlMoved, (char*)"extract_low");
	childSetCommitCallback("wmiBloomSize", &LLFloaterPostProcess::onFloatControlMoved, (char*)"bloom_width");
	childSetCommitCallback("wmiBloomStrength", &LLFloaterPostProcess::onFloatControlMoved, (char*)"bloom_strength");

	// Effect loading and saving.
	LLComboBox* comboBox = getChild<LLComboBox>("PPEffectsCombo");
	getChild<LLButton>("PPLoadEffect")->setCommitCallback(boost::bind(&LLFloaterPostProcess::onLoadEffect, this, comboBox));
	comboBox->setCommitCallback(boost::bind(&LLFloaterPostProcess::onChangeEffectName, this, _1));

	LLLineEditor* editBox = getChild<LLLineEditor>("PPEffectNameEditor");
	getChild<LLButton>("PPSaveEffect")->setCommitCallback(boost::bind(&LLFloaterPostProcess::onSaveEffect, this, editBox));

	syncMenu();
	return TRUE;
}

// Bool Toggle
void LLFloaterPostProcess::onBoolToggle(LLUICtrl* ctrl, void* userData)
{
	char const * boolVariableName = (char const *)userData;
	
	// check the bool
	LLCheckBoxCtrl* cbCtrl = static_cast<LLCheckBoxCtrl*>(ctrl);
	gPostProcess->tweaks[boolVariableName] = cbCtrl->getValue();
}

// Float Moved
void LLFloaterPostProcess::onFloatControlMoved(LLUICtrl* ctrl, void* userData)
{
	char const * floatVariableName = (char const *)userData;
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	gPostProcess->tweaks[floatVariableName] = sldrCtrl->getValue();
}

// Color Moved
void LLFloaterPostProcess::onColorControlRMoved(LLUICtrl* ctrl, void* userData)
{
	char const * floatVariableName = (char const *)userData;
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	gPostProcess->tweaks[floatVariableName][0] = sldrCtrl->getValue();
}

// Color Moved
void LLFloaterPostProcess::onColorControlGMoved(LLUICtrl* ctrl, void* userData)
{
	char const * floatVariableName = (char const *)userData;
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	gPostProcess->tweaks[floatVariableName][1] = sldrCtrl->getValue();
}

// Color Moved
void LLFloaterPostProcess::onColorControlBMoved(LLUICtrl* ctrl, void* userData)
{
	char const * floatVariableName = (char const *)userData;
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	gPostProcess->tweaks[floatVariableName][2] = sldrCtrl->getValue();
}

// Color Moved
void LLFloaterPostProcess::onColorControlIMoved(LLUICtrl* ctrl, void* userData)
{
	char const * floatVariableName = (char const *)userData;
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	gPostProcess->tweaks[floatVariableName][3] = sldrCtrl->getValue();
}

void LLFloaterPostProcess::onLoadEffect(LLComboBox* comboBox)
{
	LLSD::String effectName(comboBox->getSelectedValue().asString());

	gPostProcess->setSelectedEffect(effectName);
	llinfos << "Loading Effect: " << effectName << llendl; // KL for testing

	syncMenu();
}

void LLFloaterPostProcess::onSaveEffect(LLLineEditor* editBox)
{
	std::string effectName(editBox->getValue().asString());

	if (gPostProcess->mAllEffects.has(effectName))
	{
		LLSD payload;
		payload["effect_name"] = effectName;
		LLNotificationsUtil::add("PPSaveEffectAlert", LLSD(), payload, boost::bind(&LLFloaterPostProcess::saveAlertCallback, this, _1, _2));
	}
	else
	{
		gPostProcess->saveEffect(effectName);
		llinfos << "Saving Effect: " << effectName << llendl; // KL for testing
		syncMenu();
	}
}

void LLFloaterPostProcess::onChangeEffectName(LLUICtrl* ctrl)
{
	// get the combo box and name
	LLLineEditor* editBox = getChild<LLLineEditor>("PPEffectNameEditor");

	// set the parameter's new name
	editBox->setValue(ctrl->getValue());
}

bool LLFloaterPostProcess::saveAlertCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);

	// if they choose save, do it.  Otherwise, don't do anything
	if (option == 0)
	{
		gPostProcess->saveEffect(notification["payload"]["effect_name"].asString());

		syncMenu();
	}
	return false;
}

void LLFloaterPostProcess::syncMenu()
{
	// add the combo box contents
	LLComboBox* comboBox = getChild<LLComboBox>("PPEffectsCombo");

	comboBox->removeall();

	LLSD::map_const_iterator currEffect;
	for(currEffect = gPostProcess->mAllEffects.beginMap();
		currEffect != gPostProcess->mAllEffects.endMap();
		++currEffect) 
	{
		comboBox->add(currEffect->first);
	}

	// set the current effect as selected.
	comboBox->selectByValue(gPostProcess->getSelectedEffect());

	/// Sync Color Filter Menu
	childSetValue("wmiColorFilterToggle", gPostProcess->tweaks.useColorFilter());
	//childSetValue("ColorFilterGamma", gPostProcess->tweaks.gamma());
	childSetValue("wmiColorFilterBrightness", gPostProcess->tweaks.brightness());
	childSetValue("wmiColorFilterSaturation", gPostProcess->tweaks.saturation());
	childSetValue("wmiColorFilterContrast", gPostProcess->tweaks.contrast());
	childSetValue("wmiColorFilterBaseR", gPostProcess->tweaks.contrastBaseR());
	childSetValue("wmiColorFilterBaseG", gPostProcess->tweaks.contrastBaseG());
	childSetValue("wmiColorFilterBaseB", gPostProcess->tweaks.contrastBaseB());
	childSetValue("wmiColorFilterBaseI", gPostProcess->tweaks.contrastBaseIntensity());
	
	/// Sync Night Vision Menu
	childSetValue("wmiNightVisionToggle", gPostProcess->tweaks.useNightVisionShader());
	childSetValue("wmiNightVisionBrightMult", gPostProcess->tweaks.brightMult());
	childSetValue("wmiNightVisionNoiseSize", gPostProcess->tweaks.noiseSize());
	childSetValue("wmiNightVisionNoiseStrength", gPostProcess->tweaks.noiseStrength());

	/// Sync Bloom Menu
	childSetValue("wmiBloomToggle", LLSD(gPostProcess->tweaks.useBloomShader()));
	childSetValue("wmiBloomExtract", gPostProcess->tweaks.extractLow());
	childSetValue("wmiBloomSize", gPostProcess->tweaks.bloomWidth());
	childSetValue("wmiBloomStrength", gPostProcess->tweaks.bloomStrength());
}
