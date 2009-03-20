/**
 * @file rexogrematerial.h
 * @brief RexOgreMaterial class header file
 *
 * Copyright (c) 2008 LudoCraft
 */

#ifndef REXOGREMATERIAL_H
#define REXOGREMATERIAL_H

#include <OGRE/Ogre.h>
#include <string>
#include <vector>

#include "llviewerimage.h"

//! An "advanced" material, which may contain multiple textures created from LLViewerImages, and shaders. Shaders so far local-only.
class RexOgreMaterial
{
public:
    //! Constructor
    /*!
        \param name Name of the material
        \param id UUID of the asset (optional)
    */
    RexOgreMaterial(const std::string &name, const LLUUID& id = LLUUID::null);

    //! Destructor
    ~RexOgreMaterial();

    //! Creates material from buffer. Destroys old material if already exists.
    /*! \return true if successfully created
     */
    bool create(U8* buffer, S32 size);

    //! Pump up the material's LLViewerImages
    void pumpImages(F32 totalSize);

    //! Returns Ogre material
    Ogre::MaterialPtr getMaterial() { return mMaterial; }

    //! Returns name, will be same as the Ogre material's name
    const std::string& getName() const { return mName; }

    //! Returns ID
    const LLUUID& getID() const { return mID; }

    //! Returns if material successfully created
    bool isLoaded() const { return !mMaterial.isNull(); }

	 //! @return True if the material is both loaded and usable in the rendering pipeline (has at least one usable technique).
	 //! Currently causes loading of materials and textures as well. This should be refactored at some point. (Figure out who's responsible of loading/compiling materials.)
	 bool isUsable();// const;

protected:
    //! Destroys material
    void destroyMaterial();

    //! Material name
    std::string mName;

    //! Possible ID 
    LLUUID mID;

    //! Pointer to Ogre material
    Ogre::MaterialPtr mMaterial;

    //! Images used in this material, if any
    std::vector<LLPointer<LLViewerImage> > mImages;
};

#endif // REXOGREMATERIAL_H
