/** 
 * @file listener.cpp
 * @brief Implementation of LISTENER class abstracting the audio support
 *
 * $LicenseInfo:firstyear=2000&license=viewergpl$
 * 
 * Copyright (c) 2000-2008, Linden Research, Inc.
 * 
 */

#include "linden_common.h"

#include "listener.h"

#define DEFAULT_AT  0.0f,0.0f,-1.0f
#define DEFAULT_UP  0.0f,1.0f,0.0f

//-----------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------
LLListener::LLListener()
{
	init();
}

//-----------------------------------------------------------------------
LLListener::~LLListener()
{
}

//-----------------------------------------------------------------------
void LLListener::init(void)
{
	mPosition.zeroVec();
	mListenAt.setVec(DEFAULT_AT);
	mListenUp.setVec(DEFAULT_UP);
	mVelocity.zeroVec();
}

//-----------------------------------------------------------------------
void LLListener::translate(LLVector3 offset)
{
	mPosition += offset;
}

//-----------------------------------------------------------------------
void LLListener::setPosition(LLVector3 pos)
{
	mPosition = pos;
}

//-----------------------------------------------------------------------
LLVector3 LLListener::getPosition(void)
{
	return(mPosition);
}

//-----------------------------------------------------------------------
LLVector3 LLListener::getAt(void)
{
	return(mListenAt);
}

//-----------------------------------------------------------------------
LLVector3 LLListener::getUp(void)
{
	return(mListenUp);
}

//-----------------------------------------------------------------------
void LLListener::setVelocity(LLVector3 vel)
{
	mVelocity = vel;
}

//-----------------------------------------------------------------------
void LLListener::orient(LLVector3 up, LLVector3 at)
{
	mListenUp = up;
	mListenAt = at;
}

//-----------------------------------------------------------------------
void LLListener::set(LLVector3 pos, LLVector3 vel, LLVector3 up, LLVector3 at)
{
	mPosition = pos;
	mVelocity = vel;

	setPosition(pos);
	setVelocity(vel);
	orient(up,at);
}

//-----------------------------------------------------------------------
void LLListener::setDopplerFactor(F32 factor)
{
}

//-----------------------------------------------------------------------
F32 LLListener::getDopplerFactor()
{
	return (1.f);
}

//-----------------------------------------------------------------------
void LLListener::setDistanceFactor(F32 factor)
{
}

//-----------------------------------------------------------------------
F32 LLListener::getDistanceFactor()
{
	return (1.f);
}

//-----------------------------------------------------------------------
void LLListener::setRolloffFactor(F32 factor)
{
}

//-----------------------------------------------------------------------
F32 LLListener::getRolloffFactor()
{
	return (1.f);
}

//-----------------------------------------------------------------------
void LLListener::commitDeferredChanges()
{
}

