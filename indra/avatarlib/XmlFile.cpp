// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "XmlFile.h"
#include "XmlManager.h"
#include "RexException.h"
#include <OGRE/Ogre.h>

using namespace Ogre;

namespace Rex
{
   XmlFile::XmlFile(Ogre::ResourceManager *creator, const Ogre::String &name,
                    Ogre::ResourceHandle handle, const Ogre::String &group,
                    bool isManual, Ogre::ManualResourceLoader *loader) : 
   Ogre::Resource(creator, name, handle, group, isManual, loader), Parsed(false), Size(0)
   {
      if (createParamDictionary("XmlFile"))
      {
         ParamDictionary* dict = getParamDictionary();
      }
   }

   XmlFile::~XmlFile()
   {
   //   unload();
   }

   void XmlFile::loadImpl()
   {
      Ogre::LogManager::getSingletonPtr()->logMessage("XML: Loading " + mName + ".");

      Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(mName, mGroup, true, this);

      if (stream->size() == 0)
      {
         REX_EXCEPT(RexException::ERR_INTERNAL_ERROR, "Empty resource", getName());
      }

      XMLDoc = XmlDocument(mName.c_str());
      XMLDoc.Parse(stream->getAsString().c_str());


      if ( XMLDoc.Error() )
	   {
         REX_EXCEPT(RexException::ERR_INTERNAL_ERROR, "Failed to parse xml file. Check if file is proper xml.", getName());
	   }

      Parsed = true;

      Size = stream->size();
   }

   void XmlFile::unloadImpl()
   {
   //   Ogre::LogManager::getSingletonPtr()->logMessage("XML: Unloading " + mName + ".");

      Size = 0;
      // unload
      XMLDoc.Clear();
   }

   size_t XmlFile::calculateSize() const
   {
      //! \todo How do we calculate this size properly?
      

      return Size;
   }

   void XmlFile::remove()
   {
      XmlManager::getSingleton().remove(this->getHandle());
   }

   XmlDocument *XmlFile::getDocument()
   {
      if (!Parsed)
         return 0;

      return &XMLDoc;
   }

   XmlElement *XmlFile::getRootNode()
   {
      if (!Parsed)
         return false;

      XmlElement *element = XMLDoc.RootElement();

      return element;
   }

   int XmlFile::queryColorAttribute(const XmlElement *element, const char *attribute, Ogre::ColourValue *value)
   {
      Ogre::Vector4 vec;

      int result = queryVectorAttribute(element, attribute, &vec, 4);
      if (result == TIXML_SUCCESS)
      {
         value->r = vec.x;
         value->g = vec.y;
         value->b = vec.z;
         value->a = vec.w;
      }

      return result;
   }

   int XmlFile::queryVectorAttribute(const XmlElement *element, const char *attribute, Ogre::Vector4 *value, int vectorelements)
   {
      assert(element != NULL);
      assert(attribute != NULL);
      assert(value != NULL);

      const char *strValue = element->Attribute(attribute);
      if (strValue == NULL || strValue[0] == '\0')
         return TIXML_NO_ATTRIBUTE;

      Ogre::Vector4 vec;
      int count = 0;
      try
      {
         count = sscanf(strValue, "%f %f %f %f", &vec.x, &vec.y, &vec.z, &vec.w);
      } catch(...)
      {
         // No need to handle
      }
	   if (count == vectorelements)
	   {
		   *value = vec;
		   return TIXML_SUCCESS;
	   }

	   return TIXML_WRONG_TYPE;
   }

   int XmlFile::queryVector4Attribute(const XmlElement *element, const char *attribute, Ogre::Vector4 *value)
   {
      Ogre::Vector4 vec;

      int result = queryVectorAttribute(element, attribute, &vec, 4);
      if (result == TIXML_SUCCESS)
         *value = vec;

      return result;
   }

   int XmlFile::queryVector3Attribute(const XmlElement *element, const char *attribute, Ogre::Vector3 *value)
   {
      Ogre::Vector4 vec;

      int result = queryVectorAttribute(element, attribute, &vec, 3);
      if (result == TIXML_SUCCESS)
         *value = Vector3(vec.ptr());

      return result;
   }

   int XmlFile::queryVector2Attribute(const XmlElement *element, const char *attribute, Ogre::Vector2 *value)
   {
      Ogre::Vector4 vec;

      int result = queryVectorAttribute(element, attribute, &vec, 2);
      if (result == TIXML_SUCCESS)
         *value = Vector2(vec.ptr());

      return result;
   }

   int XmlFile::queryQuaternionAttributeAsEuler(const XmlElement *element, const char *attribute, Ogre::Quaternion *value)
   {
      Ogre::Vector4 vec;

      int result = queryVectorAttribute(element, attribute, &vec, 3);
      if (result == TIXML_SUCCESS)
      {
         Ogre::Vector3 eulerAngle(vec.ptr());
         eulerAngle.x = Ogre::Degree(eulerAngle.x).valueRadians();
         eulerAngle.y = Ogre::Degree(eulerAngle.y).valueRadians();
         eulerAngle.z = Ogre::Degree(eulerAngle.z).valueRadians();
         *value = quaternionFromEuler(eulerAngle);
      }

      return result;
   }

