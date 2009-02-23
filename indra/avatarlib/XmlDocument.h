// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __XmlDocument_h__
#define __XmlDocument_h__

#include "tinyxml.h"

namespace Rex
{
   
//   typedef TiXmlDocument XmlDocument;
//   typedef TiXmlNode XmlNode;
//   typedef TiXmlElement XmlElement;

   class XmlNode //: public TiXmlNode
   {
   public:
      //! Create xml node from TiXmlElement
      XmlNode(TiXmlElement *element);

      //! Create xml node from TiXmlElement
      XmlNode(const TiXmlElement *element);

      virtual ~XmlNode() {}

   protected: 
      TiXmlElement *Element;
      const TiXmlElement *ConstElement;
   };

   class XmlDocument //: public TiXmlDocument
   {
   public:
      //! Create an empty document, that has no name.
      XmlDocument() : TiXmlDocument();
      {
      }
	   //! Create a document with a name. The name of the document is also the filename of the xml.
//	   TiXmlDocument( const char * documentName );

	   //! Create a document with a name. The name of the document is also the filename of the xml.
      XmlDocument(const std::string &documentName) : TiXmlDocument(documentName)
      {
      }

      virtual ~XmlDocument() {}

      virtual const char *parse(const char *data);

      XmlNode *getRootNode();
      const XmlNode *getRootNode() const;

   protected:

      TiXmlDocument Doc;
   };

}
#endif // __XmlDocument_h__