/**
 * @file rexparticlescript.h
 * @brief RexParticleScript class header
 *
 * Copyright (c) 2008 LudoCraft
 */

#ifndef REXPARTICLEASSET_H
#define REXPARTICLEASSET_H

#include <vector>
#include <string>

#include "llviewerimage.h"

//! Particle script loaded from server, creating one or more particle system templates. Particle systems may use textures and in the future materials
class RexParticleScript
{
public:
    //! Constructor
    RexParticleScript(const LLUUID& id);
    
    //! Destructor
    ~RexParticleScript();

    //! Creates particle template(s) from buffer.
    /*! If templates already exist, will be removed.
        \return true if successfully created
     */
    bool create(U8* buffer, S32 size);

    //! Removes all particle system templates created for this asset
    void purge();

    //! Pumps up texture stats of viewerimages' needed for this particle asset
    void pumpImages();

    //! Returns number of particle system templates
    unsigned getNumParticleSystems() const { return mParticleSystems.size(); }

    //! Returns name of a particle system (returns empty if index out of range)
    const std::string&  getParticleSystemName(unsigned index) const;

    //! Returns the vector of particle systems
    const std::vector<std::string>& getParticleSystemNames() const { return mParticleSystems; }

    //! Returns image+material count, 0 if using only inbuilt materials
    unsigned getNumImages() const { return mImages.size() + mMaterials.size(); }

    //! Returns particle script as string
	std::string getParticleString() { return mParticleString; }

protected:
    //! Asset UUID
    LLUUID mID;

    //! Images used in particle system template(s), if any
    std::vector<LLPointer<LLViewerImage> > mImages;

    //! Materials used in particle system template(s), if any
    /*! Note: UUID's stored instead of direct pointers to RexOgreMaterial's, because materials may not yet be loaded
        at creation time.
     */ 
    std::vector<LLUUID> mMaterials;

    //! Particle system templates successfully created from this asset
    std::vector<std::string> mParticleSystems;

	//! Particle script in string format for the script editor
	std::string mParticleString;
};

#endif // REXPARTICLEASSET_H