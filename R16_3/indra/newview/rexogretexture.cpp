/** 
 * @file rexogretexture.cpp
 * @brief RexOgreTexture class implementation
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"
#include "llviewerimage.h"
#include "llogre.h"
#include "llgl.h"
#include "rexogretexture.h"

RexOgreTexture::RexOgreTexture(const std::string& name) :
    mName(name),
    mPrevWidth(-1), 
    mPrevHeight(-1),
    mPrevMips(0)
{
}

RexOgreTexture::~RexOgreTexture()
{
    destroyTexture();
}

void RexOgreTexture::destroyTexture()
{
    if (mTexture.isNull())
        return;

    LLOgreRenderer* renderer = LLOgreRenderer::getPointer();
    if (!renderer->isInitialized())
        return;

    renderer->setOgreContext();

    Ogre::ResourcePtr castPtr = static_cast<Ogre::ResourcePtr>(mTexture);
    Ogre::TextureManager::getSingleton().remove(castPtr);

    if (mTexture.isNull() == false)
    {
        mTexture.setNull();
        //llwarns << "Texture not removed properly." << mName << llendl;
    }

    renderer->setSLContext();
}

bool RexOgreTexture::setTexture(LLImageRaw* image)
{
    S32 format;

    switch (image->getComponents())
    {
    case 1:
        format = GL_LUMINANCE;
        break;

    case 2:
        format = GL_LUMINANCE_ALPHA;
        break;

    case 3:
        format = GL_RGB;
        break;

    case 4:
        format = GL_RGBA;
        break;

    default:
        llwarns << "Unsupported number of components." << llendl;
        return false;
    }

    return setTexture(format, image->getData(), image->getWidth(), image->getHeight(), image->getDataSize(), true, false);
}

bool RexOgreTexture::setTexture(S32 format, const U8* data_in, S32 width, S32 height, S32 size, bool useMips, bool hasMips)
{
    bool success = false;

    LLOgreRenderer *renderer = LLOgreRenderer::getPointer();
    // Ogre may not have been yet initialised when some textures get loaded (preloaded images)
    if (!renderer->isInitialized())
        return success;

    int numMips = 0;
    if (useMips)
        numMips = Ogre::MIP_DEFAULT;

    Ogre::TextureUsage usage = Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE;
    if (!hasMips)
    {
        if (useMips)
            usage = (Ogre::TextureUsage)(usage | Ogre::TU_AUTOMIPMAP);
    } else
    {
        llwarns << "Source image " << mName << " contains mipmaps, cannot create texture" << llendl;
        return success; // We don't yet handle the case where image already contains mips, we always auto-generate them
    }

    Ogre::PixelFormat ogreFormat = Ogre::PF_BYTE_RGBA; // default format
    switch (format)
    {
    case GL_RGBA:
        ogreFormat = Ogre::PF_A8B8G8R8;
        break;

    case GL_RGB:
        ogreFormat = Ogre::PF_B8G8R8;
        break;

    case GL_ALPHA:
        ogreFormat = Ogre::PF_A8;
        break;

    case GL_LUMINANCE:
        ogreFormat = Ogre::PF_L8;
        break;

   case GL_LUMINANCE_ALPHA:
      ogreFormat = Ogre::PF_BYTE_LA;
      break;

    default:
        // unsupported format
        llwarns << "Unsupported Ogre texture format " << mName << " source:" << size << " LL format " << format << " Ogre format " << ogreFormat << llendl;
        return success;
    }

    renderer->setOgreContext();

    try
    {
        // If no texture yet, must create now
        if (mTexture.isNull())
        {
            // See first that the texture does not already exist, as a result of failed earlier load, if it does, "take ownership" :)
            // That option is kind of evil and we hope it doesn't happen, but Ogre didn't seem to crash
            Ogre::TexturePtr oldTex = Ogre::TextureManager::getSingleton().getByName(mName);
            if (oldTex.isNull())
            {
                mTexture = Ogre::TextureManager::getSingleton().createManual(
                mName,
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D,
                width, height,
                numMips,
                ogreFormat,
                usage);

                mPrevFormat = ogreFormat;
                mPrevWidth = width;
                mPrevHeight = height;
                mPrevMips = numMips;
            }
            else
            {
                llwarns << "Reusing old texture " << mName << llendl;
                mTexture = oldTex;
                mPrevWidth = -1;
                mPrevHeight = -1;
                mPrevMips = 0;
            }
        }

        // Must unload/reload if format or size changed
        if ((ogreFormat != mPrevFormat) || (width != mPrevWidth) || (height != mPrevHeight) || (numMips != mPrevMips))
        {
            mTexture->freeInternalResources();
            mTexture->setWidth(width);
            mTexture->setHeight(height);
            mTexture->setFormat(ogreFormat);
            mTexture->setNumMipmaps((numMips == Ogre::MIP_DEFAULT)? Ogre::TextureManager::getSingleton().getDefaultNumMipmaps() :
                static_cast<size_t>(numMips));
            mTexture->setUsage(usage);
            mTexture->createInternalResources();

            mPrevFormat = ogreFormat;
            mPrevWidth = width;
            mPrevHeight = height;
            mPrevMips = numMips;
        }

        Ogre::Box src(0,0, width, height);
        Ogre::PixelBox pixelBox(src, ogreFormat, (void*)data_in);
        mTexture->getBuffer()->blitFromMemory(pixelBox);

        success = true;
    }
    catch (Ogre::Exception e)
    {
        llwarns << "Texture " << mName << " create/update failed, " << e.getFullDescription() << llendl;
    }

    renderer->setSLContext();

    return success;
}
