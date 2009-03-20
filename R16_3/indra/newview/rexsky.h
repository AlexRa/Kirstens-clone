/** 
 * @file llrexsky.h
 * @brief Rex sky parameters
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

// reX new file

#ifndef LL_REXSKY_H
#define LL_REXSKY_H

//! Sky type
enum RexSkyType
{
   REXSKY_PLANE,
   REXSKY_BOX,
   REXSKY_DOME,
   REXSKY_NONE
};

//! Generic ogre sky parameters, see Ogre documentation for more info
struct RexSkyParameters
{
	std::string mMaterial;
   F32 mDistance;
   bool mDrawFirst;
   F32 mAngle;
   LLVector3 mAngleAxis;

   RexSkyParameters() : 
        mMaterial("Rex/skybox")
      , mDistance(50)
      , mDrawFirst(true)
      , mAngle(0.f)
      , mAngleAxis(0, 0, 1)
   {
   }
};

//! sky parameters for sky dome, see Ogre documentation for more info
struct RexSkydomeParameters
{
   F32 mCurvature;
   F32 mTiling;
   int mXSegments;
   int mYSegments;
   int mYSegments_keep;

   RexSkydomeParameters() : 
      mCurvature(10.f)
      , mTiling(8.f)
      , mXSegments(16)
      , mYSegments(16)
      , mYSegments_keep(-1)
   {
   }
};

#endif LL_REXSKY_H


