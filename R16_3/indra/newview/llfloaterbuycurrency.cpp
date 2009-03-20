/** 
 * @file llfloaterbuycurrency.cpp
 * @brief LLFloaterBuyCurrency class implementation
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2008, Linden Research, Inc.
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

#include "llfloaterbuycurrency.h"

// viewer includes
#include "llcurrencyuimanager.h"
#include "llsecondlifeurls.h"
#include "llfloater.h"
#include "llstatusbar.h"
#include "lltextbox.h"
#include "llviewchildren.h"
#include "llviewerwindow.h"
#include "lluictrlfactory.h"
#include "llweb.h"
#include "llwindow.h"
#include "llappviewer.h"

static const S32 STANDARD_BUY_AMOUNT = 1000;
static const S32 MINIMUM_BALANCE_AMOUNT = 0;

class LLFloaterBuyCurrencyUI
:	public LLFloater
{
public:
	static LLFloaterBuyCurrencyUI* soleInstance(bool createIfNeeded);

private:
	static LLFloaterBuyCurrencyUI* sInstance;

	LLFloaterBuyCurrencyUI();
	virtual ~LLFloaterBuyCurrencyUI();


public:
	LLViewChildren		mChildren;
	LLCurrencyUIManager	mManager;
	
	bool		mHasTarget;
	std::string	mTargetName;
	S32			mTargetPrice;
	
public:
	void noTarget();
	void target(const std::string& name, S32 price);
	
	virtual BOOL postBuild();
	
	void updateUI();

	virtual void draw();
	virtual BOOL canClose();
	virtual void onClose(bool app_quitting);

	static void onClickBuy(void* data);
	static void onClickCancel(void* data);
	static void onClickErrorWeb(void* data);
};


// static
LLFloaterBuyCurrencyUI* LLFloaterBuyCurrencyUI::sInstance = NULL;

// static
LLFloaterBuyCurrencyUI* LLFloaterBuyCurrencyUI::soleInstance(bool createIfNeeded)
{
	if (!sInstance  &&  createIfNeeded)
	{
		sInstance = new LLFloaterBuyCurrencyUI();

		LLUICtrlFactory::getInstance()->buildFloater(sInstance, "floater_buy_currency.xml");
		
		sInstance->center();
	}
	
	return sInstance;
}


#if LL_WINDOWS
// passing 'this' during construction generates a warning. The callee
// only uses the pointer to hold a reference to 'this' which is
// already valid, so this call does the correct thing. Disable the
// warning so that we can compile without generating a warning.
#pragma warning(disable : 4355)
#endif 
LLFloaterBuyCurrencyUI::LLFloaterBuyCurrencyUI()
:	LLFloater(std::string("Buy Credits")),
    mChildren(*this),
	mManager(*this)
{
}

LLFloaterBuyCurrencyUI::~LLFloaterBuyCurrencyUI()
{
	if (sInstance == this)
	{
		sInstance = NULL;
	}
}


void LLFloaterBuyCurrencyUI::noTarget()
{
	updateUI();
}

void LLFloaterBuyCurrencyUI::target(const std::string& name, S32 price)
{
	updateUI();
}


// virtual
BOOL LLFloaterBuyCurrencyUI::postBuild()
{
	
	updateUI();
	
	return TRUE;
}

void LLFloaterBuyCurrencyUI::draw()
{
	
		
	updateUI();
	

	LLFloater::draw();
}

BOOL LLFloaterBuyCurrencyUI::canClose()
{
	return mManager.canCancel();
}

void LLFloaterBuyCurrencyUI::onClose(bool app_quitting)
{
	LLFloater::onClose(app_quitting);
	destroy();
}

void LLFloaterBuyCurrencyUI::updateUI()
{
		
        
        childSetAction("error_web", onClickErrorWeb, this);
		childHide("step_error");
		childHide("error_message");
	
}

// static
void LLFloaterBuyCurrencyUI::onClickBuy(void* data)
{
	return;
}

// static
void LLFloaterBuyCurrencyUI::onClickCancel(void* data)
{
	LLFloaterBuyCurrencyUI* self = LLFloaterBuyCurrencyUI::soleInstance(false);
	if (self)
	{
		self->close();
	}
}

// static
void LLFloaterBuyCurrencyUI::onClickErrorWeb(void* data)
{
	LLFloaterBuyCurrencyUI* self = LLFloaterBuyCurrencyUI::soleInstance(false);
	if (self)
	{
		LLWeb::loadURLExternal( BUY_CURRENCY_URL );
		self->close();
	}
}

// static
void LLFloaterBuyCurrency::buyCurrency()
{
	if (gHideLinks)
	{
		return;
	}

	LLFloaterBuyCurrencyUI* ui = LLFloaterBuyCurrencyUI::soleInstance(true);
	ui->noTarget();
	ui->updateUI();
	ui->open();
}

void LLFloaterBuyCurrency::buyCurrency(const std::string& name, S32 price)
{
	if (gHideLinks)
	{
		LLStringUtil::format_map_t args;
		args["[NAME]"] = name;
		args["[PRICE]"] = llformat("%d", price);
		gViewerWindow->alertXml("NotEnoughCurrency", args);
		return;
	}
	
	LLFloaterBuyCurrencyUI* ui = LLFloaterBuyCurrencyUI::soleInstance(true);
	ui->target(name, price);
	ui->updateUI();
	ui->open();
}


