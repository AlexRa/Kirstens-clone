/*
 * @file   llxmlrpclistener_test.cpp
 * @author Nat Goodspeed
 * @date   2009-03-20
 * @brief  Test for llxmlrpclistener.
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

// Precompiled header
#include "../llviewerprecompiledheaders.h"
// associated header
#include "../llxmlrpclistener.h"
// STL headers
#include <iomanip>
// std headers
// external library headers
// other Linden headers
#include "../test/lltut.h"
#include "../llxmlrpctransaction.h"
#include "llevents.h"
#include "lleventfilter.h"
#include "llsd.h"
#include "llcontrol.h"
#include "tests/wrapllerrs.h"

LLControlGroup gSavedSettings("Global");

/*****************************************************************************
*   TUT
*****************************************************************************/
namespace tut
{
    struct data
    {
        data():
            pumps(LLEventPumps::instance()),
            uri("http://127.0.0.1:8000")
        {
            // These variables are required by machinery used by
            // LLXMLRPCTransaction. The values reflect reality for this test
            // executable; hopefully these values are correct.
            gSavedSettings.declareBOOL("BrowserProxyEnabled", FALSE, "", FALSE); // don't persist
            gSavedSettings.declareBOOL("NoVerifySSLCert", TRUE, "", FALSE); // don't persist
        }

        // LLEventPump listener signature
        bool captureReply(const LLSD& r)
        {
            reply = r;
            return false;
        }

        LLSD reply;
        LLEventPumps& pumps;
        std::string uri;
    };
    typedef test_group<data> llxmlrpclistener_group;
    typedef llxmlrpclistener_group::object object;
    llxmlrpclistener_group llxmlrpclistenergrp("llxmlrpclistener");

    template<> template<>
    void object::test<1>()
    {
        set_test_name("request validation");
        WrapLL_ERRS capture;
        LLSD request;
        request["uri"] = uri;
        std::string threw;
        try
        {
            pumps.obtain("LLXMLRPCTransaction").post(request);
        }
        catch (const WrapLL_ERRS::FatalException& e)
        {
            threw = e.what();
        }
        ensure_contains("threw exception", threw, "missing params");
        ensure_contains("identified missing", threw, "method");
        ensure_contains("identified missing", threw, "reply");
    }

    template<> template<>
    void object::test<2>()
    {
        set_test_name("param types validation");
        WrapLL_ERRS capture;
        LLSD request;
        request["uri"] = uri;
        request["method"] = "hello";
        request["reply"] = "reply";
        LLSD& params(request["params"]);
        params["who"]["specifically"] = "world"; // LLXMLRPCListener only handles scalar params
        std::string threw;
        try
        {
            pumps.obtain("LLXMLRPCTransaction").post(request);
        }
        catch (const WrapLL_ERRS::FatalException& e)
        {
            threw = e.what();
        }
        ensure_contains("threw exception", threw, "unknown type");
    }

    template<> template<>
    void object::test<3>()
    {
        set_test_name("success case");
        LLSD request;
        request["uri"] = uri;
        request["method"] = "hello";
        request["reply"] = "reply";
        LLSD& params(request["params"]);
        params["who"] = "world";
        // Set up a timeout filter so we don't spin forever waiting.
        LLEventTimeout watchdog;
        // Connect the timeout filter to the reply pump.
        LLTempBoundListener temp(
            pumps.obtain("reply").
            listen("watchdog", boost::bind(&LLEventTimeout::post, boost::ref(watchdog), _1)));
        // Now connect our target listener to the timeout filter.
        watchdog.listen("captureReply", boost::bind(&data::captureReply, this, _1));
        // Kick off the request...
        reply.clear();
        pumps.obtain("LLXMLRPCTransaction").post(request);
        // Set the timer
        F32 timeout(10);
        watchdog.eventAfter(timeout, LLSD().insert("timeout", 0));
        // and pump "mainloop" until we get something, whether from
        // LLXMLRPCListener or from the watchdog filter.
        LLTimer timer;
        F32 start = timer.getElapsedTimeF32();
        LLEventPump& mainloop(pumps.obtain("mainloop"));
        while (reply.isUndefined())
        {
            mainloop.post(LLSD());
        }
        ensure("timeout works", (timer.getElapsedTimeF32() - start) < (timeout + 1));
        ensure_equals(reply["responses"]["hi_there"].asString(), "Hello, world!");
    }

