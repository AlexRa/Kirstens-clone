// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __DynamicAnimationSerializer_h__
#define __DynamicAnimationSerializer_h__

#include "XmlFile.h"
#include <OGRE/Ogre.h>

namespace Rex
{
   class DynamicAnimation;

   //! Serializes a dynamic animation to/from xml.
   /*! Only reads/writes to already opened and parsed xml document.
   */
   class DynamicAnimationSerializer : public Ogre::Serializer
   {
   public:
      //! default constructor
      DynamicAnimationSerializer();

      //! destructor
      virtual ~DynamicAnimationSerializer();

      //! Exports animation fully to xml. Exports the whole animation.
      /*! Writes the animation as xml under the specified element.

          \param animation animation to export
          \param rootElement xml element
      */
      void exportAnimation(DynamicAnimation *animation, XmlElement *rootElement);

      //! Imports animation from xml.
      /*! Reads data under the specified element.
          
          \param rootElement Xml element
          \param pDest Animation to which the data will be imported to
          \return True if animation imported succesfully, false otherwise
      */
      bool importAnimation(const XmlElement *rootElement, DynamicAnimation *animationDest);
   };
}

#endif // __DynamicAnimationSerializer_h__

