#ifndef LLEXTERNALTEXTURESOURCE_H
#define LLEXTERNALTEXTURESOURCE_H

#include <OGRE/OgreExternalTextureSource.h>

class LLExternalTextureSource : public Ogre::ExternalTextureSource
{
public:
    virtual bool requiresAuthorization() const = 0;

};	//	class LLExternalTextureSource

#endif	//	LLEXTERNALTEXTURESOURCE_H
