/**
 * @file   llsdmessage.cpp
 * @author Nat Goodspeed
 * @date   2008-10-31
 * @brief  Implementation for llsdmessage.
 * 
 * $LicenseInfo:firstyear=2008&license=viewergpl$
 * 
 * Copyright (c) 2008-2010, Linden Research, Inc.
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

#if LL_WINDOWS
#pragma warning (disable : 4675) // "resolved by ADL" -- just as I want!
#endif

// Precompiled header
#include "linden_common.h"
// associated header
#include "llsdmessage.h"
// STL headers
// std headers
// external library headers
// other Linden headers
#include "llevents.h"
#include "llsdserialize.h"
#include "llhttpclient.h"
#include "llmessageconfig.h"
#include "llhost.h"
#include "message.h"
#include "llsdutil.h"

// Declare a static LLSDMessage instance to ensure that we have a listener as
// soon as someone tries to post on our canonical LLEventPump name.
static LLSDMessage httpListener;

LLSDMessage::LLSDMessage():
    // Instantiating our own local LLEventPump with a string name the
    // constructor is NOT allowed to tweak is a way of ensuring Singleton
    // semantics: attempting to instantiate a second LLSDMessage object would
    // throw LLEventPump::DupPumpName.
    mEventPump("LLHTTPClient")
{
    mEventPump.listen("self", boost::bind(&LLSDMessage::httpListener, this, _1));
}

bool LLSDMessage::httpListener(const LLSD& request)
{
    // Extract what we want from the request object. We do it all up front
    // partly to document what we expect.
    LLSD::String url(request["url"]);
    LLSD payload(request["payload"]);
    LLSD::String reply(request["reply"]);
    LLSD::String error(request["error"]);
    LLSD::Real timeout(request["timeout"]);
    // If the LLSD doesn't even have a "url" key, we doubt it was intended for
    // this listener.
    if (url.empty())
    {
        std::ostringstream out;
        out << "request event without 'url' key to '" << mEventPump.getName() << "'";
        throw ArgError(out.str());
    }
    // Establish default timeout. This test relies on LLSD::asReal() returning
    // exactly 0.0 for an undef value.
    if (! timeout)
    {
        timeout = HTTP_REQUEST_EXPIRY_SECS;
    }
    LLHTTPClient::post(url, payload,
                       new LLSDMessage::EventResponder(LLEventPumps::instance(),
                                                       request,
                                                       url, "POST", reply, error),
                       LLSD(),      // headers
                       timeout);
    return false;
}

void LLSDMessage::EventResponder::result(const LLSD& data)
{
    // If our caller passed an empty replyPump name, they're not
    // listening: this is a fire-and-forget message. Don't bother posting
    // to the pump whose name is "".
    if (! mReplyPump.empty())
    {
        LLSD response(data);
        mReqID.stamp(response);
        mPumps.obtain(mReplyPump).post(response);
    }
    else                            // default success handling
    {
        LL_INFOS("LLSDMessage::EventResponder")
            << "'" << mMessage << "' to '" << mTarget << "' succeeded"
            << LL_ENDL;
    }
}

void LLSDMessage::EventResponder::errorWithContent(U32 status, const std::string& reason, const LLSD& content)
{
    // If our caller passed an empty errorPump name, they're not
    // listening: "default error handling is acceptable." Only post to an
    // explicit pump name.
    if (! mErrorPump.empty())
    {
        LLSD info(mReqID.makeResponse());
        info["target"]  = mTarget;
        info["message"] = mMessage;
        info["status"]  = LLSD::Integer(status);
        info["reason"]  = reason;
        info["content"] = content;
        mPumps.obtain(mErrorPump).post(info);
    }
    else                        // default error handling
    {
        // convention seems to be to use llinfos, but that seems a bit casual?
        LL_WARNS("LLSDMessage::EventResponder")
            << "'" << mMessage << "' to '" << mTarget
            << "' failed with code " << status << ": " << reason << '\n'
            << ll_pretty_print_sd(content)
            << LL_ENDL;
    }
}

LLSDMessage::ResponderAdapter::ResponderAdapter(LLHTTPClient::ResponderPtr responder,
                                                const std::string& name):
    mResponder(responder),
    mReplyPump(name + ".reply", true), // tweak name for uniqueness
    mErrorPump(name + ".error", true)
{
    mReplyPump.listen("self", boost::bind(&ResponderAdapter::listener, this, _1, true));
    mErrorPump.listen("self", boost::bind(&ResponderAdapter::listener, this, _1, false));
}

bool LLSDMessage::ResponderAdapter::listener(const LLSD& payload, bool success)
{
    if (success)
    {
        mResponder->result(payload);
    }
    else
    {
        mResponder->errorWithContent(payload["status"].asInteger(), payload["reason"], payload["content"]);
    }

    /*---------------- MUST BE LAST STATEMENT BEFORE RETURN ----------------*/
    delete this;
    // Destruction of mResponder will usually implicitly free its referent as well
    /*------------------------- NOTHING AFTER THIS -------------------------*/
    return false;
}

void LLSDMessage::link()
{
}
