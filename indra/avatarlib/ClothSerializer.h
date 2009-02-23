// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __ClothSerializer_h__
#define __ClothSerializer_h__

#include <OGRE/OgrePrerequisites.h>
#include <OGRE/OgreString.h>
#include <OGRE/OgreSerializer.h>

namespace Rex
{
   class Cloth;

   enum ClothChunkID
   {
      C_CLOTH =         0x1000,
      C_MESHLINK =      0x2000,
      C_CONSTRAINTS =   0x3000,
      C_VERTEXOPTIONS = 0x4000,
      C_CLOTHOPTIONS =  0x5000
   };

   //! Exports or imports additional cloth data regarding to an Ogre mesh.
   /*! This enables mesh to be treated as a physics enabled cloth.
       \note Takes advantage of Ogre's handy Serializer class, for handling
             Endianess and stuff.
   */
   class ClothSerializer : public Ogre::Serializer
   {
   public:
      //! Default constructor
      ClothSerializer();

      //! destructor
      virtual ~ClothSerializer();

      //! Exports a cloth to the specified file. 
      /*!
          \param cloth Pointer to the cloth to export
          \param filename The destination filename
	       \param endianMode The endian mode for the written file
      */
      void exportCloth(const Cloth* cloth, const Ogre::String& filename,
         Ogre::Serializer::Endian endianMode = Ogre::Serializer::ENDIAN_NATIVE);

      //! Imports cloth from a .cloth file DataStream.
      /*!
          \param stream The DataStream holding the .mesh data. Must be initialised (pos at the start of the buffer).
          \param pDest Pointer to the Mesh object which will receive the data. Should be blank already.
      */
      void importCloth(Ogre::DataStreamPtr& stream, Cloth* dest);

   private:
      //! Writes cloths data
      void writeCloth(const Cloth* cloth);

      //! Calculate size of cloth chunk
      size_t calculateClothSize(const Cloth *cloth, const Ogre::String &mesh);

      //! Calculate size of cloth options chunk
      size_t calculateClothOptionsSize();

      //! Calculate size of constraints chunk
      size_t calculateConstraintsSize(const Cloth *cloth);

      //! Calculate size of vertices chunk
      size_t calculateVerticesSize(const Cloth *cloth);

      //! Calculate size of mesh link chunk
      size_t calculateMeshLinkSize(const Ogre::String &mesh);

      //! Read data into cloth
      void readCloth(Ogre::DataStreamPtr& stream, Cloth* dest);

      //! Read mesh link from stream
      void readMeshLink(Ogre::DataStreamPtr& stream, Cloth* dest);

      //! Read cloth options from stream
      void readClothOptions(Ogre::DataStreamPtr& stream, Cloth* dest);

      //! Read constraints data from stream into cloth
      void readConstraints(Ogre::DataStreamPtr& stream, Cloth* dest);

      //! Read vertex physics data from stream into cloth
      void readVertices(Ogre::DataStreamPtr& stream, Cloth* dest);
   };
}

#endif // __ClothSerializer_h__
