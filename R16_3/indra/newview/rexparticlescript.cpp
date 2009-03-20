/**
 * @file rexparticlescript.cpp
 * @brief RexParticleScript class implementation
 *
 * Copyright (c) 2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"

#include "rexparticlescript.h"
#include "rexogrematerial.h"
#include "rexogretexture.h"
#include "rexogrelegacymaterial.h"
#include "llviewerimagelist.h"
#include "llogre.h"
#include "llogreassetloader.h"

std::string sEmptyName;

RexParticleScript::RexParticleScript(const LLUUID& id) :
    mID(id)
{
}

RexParticleScript::~RexParticleScript()
{
    purge();
}

//! Create Ogre particle script(s) from a buffer. Returns true if successful
bool RexParticleScript::create(U8* buffer, S32 size)
{
    bool success = false;
    int numSystems = 0;
    char* buffer2 = 0;
    LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();

    purge();

 	try
	{		
		// The original stream
        Ogre::MemoryDataStreamPtr stream(new Ogre::MemoryDataStream(buffer, size, false));   
        // Buffer to which we copy a modified script
        std::ostringstream output;

        int braceLevel = 0;
        bool skipUntilNext = false;
        int skipBraceLevel = 0;

        while (!stream->eof())
        {
            Ogre::String line = stream->getLine();
            // Skip empty lines & comments
            if ((line.length()) && (line.substr(0, 2) != "//"))
            {
                // Process opening/closing braces
                if (!LLOgreAssetLoader::processBraces(line, braceLevel))
                {
                    // If not a brace and on level 0, it should be a new particlesystem; replace name with UUID + ordinal
                    if (braceLevel == 0)
                    {
                        line = mID.asString() + "_" + Ogre::StringConverter::toString(numSystems);
                        ++numSystems;
                    }
                    else
                    {
                        // Check for ColourImage, which is a risky affector and may easily crash if image can't be loaded
                        if (line.substr(0, 8) == "affector")
                        {
                            std::vector<Ogre::String> lineVec = Ogre::StringUtil::split(line, "\t ");
                            if (lineVec.size() >= 2)
                            {
                                if (lineVec[1] == "ColourImage")
                                {
                                    skipUntilNext = true;
                                    skipBraceLevel = braceLevel;
                                }
                            }
                        }
                        // Check for UUID-based image/material definition
                        else if (line.substr(0, 8) == "material")
                        {
                            std::vector<Ogre::String> lineVec = Ogre::StringUtil::split(line, "\t ");
                            if (lineVec.size() >= 2)
                            {
                                LLUUID imageID;
                                // Script mode
                                if ((lineVec.size() >= 3) && (lineVec[2].substr(0,6) == "script"))
                                {
                                    LLUUID materialID;
                                    LLUUID::parseUUID(lineVec[1].c_str(), &materialID);
                                    if (materialID != LLUUID::null)
                                    {
                                        RexOgreMaterial* mat = assetLoader->getMaterial(materialID);
                                        if (mat)
                                        {
                                            line = "material " + mat->getName();
                                        }
                                        else
                                        {
                                            // For all objects who are waiting for this particlescript, also give notification when the material is loaded
                                            assetLoader->loadAsset(materialID, LLAssetType::AT_MATERIAL, 0, true, mID);
                                            line = "material " + materialID.asString();
                                        }
                                        mMaterials.push_back(materialID);
                                    }
                                }
                                // Image mode
                                else if (LLUUID::parseUUID(lineVec[1].c_str(), &imageID))
                                {
                                    if (imageID != LLUUID::null)
                                    {
                                        LLViewerImage* image = gImageList.getImage(imageID);
                                        if (image)
                                        {
                                            // Get Ogre legacy material associated with this image
                                            RexOgreLegacyMaterial* mat = image->getOgreMaterial();
                                            if (mat)
                                            {
                                                std::string suffix = "";
                                                // Get possible material variation
                                                if (lineVec.size() >= 3)
                                                    suffix = lineVec[2]; 

                                                // Replace material definition with Ogre material
                                                line = "material " + mat->getName() + suffix;
                                                mImages.push_back(image);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // Write line to the copy
                    if (!skipUntilNext)
                        output << line << std::endl;
                    else
                        llwarns << "Skipping risky particle effect line: " << line << llendl;
                }
                else
                {
                    // Write line to the copy
                    if (!skipUntilNext)
                        output << line << std::endl;
                    else
                        llwarns << "Skipping risky particle effect line: " << line << llendl;

                    if (braceLevel <= skipBraceLevel)
                        skipUntilNext = false;
                }
            }            
        }

        // Parse particle script(s) from modified buffer
        // Apologies for ugly way of making a buffer for Ogre to use, but using the string directly seemed not to work
        std::string outStr = output.str();
		// Save the output as string for the script editor
		mParticleString = output.str();

        if (outStr.size())
        {
            buffer2 = static_cast<char *>(malloc(outStr.size()));
            for (unsigned i = 0; i < outStr.size(); ++i)
            {
                buffer2[i] = outStr[i];
            }
            
            Ogre::MemoryDataStreamPtr stream2(new Ogre::MemoryDataStream(buffer2, outStr.size(), false));
            Ogre::DataStreamPtr castPtr = static_cast<Ogre::DataStreamPtr>(stream2);
            Ogre::ParticleSystemManager::getSingleton().parseScript(castPtr, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        }
    }
    catch (Ogre::Exception e)
	{
		llwarns << "Error creating particle scripts from asset " << mID << llendl;
    }

    // Now see if particle system templates were actually created
    for (int i = 0; i < numSystems; ++i)
    {
        std::string name = mID.asString() + "_" + Ogre::StringConverter::toString(i);
        if (Ogre::ParticleSystemManager::getSingleton().getTemplate(name))
        {
            mParticleSystems.push_back(name);
            success = true;
        }
    }

    if (buffer2) 
    {
        free(buffer2);
    }

    return success;
}

void RexParticleScript::purge()
{
    mImages.clear();
    mMaterials.clear();

    for (unsigned i = 0; i < mParticleSystems.size(); ++i)
    {
        try
        {
            Ogre::ParticleSystemManager::getSingleton().removeTemplate(mParticleSystems[i]);
        } catch (...) {}
    }
    mParticleSystems.clear();
}

void RexParticleScript::pumpImages()
{
    LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();

    for (unsigned i = 0; i < mImages.size(); ++i)
    {
        if (mImages[i].notNull())
        {
            LLViewerImage* image = mImages[i];
            image->addTextureStats(1024*1024); // Great boost
            image->resetBindTime();
        }
    }

    for (unsigned i = 0; i < mMaterials.size(); ++i)
    {
        RexOgreMaterial* mat = assetLoader->getMaterial(mMaterials[i]);
        if (mat)
        {
            mat->pumpImages(1024*1024); // Great boost
        }
    }
}

const std::string& RexParticleScript::getParticleSystemName(unsigned index) const
{
    if (index >= mParticleSystems.size())
        return sEmptyName;
    else 
        return mParticleSystems[index];
}