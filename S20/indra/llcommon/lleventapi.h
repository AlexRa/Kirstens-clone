/**
 * @file   lleventapi.h
 * @author Nat Goodspeed
 * @date   2009-10-28
 * @brief  LLEventAPI is the base class for every class that wraps a C++ API
 *         in an event API
 * (see https://wiki.lindenlab.com/wiki/Incremental_Viewer_Automation/Event_API).
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

#if ! defined(LL_LLEVENTAPI_H)
#define LL_LLEVENTAPI_H

#include "lleventdispatcher.h"
#include "llinstancetracker.h"
#include <string>

/**
 * LLEventAPI not only provides operation dispatch functionality, inherited
 * from LLDispatchListener -- it also gives us event API introspection.
 * Deriving from LLInstanceTracker lets us enumerate instances.
 */
class LL_COMMON_API LLEventAPI: public LLDispatchListener,
                  public LLInstanceTracker<LLEventAPI, std::string>
{
    typedef LLDispatchListener lbase;
    typedef LLInstanceTracker<LLEventAPI, std::string> ibase;

public:
    /**
     * @param name LLEventPump name on which this LLEventAPI will listen. This
     * also serves as the LLInstanceTracker instance key.
     * @param desc Documentation string shown to a client trying to discover
     * available event APIs.
     * @param field LLSD::Map key used by LLDispatchListener to look up the
     * subclass method to invoke [default "op"].
     */
    LLEventAPI(const std::string& name, const std::string& desc, const std::string& field="op");
    virtual ~LLEventAPI();

    /// Get the string name of this LLEventAPI
    std::string getName() const { return ibase::getKey(); }
    /// Get the documentation string
    std::string getDesc() const { return mDesc; }

    /**
     * Publish only selected add() methods from LLEventDispatcher.
     * Every LLEventAPI add() @em must have a description string.
     */
    template <typename CALLABLE>
    void add(const std::string& name,
             const std::string& desc,
             CALLABLE callable,
             const LLSD& required=LLSD())
    {
        LLEventDispatcher::add(name, desc, callable, required);
    }

private:
    std::string mDesc;
};

#endif /* ! defined(LL_LLEVENTAPI_H) */
