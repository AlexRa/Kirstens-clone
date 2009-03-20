// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __XmlFile_h__
#define __XmlFile_h__

#include "tinyxml.h"
#include <OGRE/OgreResourceManager.h>

namespace Rex
{
//   class XmlNode //: public TiXmlNode
//   {
//   public:
//      //! Create xml node from TiXmlElement
//      XmlNode(TiXmlElement *element);
//
//      //! Create xml node from TiXmlElement
//      XmlNode(const TiXmlElement *element);
//
//      virtual ~XmlNode() {}
//
//   protected: 
//      TiXmlElement *Element;
//      const TiXmlElement *ConstElement;
//   };
//
//   class XmlDocument //: public TiXmlDocument
//   {
//   public:
//      //! Create an empty document, that has no name.
//      XmlDocument() : TiXmlDocument();
//      {
//      }
//	   //! Create a document with a name. The name of the document is also the filename of the xml.
////	   TiXmlDocument( const char * documentName );
//
//	   //! Create a document with a name. The name of the document is also the filename of the xml.
//      XmlDocument(const std::string &documentName) : TiXmlDocument(documentName)
//      {
//      }
//
//      virtual ~XmlDocument() {}
//
//      virtual const char *parse(const char *data);
//
//      XmlNode *getRootNode();
//      const XmlNode *getRootNode() const;
//
//   protected:
//
//      TiXmlDocument Doc;
//   };



   typedef TiXmlDocument XmlDocument;
   typedef TiXmlNode XmlNode;
   typedef TiXmlElement XmlElement;

   //! An xml file resource.
   /*! Uses TinyXml to parse xml files. Provides helper functions for
       getting and setting xml vector attributes.
   */
   class XmlFile : public Ogre::Resource
   {
   public:
      //! create default xml file resource
      XmlFile(Ogre::ResourceManager *creator, const Ogre::String &name, Ogre::ResourceHandle handle,
         const Ogre::String &group, bool isManual = false, Ogre::ManualResourceLoader *loader = 0);

      //! destructor
      virtual ~XmlFile();

      //! Get tiny xml document
      XmlDocument *getDocument();

      //! Get root xml node
      XmlElement *getRootNode();

      //! Removes this resource from resource manager
      void remove();

      // attribute queries
      ////////////////////

      //! Retrieves a colour value from element and attribute
      /*! Value gets modified only on success.

          \param element xml element
          \param attribute attribute
          \param value received color value
          \return TIXML_SUCCESS on success, TIXML_WRONG_TYPE or TIXML_NO_ATTRIBUTE otherwise
      */
      static int queryColorAttribute(const XmlElement *element, const char *attribute, Ogre::ColourValue *value);

      //! See queryColorAttribute
      static int queryVector4Attribute(const XmlElement *element, const char *attribute, Ogre::Vector4 *value);
      //! See queryColorAttribute
      static int queryVector3Attribute(const XmlElement *element, const char *attribute, Ogre::Vector3 *value);
      //! See queryColorAttribute
      static int queryVector2Attribute(const XmlElement *element, const char *attribute, Ogre::Vector2 *value);
      //! See queryColorAttribute
      /*!
          \note if query fails, calls queryQuaternionAttributeAsEuler in case the rotation is in euler angles
      */
      static int queryQuaternionAttribute(const XmlElement *element, const char *attribute, Ogre::Quaternion *value);
      //! See queryColorAttribute
      static int queryQuaternionAttributeAsEuler(const XmlElement *element, const char *attribute, Ogre::Quaternion *value);
      //! See queryColorAttribute
      static int queryBoolAttribute(const XmlElement *element, const char *attribute, bool *value);

      //! Sets attibute from colour
      /*!
          \param element xml element
          \param attribute attribute in the element to set
          \param value Colour
      */
      static void setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::ColourValue &value);
      static void setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::Vector4 &value);
      static void setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::Vector3 &value);
      static void setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::Vector2 &value);
      //! Sets attribute from quat as quat
      static void setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::Quaternion &value);
      //! sets attribute from quat as euler angles in degrees
      static void setAttributeAsEuler(XmlElement *element, const Ogre::String &attribute, const Ogre::Quaternion &value);
      static void setAttribute(XmlElement *element, const Ogre::String &attribute, bool value);

      //! Helper function for converting between euler and quaternion angles
      /*! From: http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/index.htm
      */
      static Ogre::Quaternion quaternionFromEuler(const Ogre::Vector3 &euler);

   protected:
      void loadImpl();
      void unloadImpl();
      //! Returns the xml file size.
      /*! \note The size is probably not strictly correct for this resource, as TinyXml doc is probably different
          size from the actual xml file size.
      */
      size_t calculateSize() const;

      //! returns a vector from xml attribute
      static int queryVectorAttribute(const XmlElement *element, const char *attribute, Ogre::Vector4 *value, int vectorelements);

      //! set xml attribute from float values
      static void setAttributeFloat(XmlElement *element, const Ogre::String &attribute, const Ogre::Real *values, int elements);

      //! Tiny XML doc
      XmlDocument XMLDoc;

      //! Is the file parsed properly
      bool Parsed;

      //! xml file size
      size_t Size;
   };

   //! Specialisation of SharedPtr to allow SharedPtr to be assigned to XmlFilePtr.
   /*! \note Has to be a subclass since we need operator=. We could templatise this instead of repeating per Resource subclass, except to do so requires a form VC6 does not support i.e. ResourceSubclassPtr<T> : public SharedPtr<T>
   */
   class XmlFilePtr : public Ogre::SharedPtr<XmlFile>
   {
   public:
      XmlFilePtr() : Ogre::SharedPtr<XmlFile>() {}
      explicit XmlFilePtr(XmlFile *rep) : Ogre::SharedPtr<XmlFile>(rep) {}
      XmlFilePtr(const XmlFilePtr &rep) : Ogre::SharedPtr<XmlFile>(rep) {}
      XmlFilePtr(const Ogre::ResourcePtr &r) : Ogre::SharedPtr<XmlFile>()
      {
         // lock & copy other mutex pointer
         OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
            OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<XmlFile*>(r.getPointer());
         pUseCount = r.useCountPointer();
         if (pUseCount)
         {
            ++(*pUseCount);
         }
      }

      /// Operator used to convert a ResourcePtr to a TextFilePtr
      XmlFilePtr& operator=(const Ogre::ResourcePtr& r)
      {
        if (pRep == static_cast<XmlFile*>(r.getPointer()))
            return *this;
        release();
        // lock & copy other mutex pointer
        OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
            OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<XmlFile*>(r.getPointer());
        pUseCount = r.useCountPointer();
        if (pUseCount)
        {
            ++(*pUseCount);
        }
        return *this;
      }
   };

} // namespace Rex
#endif // __XmlFile_h__

