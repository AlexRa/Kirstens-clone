/** 
 * @file rexogretexture.h
 * @brief RexOgreTexture header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#ifndef REXOGRETEXTURE_H
#define REXOGRETEXTURE_H

#include <OGRE/Ogre.h>
#include <string>

class LLImageRaw;

class RexOgreTexture
{
public:
    //! Constructor
    RexOgreTexture(const std::string& name);

    //! Destructor
    ~RexOgreTexture();

    //! Sets new texture from rawimage
    bool setTexture(LLImageRaw* image);

    //! Returns pointer to Ogre texture
    const Ogre::TexturePtr getTexture() const { return mTexture; }

    //! Returns name
    const std::string& getName() const { return mName; }

protected:
    //! Destroys texture
    void destroyTexture();
    
    //! Sets new texture
    bool setTexture(S32 format, const U8* data_in, S32 width, S32 height, S32 size, bool useMips, bool hasMips);

    //! Texture name (usually UUID as string)
    std::string mName;
    
    //! Ogre texture
    Ogre::TexturePtr mTexture;
    
    //! Previous size and Ogre format of the texture, to see whether texture can just be updated or needs to be recreated
    Ogre::PixelFormat mPrevFormat;
    int mPrevWidth;
    int mPrevHeight;
    int mPrevMips;
};

#endif // REXOGRETEXTURE_H