   int XmlFile::queryQuaternionAttribute(const XmlElement *element, const char *attribute, Ogre::Quaternion *value)
   {
      Ogre::Vector4 vec;

      int result = queryVectorAttribute(element, attribute, &vec, 4);
      if (result == TIXML_SUCCESS)
      {
         (*value).w = vec.x;
         (*value).x = vec.y;
         (*value).y = vec.z;
         (*value).z = vec.w;
      } else
      {
         return queryQuaternionAttributeAsEuler(element, attribute, value);
      }

      return result;
   }

   int XmlFile::queryBoolAttribute(const XmlElement *element, const char *attribute, bool *value)
   {
      assert(element != NULL);
      assert(attribute != NULL);
      assert(value != NULL);

      const char *strValue = element->Attribute(attribute);
      if (strValue == NULL || strValue[0] == '\0')
         return TIXML_NO_ATTRIBUTE;

      if ((!strcmp(strValue, "True")) || (!strcmp(strValue, "true")))
      {
          *value = true;
          return TIXML_SUCCESS;
      }

      if ((!strcmp(strValue, "False")) || (!strcmp(strValue, "false")))
      {
          *value = false;
          return TIXML_SUCCESS;
      }

      return TIXML_WRONG_TYPE;
   }

   void XmlFile::setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::ColourValue &value)
   {
      const Ogre::Real *values = value.ptr();
      int n = 4;
      
      setAttributeFloat(element, attribute, values, n);
   }

   void XmlFile::setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::Vector4 &value)
   {
      const Ogre::Real *values = value.ptr();
      int n = 4;
      
      setAttributeFloat(element, attribute, values, n);
   }

   void XmlFile::setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::Vector3 &value)
   {
      const Ogre::Real *values = value.ptr();
      int n = 3;
      
      setAttributeFloat(element, attribute, values, n);
   }

   void XmlFile::setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::Vector2 &value)
   {
      const Ogre::Real *values = value.ptr();
      int n = 2;
      
      setAttributeFloat(element, attribute, values, n);
   }

   void XmlFile::setAttributeAsEuler(XmlElement *element, const Ogre::String &attribute, const Ogre::Quaternion &value)
   {
      Ogre::Matrix3 rotMatrix;
      value.ToRotationMatrix(rotMatrix);

      Ogre::Radian anglex;
      Ogre::Radian angley;
      Ogre::Radian anglez;
      rotMatrix.ToEulerAnglesXYZ(anglex, angley, anglez);

      Ogre::Real angles[3];
      angles[0] = anglex.valueDegrees();
      angles[1] = angley.valueDegrees();
      angles[2] = anglez.valueDegrees();
      //angles[0] = value.getYaw().valueDegrees();
      //angles[1] = value.getPitch().valueDegrees();
      //angles[2] = value.getRoll().valueDegrees();

      setAttributeFloat(element, attribute, angles, 3);
   }

   void XmlFile::setAttribute(XmlElement *element, const Ogre::String &attribute, const Ogre::Quaternion &value)
   {
      setAttributeFloat(element, attribute, value.ptr(), 4);
   }

   void XmlFile::setAttributeFloat(XmlElement *element, const Ogre::String &attribute, const Ogre::Real *values, int elements)
   {
      assert(element != NULL);
      assert(attribute.empty() == false);
      assert(values != NULL);
      assert(elements > 0 && elements < 5);
      
      Ogre::String valueStr;
      
      int n;
      for (n=0 ; n<elements ; ++n)
      {
         std::stringstream ss;
         Ogre::String str;

         ss << values[n] << " ";
         ss >> str;

         valueStr.append(str);
         if (elements-1 != n)
            valueStr.append(" ");
      }

      element->SetAttribute(attribute, valueStr);
   }

   void XmlFile::setAttribute(XmlElement *element, const Ogre::String &attribute, bool value)
   {
      assert(element != NULL);
      assert(attribute.empty() == false);
     
      if (value)
         element->SetAttribute(attribute, "true");
      else
         element->SetAttribute(attribute, "false");
   }

   Ogre::Quaternion XmlFile::quaternionFromEuler(const Ogre::Vector3 &euler)
   {
      Ogre::Quaternion quat;
      double angle;
      
      angle = euler.y * 0.5;
      double cx = Ogre::Math::Cos(angle);
      double sx = Ogre::Math::Sin(angle);

      angle = euler.z * 0.5;
      double cy = Ogre::Math::Cos(angle);
      double sy = Ogre::Math::Sin(angle);

      angle = euler.x * 0.5;
      double cz = Ogre::Math::Cos(angle);
      double sz = Ogre::Math::Sin(angle);

      quat.x = sx * sy * cz + cx * cy * sz;
      quat.y = sx * cy * cz + cx * sy * sz;
      quat.z = cx * sy * cz - sx * cy * sz;
      quat.w = cx * cy * cz - sx * sy * sz;

      quat.normalise();
      return quat;
   }
} // namespace Rex