    template<> template<>
    void object::test<4>()
    {
        set_test_name("bogus method");
        LLSD request;
        request["uri"] = uri;
        request["method"] = "goodbye";
        request["reply"] = "reply";
        LLSD& params(request["params"]);
        params["who"] = "world";
        // Set up a timeout filter so we don't spin forever waiting.
        LLEventTimeout watchdog;
        // Connect the timeout filter to the reply pump.
        LLTempBoundListener temp(
            pumps.obtain("reply").
            listen("watchdog", boost::bind(&LLEventTimeout::post, boost::ref(watchdog), _1)));
        // Now connect our target listener to the timeout filter.
        watchdog.listen("captureReply", boost::bind(&data::captureReply, this, _1));
        // Kick off the request...
        reply.clear();
        pumps.obtain("LLXMLRPCTransaction").post(request);
        // Set the timer
        F32 timeout(10);
        watchdog.eventAfter(timeout, LLSD().insert("timeout", 0));
        // and pump "mainloop" until we get something, whether from
        // LLXMLRPCListener or from the watchdog filter.
        LLTimer timer;
        F32 start = timer.getElapsedTimeF32();
        LLEventPump& mainloop(pumps.obtain("mainloop"));
        while (reply.isUndefined())
        {
            mainloop.post(LLSD());
        }
        ensure("timeout works", (timer.getElapsedTimeF32() - start) < (timeout + 1));
        ensure_equals("XMLRPC error", reply["status"].asString(), "XMLRPCError");
    }

    template<> template<>
    void object::test<5>()
    {
        set_test_name("bad type");
        LLSD request;
        request["uri"] = uri;
        request["method"] = "getdict";
        request["reply"] = "reply";
        (void)request["params"];
        // Set up a timeout filter so we don't spin forever waiting.
        LLEventTimeout watchdog;
        // Connect the timeout filter to the reply pump.
        LLTempBoundListener temp(
            pumps.obtain("reply").
            listen("watchdog", boost::bind(&LLEventTimeout::post, boost::ref(watchdog), _1)));
        // Now connect our target listener to the timeout filter.
        watchdog.listen("captureReply", boost::bind(&data::captureReply, this, _1));
        // Kick off the request...
        reply.clear();
        pumps.obtain("LLXMLRPCTransaction").post(request);
        // Set the timer
        F32 timeout(10);
        watchdog.eventAfter(timeout, LLSD().insert("timeout", 0));
        // and pump "mainloop" until we get something, whether from
        // LLXMLRPCListener or from the watchdog filter.
        LLTimer timer;
        F32 start = timer.getElapsedTimeF32();
        LLEventPump& mainloop(pumps.obtain("mainloop"));
        while (reply.isUndefined())
        {
            mainloop.post(LLSD());
        }
        ensure("timeout works", (timer.getElapsedTimeF32() - start) < (timeout + 1));
        ensure_equals(reply["status"].asString(), "BadType");
        ensure_contains("bad type", reply["responses"]["nested_dict"].asString(), "bad XMLRPC type");
    }
} // namespace tut

/*****************************************************************************
*   Resolve link errors: use real machinery here, since we intend to exchange
*   actual XML with a peer process.
*****************************************************************************/
// Including llxmlrpctransaction.cpp drags in the static LLXMLRPCListener
// instantiated there. That's why it works to post requests to the LLEventPump
// named "LLXMLRPCTransaction".
#include "../llxmlrpctransaction.cpp"
#include "llcontrol.cpp"
#include "llxmltree.cpp"
#include "llxmlparser.cpp"
