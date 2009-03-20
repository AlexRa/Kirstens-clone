// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "ClothSerializer.h"
#include "Cloth.h"
#include "Platform.h"

using namespace Ogre;

namespace Rex
{
   const size_t STREAM_OVERHEAD_SIZE = sizeof(uint16) + sizeof(uint32);

   ClothSerializer::ClothSerializer()
   {
      mVersion = "[ClothSerializer_v0.1]";
   }

   ClothSerializer::~ClothSerializer()
   {
   }

   void ClothSerializer::exportCloth(const Cloth* cloth, const Ogre::String& filename, Ogre::Serializer::Endian endianMode)
//   void ClothSerializer::exportCloth(const Cloth *cloth, const Ogre::String &filename, Ogre::Serializer::Endian endianMode)
   {
      assert(cloth);

      Ogre::LogManager::getSingleton().logMessage("ClothSerializer writing cloth data to " + filename + "...");

		determineEndianness(endianMode);

      mpfFile = fopen(filename.c_str(), "wb");

      if (!mpfFile)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Unable to open file " + filename + " for writing",
				"ClothSerializer::exportCloth");
		}

      writeFileHeader();
      LogManager::getSingleton().logMessage("File header written.");

      LogManager::getSingleton().logMessage("Writing cloth data...");
      writeCloth(cloth);
      LogManager::getSingleton().logMessage("Cloth data exported.");

      fclose(mpfFile);
      LogManager::getSingleton().logMessage("ClothSerializer export successful.");
   }

   void ClothSerializer::importCloth(Ogre::DataStreamPtr &stream, Cloth *dest)
   {
     // Ogre::LogManager::getSingleton().logMessage("stream offset: " + Ogre::StringConverter::toString(stream->tell()));

      // Determine endianness (must be one of the first things we do (apparently)!)
      determineEndianness(stream);
      assert(dest);

//      dest->clean();

		

      // Check header
      readFileHeader(stream);

      unsigned short streamID;
      while(!stream->eof())
      {
         streamID = readChunk(stream);
         switch (streamID)
         {
         case C_CLOTH:
            readCloth(stream, dest);
            break;
         }
      }

      Ogre::LogManager::getSingleton().logMessage("ClothSerializer import successful.");
   }

   void ClothSerializer::writeCloth(const Cloth* cloth)
   {
//      Ogre::uint32 c_chunkSize = (Ogre::uint32)cloth->ConstraintsList.size() * sizeof(struct Constraint) + 4;
//      Ogre::uint32 v_chunkSize = (Ogre::uint32)cloth->VerticesData.size() * sizeof(struct VertexData) + 4;
      Ogre::String mesh = cloth->ClothMesh->getName();
      mesh = Rex::Platform::getFilenameFromPath(mesh);

      writeChunkHeader(C_CLOTH, calculateClothSize(cloth, mesh));


      // Write mesh link
      writeChunkHeader(C_MESHLINK, calculateMeshLinkSize(mesh));
      writeString(mesh);

      // Write whole cloth options
      writeChunkHeader(C_CLOTHOPTIONS, calculateClothOptionsSize());
      writeInts(&cloth->SimIterations, 1);
      writeBools(&cloth->WeldVertices, 1);
      writeFloats(&cloth->WindForceMultiplier, 1);
      writeFloats(&cloth->WindAmplitude, 1);
      writeFloats(&cloth->WindWaveLength, 1);

      // Constraints
      //
      writeChunkHeader(C_CONSTRAINTS, calculateConstraintsSize(cloth));
      Ogre::uint32 constraints = (Ogre::uint32) cloth->ConstraintsList.size();
      writeInts(&constraints, 1);

      size_t n;
      for (n=0 ; n<cloth->ConstraintsList.size() ; ++n)
      {
         writeInts(&cloth->ConstraintsList[n].particleA, 1);
         writeInts(&cloth->ConstraintsList[n].particleB, 1);
         writeFloats(&cloth->ConstraintsList[n].restlength, 1);
      }

      // Vertices
      //
      writeChunkHeader(C_VERTEXOPTIONS, calculateVerticesSize(cloth));
      Ogre::uint32 vertices = (Ogre::uint32) cloth->VerticesData.size();
      writeInts(&vertices, 1);
      for (n=0 ; n<cloth->VerticesData.size() ; ++n)
      {
         writeBools(&cloth->VerticesData[n].Fixed, 1);
         writeBools(&cloth->VerticesData[n].PhysicsEnabled, 1);
         writeBools(&cloth->VerticesData[n].AffectedByForces, 1);
         writeFloats(&cloth->VerticesData[n].Rigidness, 1);
         writeFloats(&cloth->VerticesData[n].RigidnessMultiplier, 1);
         writeFloats(&cloth->VerticesData[n].ForceMultiplier, 1);

         writeFloats(cloth->VerticesData[n].Gravity.ptr(), 3);

         writeBools(&cloth->VerticesData[n].NormalPlaneCollision, 1);
         writeFloats(&cloth->VerticesData[n].NormalPlaneCollisionDistance, 1);
         
         writeBools(&cloth->VerticesData[n].WindFlutter, 1);
         writeBools(&cloth->VerticesData[n].Welded, 1);
         writeInts(&cloth->VerticesData[n].WeldedTo, 1);
      }
   }

   size_t ClothSerializer::calculateClothSize(const Cloth *cloth, const Ogre::String &mesh)
   {
      size_t size = STREAM_OVERHEAD_SIZE;
      size += calculateMeshLinkSize(mesh);
      size += calculateClothOptionsSize();
      size += calculateConstraintsSize(cloth);
      size += calculateVerticesSize(cloth);

      return size;
   } 

   size_t ClothSerializer::calculateClothOptionsSize()
   {
      size_t size = STREAM_OVERHEAD_SIZE;
      size += sizeof(unsigned int);
      size += sizeof(bool);

      size += sizeof(Ogre::Real);
      size += sizeof(Ogre::Real);
      size += sizeof(Ogre::Real);

      return size;
   }

   size_t ClothSerializer::calculateConstraintsSize(const Cloth *cloth)
   {
      size_t size = STREAM_OVERHEAD_SIZE;
      size += sizeof(Ogre::uint32);
      size += (Ogre::uint32)cloth->ConstraintsList.size() * sizeof(struct Constraint);

      return size;
   }

   size_t ClothSerializer::calculateVerticesSize(const Cloth *cloth)
   {
      size_t size = STREAM_OVERHEAD_SIZE;
      size += sizeof(Ogre::uint32);
      size += (Ogre::uint32)cloth->VerticesData.size() * VertexData::calculateSize();

      return size;
   }

   size_t ClothSerializer::calculateMeshLinkSize(const Ogre::String &mesh)
   {
      size_t size = STREAM_OVERHEAD_SIZE;
      size += mesh.size() + 1;

      return size;
   }

   void ClothSerializer::readCloth(Ogre::DataStreamPtr &stream, Rex::Cloth *dest)
   {
      unsigned short streamID;
      if (!stream->eof())
      {
         streamID = readChunk(stream);
         while(!stream->eof())
         {
            switch(streamID)
            {
            case C_MESHLINK:
               readMeshLink(stream, dest);
               break;

            case C_CLOTHOPTIONS:
               readClothOptions(stream, dest);
               break;

				case C_CONSTRAINTS:
               readConstraints(stream, dest);
               break;

            case C_VERTEXOPTIONS:
               readVertices(stream, dest);
               break;
            }

            if (!stream->eof())
            {
               streamID = readChunk(stream);
            }
         }
         //if (!stream->eof())
         //{
         //    // Backpedal back to start of stream
         //    stream->skip(-STREAM_OVERHEAD_SIZE);
         //}

      }
   }

   void ClothSerializer::readMeshLink(Ogre::DataStreamPtr& stream, Cloth* dest)
   {
      Ogre::String meshName = readString(stream);
      dest->setMeshName(meshName);
      //try
      //{  
      //   Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
      //   dest->setMesh(mesh);
      //} catch (Ogre::Exception)
      //{
      //   Ogre::LogManager::getSingleton().logMessage("Failed to load mesh: " + meshName + ".");
      //}
   }

   void ClothSerializer::readClothOptions(Ogre::DataStreamPtr& stream, Cloth* dest)
   {
      readInts(stream, &dest->SimIterations, 1);
      readBools(stream, &dest->WeldVertices, 1);

      readFloats(stream, &dest->WindForceMultiplier, 1);
      readFloats(stream, &dest->WindAmplitude, 1);
      readFloats(stream, &dest->WindWaveLength, 1);
   }

   void ClothSerializer::readConstraints(Ogre::DataStreamPtr& stream, Cloth* dest)
   {
      Ogre::LogManager::getSingleton().logMessage("Reading constraints...");
      Ogre::uint32 constraints = 0;
      readInts(stream, &constraints, 1);

      Ogre::LogManager::getSingleton().logMessage("Constraints:" + Ogre::StringConverter::toString(constraints));

      dest->ConstraintsList.clear();
      dest->ConstraintsList.reserve(constraints);
      size_t n;
      for (n=0 ; n<constraints ; ++n)
      {
         Constraint c;
         readInts(stream, &c.particleA, 1);
         readInts(stream, &c.particleB, 1);
         readFloats(stream, &c.restlength, 1);

         dest->ConstraintsList.push_back(c);
      }
   }

   void ClothSerializer::readVertices(Ogre::DataStreamPtr& stream, Cloth* dest)
   {
      Ogre::LogManager::getSingleton().logMessage("Reading vertices...");

      Ogre::uint32 vertices = 0;
      readInts(stream, &vertices, 1);

      Ogre::LogManager::getSingleton().logMessage("Vertices:" + Ogre::StringConverter::toString(vertices));

      dest->VerticesData.clear();
      dest->VerticesData.reserve(vertices);
      size_t n;
      for (n=0 ; n<vertices ; ++n)
      {
         VertexData v;
         readBools(stream, &v.Fixed, 1);
         readBools(stream, &v.PhysicsEnabled, 1);
         readBools(stream, &v.AffectedByForces, 1);
         readFloats(stream, &v.Rigidness, 1);
         readFloats(stream, &v.RigidnessMultiplier, 1);
         readFloats(stream, &v.ForceMultiplier, 1);
   
         readFloats(stream, v.Gravity.ptr(), 3);

         readBools(stream, &v.NormalPlaneCollision, 1);
         readFloats(stream, &v.NormalPlaneCollisionDistance, 1);

         readBools(stream, &v.WindFlutter, 1);
         readBools(stream, &v.Welded, 1);
         readInts(stream, &v.WeldedTo, 1);

         dest->VerticesData.push_back(v);
      }
   }
}
