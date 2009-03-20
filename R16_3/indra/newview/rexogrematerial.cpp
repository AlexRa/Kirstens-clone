/**
 * @file rexogrematerial.cpp
 * @brief RexOgreMaterial class implementation
 *
 * Copyright (c) 2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"

#include "rexogrematerial.h"
#include "rexogretexture.h"
#include "llviewerimagelist.h"
#include "llogre.h"
#include "llogreassetloader.h"

RexOgreMaterial::RexOgreMaterial(const std::string& name, const LLUUID& id) :
    mName(name),
    mID(id)
{
}

RexOgreMaterial::~RexOgreMaterial()
{
    destroyMaterial();
}

void RexOgreMaterial::destroyMaterial()
{
    if (mMaterial.isNull())
        return;
    
    LLOgreRenderer* renderer = LLOgreRenderer::getPointer();
    if (!renderer->isInitialized())
        return;

    renderer->setOgreContext();

    Ogre::ResourcePtr castPtr;

    castPtr = static_cast<Ogre::ResourcePtr>(mMaterial);
    Ogre::MaterialManager::getSingleton().remove(castPtr);

    if (mMaterial.isNull() == false)
        mMaterial.setNull();

    renderer->setSLContext();

    mImages.clear();
}

bool RexOgreMaterial::create(U8* buffer, S32 size)
{
    bool success = false;
    int numMaterials = 0;
    char* buffer2 = 0;

    // If already loaded, remove
    if (isLoaded())
        destroyMaterial();

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
                // If not a brace and on level 0, it should be a new material; replace name
                if ((braceLevel == 0) && (line.substr(0, 8) == "material"))
                {
                    if (numMaterials == 0)
                    {
                        line = "material " + mName;
                        ++numMaterials;
                    }
                    else
						  {
							  llwarns << "WARNING: Defining more than one material in a script file not supported! When processing " << line << llendl;
							  break; // Do not define more than one material from script
						  }
                }
                else
                {
                    // Check for UUID-based textures
                    if ((line.substr(0, 8) == "texture ") && (line.length() > 8))
                    {
                        std::string texName = line.substr(8);
                        if (LLUUID::validate(texName))
                        {
                            LLUUID imageID(texName);
                            if (imageID != LLUUID::null)
                            {
                                LLViewerImage* image = gImageList.getImage(imageID);
                                if (image)
                                {
                                    RexOgreTexture* tex = image->getOgreTexture();
                                    line = "texture " + tex->getName();
                                    mImages.push_back(LLPointer<LLViewerImage>(image));
                                }
                            }
                        }
                    }
                }

                // Write line to the copy
                if (!skipUntilNext)
                    output << line << std::endl;
                else
                    llwarns << "Skipping risky material script line: " << line << llendl;
            }
            else
            {
                // Write line to the copy
                if (!skipUntilNext)
                    output << line << std::endl;
                else
                    llwarns << "Skipping risky material script line: " << line << llendl;

                if (braceLevel <= skipBraceLevel)
                    skipUntilNext = false;
            }
        }
    }

    // Buffer/shader creation possibly follows, so switch context
    LLOgreRenderer* renderer = LLOgreRenderer::getPointer();
    renderer->setOgreContext();

    try
    {
        // Parse material script from modified buffer
        // Apologies for ugly way of making a buffer for Ogre to use, but using the string directly seemed not to work
        std::string outStr = output.str();
        if (outStr.size())
        {
            buffer2 = static_cast<char *>(malloc(outStr.size()));
            for (unsigned i = 0; i < outStr.size(); ++i)
            {
                buffer2[i] = outStr[i];
            }
            
            Ogre::MemoryDataStreamPtr stream2(new Ogre::MemoryDataStream(buffer2, outStr.size(), false));
            Ogre::DataStreamPtr castPtr = static_cast<Ogre::DataStreamPtr>(stream2);
            Ogre::MaterialManager::getSingleton().parseScript(castPtr, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            // See if was created
            Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName(mName);
            if (!matPtr.isNull())
            {
                mMaterial = matPtr;
                success = true;
            }
            else
            {
                llwarns << "Error creating material " << mName << llendl;
                mImages.clear();
            }
        }
    }
    catch (Ogre::Exception e)
	{
		llwarns << "Error creating material " << mName << llendl;
        mImages.clear();
    }

    if (buffer2) 
    {
        free(buffer2);
    }

	renderer->setSLContext();

	return success && isUsable();
}

void RexOgreMaterial::pumpImages(F32 totalSize)
{
    for (U32 i = 0; i < mImages.size(); ++i)
    {
        if (mImages[i].notNull())
        {
            LLViewerImage* image = mImages[i];
            image->addTextureStats(totalSize);
            image->resetBindTime();
        }
    }
}

//! \todo Remove logging in release mode.
bool RexOgreMaterial::isUsable()// const
{
	if (mMaterial.isNull())
	{
		llwarns << "Material " << getName() << " is not usable because Ogre::MaterialPtr is null!" << llendl;
		return false;
	}

	bool bUsable = true; // Accumulate here to be able to print both reasons below.

	if (!mMaterial->isLoaded())
	{
		mMaterial->load();
		if (!mMaterial->isLoaded())
		{
			llwarns << "Material " << getName() << " is not usable because Ogre::MaterialPtr is not loaded! Trying to reload did not succeed!" << llendl;
			bUsable = false;
		}
		else
			llwarns << "Material " << getName() << " was not usable because Ogre::MaterialPtr is not loaded, but reloading succeeded." << llendl;
	}

	unsigned short nSupportedTechs = mMaterial->getNumSupportedTechniques();
	if (nSupportedTechs == 0)
	{
		llwarns << "Material " << getName() << " is not usable because there are no supported techniques! Reason: "
		        << mMaterial->getUnsupportedTechniquesExplanation().c_str() << llendl;
		bUsable = false;
	}

	// Test all textures so that they are valid.
	Ogre::Material::TechniqueIterator tIter = mMaterial->getTechniqueIterator();
	int techNum = 0;
	while(tIter.hasMoreElements())
	{
		Ogre::Technique *tech = tIter.getNext();
		Ogre::Technique::PassIterator pIter = tech->getPassIterator();
		int passNum = 0;
		while(pIter.hasMoreElements())
		{
			Ogre::Pass *pass = pIter.getNext();
			Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
			int tuNum = 0;
			while(texIter.hasMoreElements())
			{
				const Ogre::TextureUnitState *tu = texIter.getNext();
				Ogre::Texture *tex = static_cast<Ogre::Texture*>(Ogre::TextureManager::getSingleton().getByName(tu->getTextureName()).getPointer());
				
				if (!tex)
				{
					llwarns << "Tech " << techNum << " Pass " << passNum << " Tu " << tuNum << " uses texture " << tu->getTextureName().c_str()
					        << ", but it doesn't exist." << llendl;
				}
				else if (!tex->isLoaded())
				{
                    // Risky with local textures that are not found (may cause Ogre crash)
					//tex->load();
					//if (!tex->isLoaded())
					//{
					//	llwarns << "Tech " << techNum << " Pass " << passNum << " Tu " << tuNum << " uses texture " << tu->getTextureName().c_str()
					//	        << ", was not loaded and could not load." << llendl;
					//}
					//else
					//	llinfos << "Tech " << techNum << " Pass " << passNum << " Tu " << tuNum << " uses texture " << tu->getTextureName().c_str()
					//	        << ", which was loaded. " << llendl;
					llwarns << "Tech " << techNum << " Pass " << passNum << " Tu " << tuNum << " uses texture " << tu->getTextureName().c_str() << " which was not loaded" << llendl;
				}
				else
					llinfos << "Tech " << techNum << " Pass " << passNum << " Tu " << tuNum << " uses texture " << tu->getTextureName().c_str() << llendl;
				++tuNum;
			}
			++passNum;
		}
		++techNum;
	}

	return bUsable;
}
