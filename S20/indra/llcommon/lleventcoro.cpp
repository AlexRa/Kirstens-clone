/**
 * @file   lleventcoro.cpp
 * @author Nat Goodspeed
 * @date   2009-04-29
 * @brief  Implementation for lleventcoro.
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
#include "linden_common.h"
// associated header
#include "lleventcoro.h"
// STL headers
#include <map>
// std headers
// external library headers
// other Linden headers
#include "llsdserialize.h"
#include "llerror.h"
#include "llcoros.h"

std::string LLEventDetail::listenerNameForCoroImpl(const void* self_id)
{
    // First, if this coroutine was launched by LLCoros::launch(), find that name.
    std::string name(LLCoros::instance().getNameByID(self_id));
    if (! name.empty())
    {
        return name;
    }
    // Apparently this coroutine wasn't launched by LLCoros::launch(). Check
    // whether we have a memo for this self_id.
    typedef std::map<const void*, std::string> MapType;
    static MapType memo;
    MapType::const_iterator found = memo.find(self_id);
    if (found != memo.end())
    {
        // this coroutine instance has called us before, reuse same name
        return found->second;
    }
    // this is the first time we've been called for this coroutine instance
    name = LLEventPump::inventName("coro");
    memo[self_id] = name;
    LL_INFOS("LLEventCoro") << "listenerNameForCoroImpl(" << self_id << "): inventing coro name '"
                            << name << "'" << LL_ENDL;
    return name;
}

void LLEventDetail::storeToLLSDPath(LLSD& dest, const LLSD& rawPath, const LLSD& value)
{
    if (rawPath.isUndefined())
    {
        // no-op case
        return;
    }

    // Arrange to treat rawPath uniformly as an array. If it's not already an
    // array, store it as the only entry in one.
    LLSD path;
    if (rawPath.isArray())
    {
        path = rawPath;
    }
    else
    {
        path.append(rawPath);
    }

    // Need to indicate a current destination -- but that current destination
    // needs to change as we step through the path array. Where normally we'd
    // use an LLSD& to capture a subscripted LLSD lvalue, this time we must
    // instead use a pointer -- since it must be reassigned.
    LLSD* pdest = &dest;

    // Now loop through that array
    for (LLSD::Integer i = 0; i < path.size(); ++i)
    {
        if (path[i].isString())
        {
            // *pdest is an LLSD map
            pdest = &((*pdest)[path[i].asString()]);
        }
        else if (path[i].isInteger())
        {
            // *pdest is an LLSD array
            pdest = &((*pdest)[path[i].asInteger()]);
        }
        else
        {
            // What do we do with Real or Array or Map or ...?
            // As it's a coder error -- not a user error -- rub the coder's
            // face in it so it gets fixed.
            LL_ERRS("lleventcoro") << "storeToLLSDPath(" << dest << ", " << rawPath << ", " << value
                                   << "): path[" << i << "] bad type " << path[i].type() << LL_ENDL;
        }
    }

    // Here *pdest is where we should store value.
    *pdest = value;
}

LLSD errorException(const LLEventWithID& result, const std::string& desc)
{
    // If the result arrived on the error pump (pump 1), instead of
    // returning it, deliver it via exception.
    if (result.second)
    {
        throw LLErrorEvent(desc, result.first);
    }
    // That way, our caller knows a simple return must be from the reply
    // pump (pump 0).
    return result.first;
}

LLSD errorLog(const LLEventWithID& result, const std::string& desc)
{
    // If the result arrived on the error pump (pump 1), log it as a fatal
    // error.
    if (result.second)
    {
        LL_ERRS("errorLog") << desc << ":" << std::endl;
        LLSDSerialize::toPrettyXML(result.first, LL_CONT);
        LL_CONT << LL_ENDL;
    }
    // A simple return must therefore be from the reply pump (pump 0).
    return result.first;
}
