/**
 * @file RexOgreMaterialeffect.cpp
 * @brief Ogre material effect classes.
 *
 * Includes definitions for various Ogre material effects, such as additive blending
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#ifndef REXOGREMATERIALEFFECT_H
#define REXOGREMATERIALEFFECT_H

#ifdef LL_LINUX
#undef Status
#endif

#include <OGRE/Ogre.h>

//! Interface for ogre material effect classes
/*! Not abstract class, since we may need an instance for default material
*/
class RexOgreMaterialeffect
{
public:
    RexOgreMaterialeffect() { }//: mEnabled(false) { }

    RexOgreMaterialeffect(const std::string& suffix) : mSuffix(suffix) { }//: mEnabled(false) { }

   //! destructor
   virtual ~RexOgreMaterialeffect() {}

   //! Enable this material effect on the specified material
   /*!
       \param material Ogre material
   */
   virtual void enable(Ogre::MaterialPtr material) { mEnabled[material->getName()] = true; };

   //! Disable this material effect on the specified material
   /*!
       \param material Ogre material
   */
   virtual void disable(Ogre::MaterialPtr material) { mEnabled[material->getName()] = false; };

   virtual bool isEnabled(Ogre::MaterialPtr material)
   {
      const std::string &name = material->getName();
      if (mEnabled.find(name) == mEnabled.end())
         mEnabled[name] = false;

      return mEnabled[name];
   }
 
   //! Get identifying suffix
   virtual const std::string& getSuffix() const { return mSuffix; }

private:
   std::string mSuffix;
   std::map<std::string, bool> mEnabled;

};

class RexOgreAdditiveBlending : public RexOgreMaterialeffect
{
public:
   //! default constructor
   RexOgreAdditiveBlending() : RexOgreMaterialeffect("add") {}

   virtual ~RexOgreAdditiveBlending() {}

   virtual void enable(Ogre::MaterialPtr material);

   virtual void disable(Ogre::MaterialPtr material);

private:
   //! Default scene blend type for material
   std::map<std::string, Ogre::SceneBlendType> mBlendType;
};

class RexOgreAdditiveBlendingNoDepth : public RexOgreMaterialeffect
{
public:
   //! default constructor
   RexOgreAdditiveBlendingNoDepth() : RexOgreMaterialeffect("addnodepth") {}

   virtual ~RexOgreAdditiveBlendingNoDepth() {}

   virtual void enable(Ogre::MaterialPtr material);

   virtual void disable(Ogre::MaterialPtr material);

private:
   //! Default scene blend type for material
   std::map<std::string, Ogre::SceneBlendType> mBlendType;
};

#endif // REXOGREMATERIALEFFECT_H


