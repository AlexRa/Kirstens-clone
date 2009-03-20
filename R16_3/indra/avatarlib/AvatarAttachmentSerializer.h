// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __AvatarAttachmentSerializer_h__
#define __AvatarAttachmentSerializer_h__

#include "AvatarAttachment.h"
#include "XmlFile.h"


namespace Rex
{
   class AvatarAttachmentSerializer
   {
   public:
      //! default constructor
      AvatarAttachmentSerializer();

      //! destructor
      ~AvatarAttachmentSerializer();

      //! Export attachment to a file
      /*!
          \param attachment the attachment to export
          \param filename the file where to export to
      */
      void exportAttachment(const AvatarAttachment *attachment, const std::string &filename);

      //! Export attachment to an xml element
      /*! 
          \note Set onlyAttached to true when exporting avatar so only the attached meshes get exported.

          \param attachment the attachment to export
          \param root The xml element under which the attachment is exported to
          \param onlyAttached If true, exports only bones that are currently attached to avatar, otherwise export all bones
      */
      void exportAttachment(const AvatarAttachment *attachment, XmlElement *root, bool onlyAttached = false);

      //! Imports an attachment from a file
      void importAttachment(const std::string &filename, AvatarAttachment *attachment);

      //! Imports attachment from an xml element
      void importAttachment(const XmlElement *root, AvatarAttachment *attachment);
   };
}

#endif // __AvatarAttachmentSerializer_h__
