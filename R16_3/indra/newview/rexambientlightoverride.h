
/** 
 * @file rexambientlightoverride.h
 * @brief AmbientLightOverride header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

// rex new file

#ifndef LL_REXAMBIENTLIGHTOVERRIDE_H
#define LL_REXAMBIENTLIGHTOVERRIDE_H

//! holds world ambient light overrides. We can't define this in llogre.h, because LLVector3
//! and Ogre::Vector3 might not be defined yet when it gets included.
struct AmbientLightOverride
{
   bool mOverride;
   LLVector3 mSunDirection;
   LLColor4 mLightColour;
};

#endif // LL_REXAMBIENTLIGHTOVERRIDE_H

