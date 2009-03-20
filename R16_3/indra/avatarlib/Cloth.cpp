// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt


#ifdef REX_THREADED
#  include "linden_common.h"
#undef REX_THREADED
#endif

#include "Cloth.h"

#include "ClothThread.h"
#include "ClothSerializer.h"
#include "ClothManager.h"

namespace Rex
{
   ClothThread *Cloth::Thread = 0;
   bool Cloth::smRenderPhysics = true;
   Ogre::Math *Cloth::OgreMath = 0;

   const bool          VertexData::DEFAULT_PHYSICSENABLED = false;
   const bool          VertexData::DEFAULT_FIXED = false;
   const bool          VertexData::DEFAULT_AFFECTEDBYFORCES = true;
   const Ogre::Real    VertexData::DEFAULT_RIGIDNESS = 0.0f;
   const Ogre::Real    VertexData::DEFAULT_RIGIDNESSMULTIPLIER = 1.0f;
   const Ogre::Vector3 VertexData::DEFAULT_GRAVITY = Ogre::Vector3(0.f, -0.6f, 0.f);
   const bool          VertexData::DEFAULT_NORMALPLANECOLLISION = false;
   const Ogre::Real    VertexData::DEFAULT_NORMALPLANECOLLISIONDISTANCE = 0.0f;
   const bool          VertexData::DEFAULT_WINDFLUTTER = false;
   const Ogre::Real    VertexData::DEFAULT_FORCEMULTIPLIER = 1.0f;

   const bool Cloth::DEFAULT_WELDVERTICES = true;
   const int  Cloth::DEFAULT_SIMITERATIONS = 1;
   const Ogre::Real Cloth::DEFAULT_WINDAMPLITUDE = 0.05f;
   const Ogre::Real Cloth::DEFAULT_WINDWAVELENGTH = 25.0f;
   const Ogre::Real Cloth::DEFAULT_WINDFORCEMULTIPLIER = 1.0f;

   VertexData::VertexData() : 
      PhysicsEnabled(VertexData::DEFAULT_PHYSICSENABLED)
      , Fixed(VertexData::DEFAULT_FIXED)
      , AffectedByForces(VertexData::DEFAULT_AFFECTEDBYFORCES)
      , Rigidness(VertexData::DEFAULT_RIGIDNESS)
      , RigidnessMultiplier(VertexData::DEFAULT_RIGIDNESSMULTIPLIER)
      , Gravity(VertexData::DEFAULT_GRAVITY)
      , NormalPlaneCollision(VertexData::DEFAULT_NORMALPLANECOLLISION)
      , NormalPlaneCollisionDistance(VertexData::DEFAULT_NORMALPLANECOLLISIONDISTANCE)
      , WindFlutter(VertexData::DEFAULT_WINDFLUTTER)
      , ForceMultiplier(VertexData::DEFAULT_FORCEMULTIPLIER)
      , Welded(false)
      , WeldedTo(0)
   {
   }

   Cloth::Cloth(Ogre::ResourceManager* creator, const Ogre::String &name, 
                    Ogre::ResourceHandle handle, const Ogre::String &group, bool isManual, 
                    Ogre::ManualResourceLoader *loader) : 
      Ogre::Resource(creator, name, handle, group, isManual, loader)
      , ClothVertices(0)
      , VertexCount(0)
      , SimIterations(1)
      , ClothIndices(0)
      , IndexCount(0)
      , ClothVerticesFixed(0)
      , ClothVerticesPrevFrame(0)
      , ParentNode(0)
      , ClothNormals(0)
      , WindAngle(0)
      , WindAmplitude(DEFAULT_WINDAMPLITUDE)
      , WindWaveLength(DEFAULT_WINDWAVELENGTH)
      , WindForceMultiplier(DEFAULT_WINDFORCEMULTIPLIER)
      , WeldVertices(true)
      , TotalForce(Ogre::Vector3::ZERO)
      , TotalAbsoluteForce(Ogre::Vector3::ZERO)
      , ClothVertexSize(0)
      , CollisionEntity(0)
   {
      createParamDictionary("Cloth");
   }

   Cloth::~Cloth()
   {
      unload();
      clean();
   }

   void Cloth::clean()
   {
      if (Thread)
      {
         Thread->removeCloth(this);
      }

      if (ClothNormals)
      {
         delete [] ClothNormals;
         ClothNormals = 0;
      }
      if (ClothVertices)
      {
         delete [] ClothVertices;
         ClothVertices = 0;
         VertexCount = 0;
      }
      if (ClothVerticesFixed)
      {
         delete [] ClothVerticesFixed;
         ClothVerticesFixed = 0;
      }
      if (ClothVerticesPrevFrame)
      {
         delete [] ClothVerticesPrevFrame;
         ClothVerticesPrevFrame = 0;
      }
      if (ClothIndices)
      {
         delete [] ClothIndices;
         ClothIndices = 0;
         IndexCount = 0;
      }
      //if (Thread)
      //{
      //   delete Thread;
      //   Thread = 0;
      //}
      VerticesData.clear();
      ConstraintsList.clear();
      Collision.VertexA.clear();
      Collision.VertexB.clear();
      Collision.VertexC.clear();

      WindAmplitude = DEFAULT_WINDAMPLITUDE;
      WindWaveLength = DEFAULT_WINDWAVELENGTH;
      WindForceMultiplier = DEFAULT_WINDFORCEMULTIPLIER;
      WeldVertices = true;
      TotalForce = Ogre::Vector3::ZERO;
      TotalAbsoluteForce = Ogre::Vector3::ZERO;
      MeshName.clear();

  //    VertexBuffer.setNull();
   }

   ClothPtr Cloth::clone(const Ogre::String &newName, const Ogre::String &newGroup)
   {
      Ogre::String theGroup;
      if (newGroup == Ogre::StringUtil::BLANK)
      {
         theGroup = this->getGroup();
      }
      else
      {
         theGroup = newGroup;
      }

      ClothPtr newCloth = ClothManager::getSingleton().create(newName, theGroup);

      newCloth->VerticesData = VerticesData;
      newCloth->ConstraintsList = ConstraintsList;

      newCloth->SimIterations = SimIterations;
      newCloth->WeldVertices = WeldVertices;

      newCloth->WindAmplitude = WindAmplitude;
      newCloth->WindForceMultiplier = WindForceMultiplier;
      newCloth->WindWaveLength = WindWaveLength;

      return newCloth;
   }

   void Cloth::loadImpl()
   {
      clean();
      ClothSerializer serializer;
      Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(mName, mGroup, true, this);
      serializer.importCloth(stream, this);
   }

   void Cloth::unloadImpl()
   {
      clean();

   }

   size_t Cloth::calculateSize() const
   {
       return 0;
   }

   // static
   void Cloth::initClass()
   {
      OgreMath = new Ogre::Math(); // deal with this
      Thread = new ClothThread();
      Thread->start();
   }

   // static
   void Cloth::deinitClass()
   {
      if (OgreMath)
      {
         delete OgreMath;
         OgreMath = 0;
      }
      if (Thread)
      {
         Thread->shutdown();
         delete Thread;
         Thread = 0;
      }
   }
   

   void Cloth::startSimulation()
   {
      if (ClothMesh.isNull())
      {
         try
         {
            ClothMesh = Ogre::MeshManager::getSingleton().load(MeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
         } catch(Ogre::Exception e)
         {
            Ogre::LogManager::getSingleton().logMessage("Failed to load mesh: " + MeshName + " for cloth " + getName() + ".");
         }
      }

      if (ClothMesh.isNull() == false)
      {
         preprocessPhysics(ClothMesh, false);

//      clean();
         Thread->addCloth(this);
      }

   }

   void Cloth::prepare(bool includeSubmeshes)
   {
//      clean();

      if (ClothMesh.isNull())
      {
         ClothMesh =  Ogre::MeshManager::getSingleton().getByName(MeshName);
         if (ClothMesh.isNull())
         {
            try
            {
               ClothMesh = Ogre::MeshManager::getSingleton().load(MeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::HardwareBuffer::HBU_DISCARDABLE);
            } catch(Ogre::Exception e)
            {
               Ogre::LogManager::getSingleton().logMessage("Failed to load mesh: " + MeshName + " for cloth " + getName() + ".");
            }
         }
      }

      if (ClothMesh.isNull() == false)
         preprocessPhysics(ClothMesh, includeSubmeshes);
   }

   void Cloth::setMesh(const Ogre::MeshPtr &mesh)
   {   
//      clean();
      ClothMesh = mesh;

      MeshName = mesh->getName();

//      if (mesh.isNull() == false)
//         preprocessPhysics(mesh);
   }

   void Cloth::setMeshName(const Ogre::String &mesh)
   {
      MeshName = mesh;
   }

   Ogre::MeshPtr &Cloth::getMesh()
   {
      return ClothMesh;
   }

   const Ogre::String &Cloth::getMeshName() const
   {
      return MeshName;
   }

   void Cloth::setParentNode(const Ogre::SceneNode *node)
   {
      this->ParentNode = node;
   }

   void Cloth::setCollisionMesh(const Ogre::MeshPtr &mesh)
   {
      CollisionEntity = 0;
      CollisionMesh = mesh;
      generateCollisionData();
   }

   void Cloth::createCollisionShape(Ogre::SceneManager *sceneManager, Ogre::SceneNode *node)
   {
      HeadCollision.createDebugShape(sceneManager);
//      HeadCollision.attach(node, Ogre::Quaternion::IDENTITY, Ogre::Vector3(0, -0.1f, -0.18f));
      HeadCollision.attach(node, Ogre::Quaternion::IDENTITY, Ogre::Vector3(0, -0.8f, -0.20f));
//      HeadCollision.setSize(Ogre::Vector3(0.25f, 1.f, 0.25f));
      HeadCollision.setRadius(Ogre::Vector3(0.45f, 0.2f, 0.25f));
      HeadCollision.setHeight(1.7f);
      HeadCollision.setDebugShapeVisible(false);
   }

   //void Cloth::createCollisionShape(Ogre::SceneManager *sceneManager, Ogre::Entity *entity, const Ogre::String &bone)
   //{
   //   createCollisionShape(sceneManager);
   ////   HeadCollision.attach(entity, bone, Ogre::Quaternion(Ogre::Degree(90.f), Ogre::Vector3(0, 0, 1)), Ogre::Vector3(0.12f, 0, 0));
   //   HeadCollision.setSize(Ogre::Vector3(0.11f, 0.12f, 0.11f));
   //}

   void Cloth::setCollisionEntity(Ogre::Entity *entity)
   {
      CollisionMesh = Ogre::MeshPtr();
      CollisionEntity = entity;
      if (entity)
         CollisionEntity->addSoftwareAnimationRequest(false);
   }

   void Cloth::preprocessPhysics(const Ogre::MeshPtr &mesh, bool includeSubmeshes)
   {
      Ogre::LogManager::getSingleton().logMessage("Preprocessing cloth physics for mesh: " + mesh->getName());
      getMeshData(mesh, includeSubmeshes);
//      generateConstraints();
   }

   void Cloth::preparePhysics()
   {
      generateConstraints();
   }

   bool Cloth::isReady()
   {
      return (!ConstraintsList.empty());
   }

   void Cloth::generateCollisionDataOld(const Ogre::MeshPtr &mesh)
   {
      assert(mesh.isNull() == false);
      
      Ogre::SubMesh *subMesh = mesh->getSubMesh(0);
      size_t vertex_count = 0;

      if(subMesh->useSharedVertices)
         vertex_count += mesh->sharedVertexData->vertexCount;
	   else
         vertex_count += subMesh->vertexData->vertexCount;
	   
	   Ogre::VertexData* vertex_data = subMesh->useSharedVertices ? mesh->sharedVertexData : subMesh->vertexData;
	   const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
      const Ogre::VertexElement* normalElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
      
      Ogre::HardwareVertexBufferSharedPtr vbuffer = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
      Ogre::HardwareVertexBufferSharedPtr vbufferNormal = vertex_data->vertexBufferBinding->getBuffer(normalElem->getSource());

      size_t bufferSize = vertex_count * vbuffer->getVertexSize();
      size_t bufferSizeNormals = vertex_count * vbufferNormal->getVertexSize();
      assert(bufferSize == vbuffer->getSizeInBytes());

      float *vertices = new float[bufferSize];
      float *normals = new float[bufferSizeNormals];
      vbuffer->readData(0, bufferSize, vertices);
      vbufferNormal->readData(0, bufferSizeNormals, normals);

      Ogre::Real closestDistance = 2.0;
      Ogre::Vector3 closest_vertex;
      size_t closestIndex = -1;
      size_t n, k;
      for (n=0 ; n<VertexCount ; ++n)
      {
         Ogre::Vector3 clothVertex(ClothVerticesFixed[n*ClothVertexSize], ClothVerticesFixed[n*ClothVertexSize+1], ClothVerticesFixed[n*ClothVertexSize+2]);
      
         for (k=0 ; k<vertex_count ; ++k)
         {
            Ogre::Vector3 meshVertex(vertices[k*ClothVertexSize], vertices[k*ClothVertexSize+1], vertices[k*ClothVertexSize+2]);
            Ogre::Real dist = clothVertex.distance(meshVertex);
            if (dist < closestDistance)
            {
               closestDistance = dist;
               closestIndex = k;
               closest_vertex = meshVertex;
            }
         }

         if (closestDistance < 2.0)
         {
//            Ogre::Vector3 normal(normals[k*8+6], normals[k*8+7], normals[k*8+5]);
//            Ogre::Vector3 normal(vertices[closestIndex*24+4], vertices[closestIndex*24+5], vertices[closestIndex*24+7]);
//            Ogre::Vector3 normal(vertices[closestIndex*8], vertices[closestIndex*8+1], vertices[closestIndex*8+2]);
            Ogre::Vector3 normal(vertices[closestIndex*8+3], vertices[closestIndex*8+4], vertices[closestIndex*8+5]);
            Ogre::Plane collisionPlane(normal, closest_vertex.length());
//            VerticesData[n].CollisionPlane = collisionPlane;
         }
      }
      
      delete [] vertices;
      delete [] normals;
   }

   void Cloth::reform()
   {
      memcpy(ClothVertices, ClothVerticesFixed, VertexCount * 8 * 4);
      VertexBuffer->writeData(0, VertexBuffer->getSizeInBytes(), ClothVertices, true);
   }

   void Cloth::getMeshData(const Ogre::MeshPtr &mesh, bool includeSubmeshes)
   {
      size_t vertex_count = 0;

      unsigned short maxSubmesh = includeSubmeshes ? mesh->getNumSubMeshes() : 1;
      unsigned short n;
      size_t bufferTotalSize = 0;
      size_t indexBufferTotalSize = 0;
      size_t bufferOffset = 0;
      size_t index_offset = 0;
      for (n=0 ; n<maxSubmesh ; ++n)
      {
         Ogre::SubMesh *clothMesh = mesh->getSubMesh(n);
         Ogre::VertexData* vertex_data = clothMesh->useSharedVertices ? mesh->sharedVertexData : clothMesh->vertexData;

	      const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
         VertexBuffer = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
         bufferTotalSize += VertexBuffer->getSizeInBytes();

         Ogre::IndexData* index_data = clothMesh->indexData;
         indexBufferTotalSize += index_data->indexCount;
      }

      ClothVertices = new float[bufferTotalSize];
      ClothVerticesFixed = new float[bufferTotalSize];
      ClothVerticesPrevFrame = new float[bufferTotalSize];
      ClothIndices = new unsigned long[indexBufferTotalSize];

      for (n=0 ; n<maxSubmesh ; ++n)
      {
         Ogre::SubMesh *clothMesh = mesh->getSubMesh(n);

         if (!clothMesh)
            return;

         
   //      size_t index_count = 0;

         //size_t n;
         //for (n=0 ; n<mesh->getNumSubMeshes() ; ++n)
         //{
         //   clothMesh = mesh->getSubMesh(n);

         if(clothMesh->useSharedVertices)
            vertex_count += mesh->sharedVertexData->vertexCount;
	      else
            vertex_count += clothMesh->vertexData->vertexCount;

   //      index_count += clothMesh->indexData->indexCount;

   //      ClothVertices = new float[vertex_count * 3 * 100];
   //      ClothVertices = new float[vertex_count * 8 * 4];
   	   
	      Ogre::VertexData* vertex_data = clothMesh->useSharedVertices ? mesh->sharedVertexData : clothMesh->vertexData;
	      const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
   //      const Ogre::VertexElement* normalElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
   //
   //      // Get normals first
   //      Ogre::HardwareVertexBufferSharedPtr nbuf =
   //        vertex_data->vertexBufferBinding->getBuffer(normalElem->getSource());
   //
   //      unsigned char* normal =
   //        static_cast<unsigned char*>(nbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
   //
   //      ClothNormals = new Ogre::Vector3[vertex_count];
   //      float* pReal;
   //      size_t j;
   //      for( j = 0; j < vertex_data->vertexCount; ++j, normal += nbuf->getVertexSize())
   //      {
   //        normalElem->baseVertexPointerToElement(normal, &pReal);
   //
   //        Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
   //
   ////        Normals[j] = (orient * (pt * scale)) + position;
   //        ClothNormals[j] = pt;
   //        Ogre::LogManager::getSingleton().logMessage("Normal x: " + Ogre::StringConverter::toString(ClothNormals[j].x) +
   //                                                    "  Normal y: " + Ogre::StringConverter::toString(ClothNormals[j].y) + 
   //                                                    "  Normal z: " + Ogre::StringConverter::toString(ClothNormals[j].z));
   //      }
   //      nbuf->unlock();
         
         VertexBuffer = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

   //      size_t bufferSize = vertex_count * VertexBuffer->getVertexSize();
         size_t bufferSize = VertexBuffer->getSizeInBytes();
         assert(bufferSize == VertexBuffer->getSizeInBytes());

         ClothVertexSize = VertexBuffer->getVertexSize() / 4;

         //ClothVertices = new float[bufferSize];
         //ClothVerticesFixed = new float[bufferSize];
         //ClothVerticesPrevFrame = new float[bufferSize];

         VertexBuffer->readData(0, bufferSize, &ClothVertices[bufferOffset / sizeof(float)]);
         VertexBuffer->readData(0, bufferSize, &ClothVerticesFixed[bufferOffset / sizeof(float)]);
         VertexBuffer->readData(0, bufferSize, &ClothVerticesPrevFrame[bufferOffset / sizeof(float)]);
         bufferOffset += bufferSize;
               

         VertexCount = vertex_count;

         
         //
         //delete [] clothVertices;

         Ogre::IndexData* index_data = clothMesh->indexData;

         size_t index_count = index_data->indexCount;
         size_t numTris = index_count / 3;
   //      ClothIndices = new unsigned long[index_count];

         Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

         bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

         unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
         unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

          //    size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

         if ( use32bitindexes )
         {
            size_t k;
            for ( k = 0; k < numTris*3; ++k)
            {
               ClothIndices[index_offset++] = pLong[k];// + static_cast<unsigned long>(offset);
            }
         }
         else
         {
            size_t k;
            for (k = 0; k < numTris*3; ++k)
            {
               ClothIndices[index_offset++] = static_cast<unsigned long>(pShort[k]);// + static_cast<unsigned long>(offset);
            }
         }

   //      size_t k;
   //      for (k=0 ; k<std::min((size_t)40, index_count); k += 3)
   //      {
   //         if (!use32bitindexes)
   //         {
   //         unsigned short temp = pShort[k+1];
   //         pShort[k+1] = pShort[k+2];
   //         pShort[k+2] = temp;
   //
   ////         unsigned long temp = ClothIndices[k+1];
   ////         ClothIndices[k+1] = ClothIndices[k+2];
   ////         ClothIndices[k+2] = temp;
   //         } else
   //         {
   //            unsigned long temp = pLong[k+1];
   //            pLong[k+1] = pLong[k+2];
   //            pLong[k+2] = temp;
   //         }
   //      }

         ibuf->unlock();

         IndexCount += index_count;
      }

      if (VerticesData.empty() == false && VerticesData.size() != vertex_count)
      {
         Ogre::LogManager::getSingleton().logMessage("Warning: Cloth vertice data does not match mesh vertices, wiping vertex data.");
         VerticesData.clear();
      }
      if (VerticesData.empty())
      {
         Ogre::LogManager::getSingleton().logMessage("Generating cloth vertex data.");
         VerticesData.reserve(vertex_count);
         size_t n;
         for (n=0 ; n<vertex_count ; ++n)
         {
            VertexData data;

            VerticesData.push_back(data);
         }
      }
   }

   void Cloth::generateConstraints()
   {

    //  Ogre::IndexData* index_data = clothMesh->indexData;

    //  size_t index_count = index_data->indexCount;
    //  size_t numTris = index_count / 3;
    //  unsigned long *indices = new unsigned long[index_count];

    //  Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

    //  bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

    //  unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
    //  unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

	   //size_t index_offset = 0;
    //   //    size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

    //  if ( use32bitindexes )
    //  {
    //     size_t k;
    //     for ( k = 0; k < numTris*3; ++k)
    //     {
    //        indices[index_offset++] = pLong[k];// + static_cast<unsigned long>(offset);
    //     }
    //  }
    //  else
    //  {
    //     size_t k;
    //     for (k = 0; k < numTris*3; ++k)
    //     {
    //        indices[index_offset++] = static_cast<unsigned long>(pShort[k]);// + static_cast<unsigned long>(offset);
    //     }
    //  }

    //  ibuf->unlock();

      Ogre::LogManager::getSingleton().logMessage("Generating cloth constraints.");
      ConstraintsList.clear();
      size_t k;
      for (k=0 ; k<IndexCount; k += 3)
      {
         createConstraint(k, k+1);
         createConstraint(k, k+2);
         createConstraint(k+1, k+2);
	   }

      containSeams();

	 //  delete [] indices;
   }

   void Cloth::createConstraint(size_t v1, size_t v2)
   {
      if (VerticesData[ClothIndices[v1]].PhysicsEnabled == true && VerticesData[ClothIndices[v2]].PhysicsEnabled == true)
      {
         Constraint c;
         c.particleA = ClothIndices[v1];
         c.particleB = ClothIndices[v2];
         Ogre::Vector3 vertA = Ogre::Vector3(ClothVertices[ClothIndices[v1]*ClothVertexSize], ClothVertices[ClothIndices[v1]*ClothVertexSize+1], ClothVertices[ClothIndices[v1]*ClothVertexSize+2]);
         Ogre::Vector3 vertB = Ogre::Vector3(ClothVertices[ClothIndices[v2]*ClothVertexSize], ClothVertices[ClothIndices[v2]*ClothVertexSize+1], ClothVertices[ClothIndices[v2]*ClothVertexSize+2]);
         c.restlength = vertA.distance(vertB);
////            c.restlength = Vertices[k].distance(Vertices[k+1]);
         if (c.restlength > std::numeric_limits<Ogre::Real>::epsilon())
         {
            // Very slow, but no biggie since constraints should always be pre-generated
            if (std::find(ConstraintsList.begin(), ConstraintsList.end(), c) == ConstraintsList.end())
               ConstraintsList.push_back(c);
         }
      }
   }
   
   void Cloth::containSeams()
   {
      if (!WeldVertices)
         return;

      size_t v1;
      for (v1=0 ; v1<VertexCount ; ++v1)
      {
         Ogre::Vector3 vertA = Ogre::Vector3(ClothVertices[v1*ClothVertexSize], ClothVertices[v1*ClothVertexSize+1], ClothVertices[v1*ClothVertexSize+2]);
         size_t v2;
         for (v2=v1+1 ; v2<VertexCount ; ++v2)
         {
            Ogre::Vector3 vertB = Ogre::Vector3(ClothVertices[v2*ClothVertexSize], ClothVertices[v2*ClothVertexSize+1], ClothVertices[v2*ClothVertexSize+2]);
            if ( Ogre::Math::RealEqual(vertA.x, vertB.x) &&
                 Ogre::Math::RealEqual(vertA.y, vertB.y) &&
                 Ogre::Math::RealEqual(vertA.z, vertB.z) )
            {
//               Ogre::LogManager::getSingleton().logMessage("Seam found! " + Ogre::StringConverter::toString(v1));
               if (VerticesData[v1].Welded == false && VerticesData[v2].Welded == false)
               {
                  VerticesData[v2].Welded = true;
                  VerticesData[v2].WeldedTo = (Ogre::uint32)v1;
               }
            }
         }
      }
   }

   void Cloth::setForce(const Ogre::Vector3 &force)
   {
      if (ParentNode)
      {
         Ogre::Vector3 appForce = Ogre::Vector3(-force.y, force.z, -force.x);
         //Ogre::Vector3 appForce = Ogre::Vector3(-force.x, force.z, -force.y);
         //Ogre::LogManager::getSingleton().logMessage("before x: " + Ogre::StringConverter::toString(appForce.x) +
         //                                            "  y: " + Ogre::StringConverter::toString(appForce.y) +
         //                                            "  z: " + Ogre::StringConverter::toString(appForce.z));


   //      force = ParentNode->getOrientation().Inverse() * force;
   //      TotalForce = ParentNode->_getDerivedOrientation() * force;
   //      Ogre::Radian yaw = ParentNode->getWorldOrientation().getYaw();
         Ogre::Radian roll = ParentNode->getWorldOrientation().getRoll();
         roll += Ogre::Radian(Ogre::Math::HALF_PI);
   //      Ogre::Radian pitch = ParentNode->getWorldOrientation().getPitch();
         Ogre::Quaternion quat(roll, Ogre::Vector3::UNIT_Y);
         
   //      Ogre::LogManager::getSingleton().logMessage("roll: " + Ogre::StringConverter::toString(roll));

         //Ogre::LogManager::getSingleton().logMessage("yaw: " + Ogre::StringConverter::toString(yaw) +
         //                                            "roll: " + Ogre::StringConverter::toString(roll) +
         //                                            "pitch: " + Ogre::StringConverter::toString(pitch));

         TotalForce = quat.Inverse() * appForce;
         //Ogre::LogManager::getSingleton().logMessage("after x: " + Ogre::StringConverter::toString(TotalForce.x) +
         //                                            "  y: " + Ogre::StringConverter::toString(TotalForce.y) +
         //                                            "  z: " + Ogre::StringConverter::toString(TotalForce.z));
         //Ogre::LogManager::getSingleton().logMessage("");

   //      TotalForce = Ogre::Vector3(TotalForce.z, TotalForce.y, TotalForce.x);
      } else
      {
         TotalForce = force;
      }
   }

   void Cloth::setAbsoluteForce(const Ogre::Vector3 &force)
   {
      if (ParentNode)
      {
         Ogre::Vector3 appForce = Ogre::Vector3(-force.y, force.z, -force.x);

         Ogre::Radian roll = ParentNode->getWorldOrientation().getRoll();
         roll += Ogre::Radian(Ogre::Math::HALF_PI);
         Ogre::Quaternion quat(roll, Ogre::Vector3::UNIT_Y);


         TotalAbsoluteForce = quat.Inverse() * appForce;
      } else
      {
         TotalAbsoluteForce = force;
      }
   }

   void Cloth::addTime(Ogre::Real time, bool applyforce)
   {
      if (!VertexCount || !smRenderPhysics)
         return;


//      static int iter = 0;
//      iter++;
//      if (iter < 100)
//      {
////         firstTime = false;
//         reform(); // Work-around for cloth physics on occasion appearing completely out of shape at the beginning
//      }

#ifndef REX_THREADED
      applyConstraints();
      applyForceAndModifiers(time, applyforce);      
      recalculateNormals();
#endif

      generateCollisionData();


      // Write out vertex data after applying all constraints
      Thread->lockVertexMutex(this);
      VertexBuffer->writeData(0, VertexBuffer->getSizeInBytes(), ClothVertices, true);
      Thread->unlockVertexMutex(this);
   }

   void Cloth::applyConstraints()
   {
      unsigned int n;
      size_t k;
      for (n=0 ; n<SimIterations ; ++n)
      {
         for (k=0 ; k<ConstraintsList.size() ; ++k)
         {
            
            Constraint& c = ConstraintsList[k];
            Ogre::Vector3 x1 = Ogre::Vector3(ClothVertices[c.particleA*ClothVertexSize], ClothVertices[c.particleA*ClothVertexSize+1], ClothVertices[c.particleA*ClothVertexSize+2]);
            Ogre::Vector3 x2 = Ogre::Vector3(ClothVertices[c.particleB*ClothVertexSize], ClothVertices[c.particleB*ClothVertexSize+1], ClothVertices[c.particleB*ClothVertexSize+2]);

            Ogre::Vector3 delta(x2-x1);

            delta *= (c.restlength * c.restlength) / (delta.dotProduct(delta) + c.restlength * c.restlength) - 0.5f;

            x1 -= VerticesData[c.particleA].Fixed ? Ogre::Vector3::ZERO : delta;
            x2 += VerticesData[c.particleB].Fixed ? Ogre::Vector3::ZERO : delta;
//            x1 -= delta;
//            x2 += delta;


            ClothVertices[c.particleA*ClothVertexSize] = x1.x;
            ClothVertices[c.particleA*ClothVertexSize+1] = x1.y;
            ClothVertices[c.particleA*ClothVertexSize+2] = x1.z;
            ClothVertices[c.particleB*ClothVertexSize] = x2.x;
            ClothVertices[c.particleB*ClothVertexSize+1] = x2.y;
            ClothVertices[c.particleB*ClothVertexSize+2] = x2.z;
         }
      }
   }

   void Cloth::applyForceAndModifiers(Ogre::Real time, bool applyforce)
   {
      WindAngle += Ogre::Radian(3.5f * time);
      Ogre::Real currentAmplitude = WindAmplitude;
      if (WindAngle > Ogre::Radian(Ogre::Math::TWO_PI))
      {
//         WindAmplitude = Ogre::Math::RangeRandom(0.01f, 0.1f);
//         WindAmplitude = Ogre::Math::RangeRandom(0.05f, 0.08f);
         currentAmplitude = Ogre::Math::RangeRandom(std::max(0.001f, WindAmplitude - 0.03f), WindAmplitude + 0.03f);
         WindAngle = 0;
      }

      bool isAffectedByForce = (TotalForce.squaredLength() > 0.00001f || TotalAbsoluteForce.squaredLength() > 0.00001f) ? true : false;

//      Ogre::Real currentAmplitude = WindAmplitude;
      if (isAffectedByForce)
      {
         currentAmplitude *= WindForceMultiplier;
      }

//      Ogre::Real randomFluctuation = Ogre::Math::RangeRandom(0.2f, 1.4f);

      size_t k;
      for (k=0 ; k<VertexCount ; ++k)
      {
         if (VerticesData[k].PhysicsEnabled == true && VerticesData[k].Fixed == false)
         {
            if (VerticesData[k].Welded == true)
            {
               ClothVertices[k*ClothVertexSize] = ClothVertices[VerticesData[k].WeldedTo*ClothVertexSize];
               ClothVertices[k*ClothVertexSize+1] = ClothVertices[VerticesData[k].WeldedTo*ClothVertexSize+1];
               ClothVertices[k*ClothVertexSize+2]  = ClothVertices[VerticesData[k].WeldedTo*ClothVertexSize+2];
            } else
            {

   //            if (!applyforce)
   ////            ClothVertices[k*8+1] -= 1.2f * time;
   //               ClothVertices[k*8+1] -= 0.4f * time;
   //            else

               if (VerticesData[k].WindFlutter)
               {
////                  ClothVertices[k*ClothVertexSize+2] += WindAmplitude * time * VerticesData[k].ForceMultiplier * (1.0f + Ogre::Math::Sin(WindAngle + Ogre::Radian(ClothVertices[k*ClothVertexSize] * WindWaveLength), true));
                  ClothVertices[k*ClothVertexSize+2] += currentAmplitude * time * (Ogre::Math::Sin(WindAngle + Ogre::Radian(ClothVertices[k*ClothVertexSize] * WindWaveLength), true));
//                  ClothVertices[k*ClothVertexSize+2] += 0.003f * currentAmplitude * (Ogre::Math::Sin(WindAngle + Ogre::Radian(ClothVertices[k*ClothVertexSize] * WindWaveLength), true));
               }

               Ogre::Vector3 vertex(ClothVertices[k*ClothVertexSize], ClothVertices[k*ClothVertexSize+1], ClothVertices[k*ClothVertexSize+2]);
               Ogre::Vector3 vertexNormal(ClothVertices[k*ClothVertexSize+3], ClothVertices[k*ClothVertexSize+4], ClothVertices[k*ClothVertexSize+5]);
//               vertexNormal.normalise();
               Ogre::Vector3 vertexOriginal(ClothVerticesFixed[k*ClothVertexSize], ClothVerticesFixed[k*ClothVertexSize+1], ClothVerticesFixed[k*ClothVertexSize+2]);
               Ogre::Vector3 vertexOriginalNormal(ClothVerticesFixed[k*ClothVertexSize+3], ClothVerticesFixed[k*ClothVertexSize+4], ClothVerticesFixed[k*ClothVertexSize+5]);
               Ogre::Vector3 vertexPrevFrame(ClothVerticesPrevFrame[k*ClothVertexSize], ClothVerticesPrevFrame[k*ClothVertexSize+1], ClothVerticesPrevFrame[k*ClothVertexSize+2]);

//               Ogre::Plane collisionPlane(vertexOriginalNormal, vertexOriginal.length());
               Ogre::Plane collisionPlane(vertexOriginalNormal, vertexOriginal - vertexOriginalNormal * VerticesData[k].NormalPlaneCollisionDistance);
               if (VerticesData[k].AffectedByForces)// && (VerticesData[k].NormalPlaneCollision == false || collisionPlane.getSide(vertex) == Ogre::Plane::POSITIVE_SIDE))
               {
                  vertex.x += TotalForce.x * VerticesData[k].ForceMultiplier;// * randomFluctuation;
                  vertex.y += TotalForce.y * VerticesData[k].ForceMultiplier;// * randomFluctuation;
                  vertex.z += TotalForce.z * VerticesData[k].ForceMultiplier;// * randomFluctuation * ClothVertices[k*ClothVertexSize];

                  vertex.x += TotalAbsoluteForce.x * VerticesData[k].ForceMultiplier;// / time * 0.1f;// * randomFluctuation;
                  vertex.y += TotalAbsoluteForce.y * VerticesData[k].ForceMultiplier;// / time * 0.1f;// * randomFluctuation;
                  vertex.z += TotalAbsoluteForce.z * VerticesData[k].ForceMultiplier;// / time * 0.1f;// * randomFluctuation * ClothVertices[k*ClothVertexSize];


//                  ClothVertices[k*ClothVertexSize]   += TotalForce.x * time * VerticesData[k].ForceMultiplier * Ogre::Math::RangeRandom(0.2f, 2.3f);
//                  ClothVertices[k*ClothVertexSize+1] += TotalForce.y * time * VerticesData[k].ForceMultiplier * Ogre::Math::RangeRandom(0.2f, 2.3f);
//                  ClothVertices[k*ClothVertexSize+2] += TotalForce.z * time * VerticesData[k].ForceMultiplier * Ogre::Math::RangeRandom(0.2f, 2.3f);
                  if (applyforce)
                  {
                     vertex.x -= 1.4f * time * VerticesData[k].ForceMultiplier;
      //               ClothVertices[k*ClothVertexSize+1] += 0.5f * time;
                  }
                  //ClothVertices[k*ClothVertexSize+2] += 0.05f * time * VerticesData[k].ForceMultiplier * (1.0f + Ogre::Math::Sin(WindAngle + Ogre::Radian(ClothVertices[k*ClothVertexSize] * 14.0f), true));
               }

//              old
//               if (fabs(VerticesData[k].Rigidness) < 0.00001f)
//               {
                  // Gravity
//                  ClothVertices[k*ClothVertexSize+1] -= 0.6f * time;
//               }

               //Ogre::Vector3 vertex(ClothVertices[k*ClothVertexSize], ClothVertices[k*ClothVertexSize+1], ClothVertices[k*ClothVertexSize+2]);
               //Ogre::Vector3 vertexNormal(ClothVertices[k*ClothVertexSize]+3, ClothVertices[k*ClothVertexSize]+4, ClothVertices[k*ClothVertexSize]+5);
               //vertexNormal.normalise();
               //Ogre::Vector3 vertexOriginal(ClothVerticesFixed[k*ClothVertexSize], ClothVerticesFixed[k*ClothVertexSize+1], ClothVerticesFixed[k*ClothVertexSize+2]);
               
               // Gravity
               vertex += VerticesData[k].Gravity * time;
               
               Ogre::Vector3 dist = vertexOriginal - vertex;
               
//               Ogre::Real forceMultiplier = 0.03f * time;
//               Ogre::Real forceMultiplier = 7.f * time;
               Ogre::Real forceMultiplier = VerticesData[k].Rigidness * time;

//               // Collisions
//               Ogre::Vector3 collisionNormal = vertexNormal.normalisedCopy();
//               Ogre::Ray collisionRay(vertex, collisionNormal);
//               size_t n;
//               Thread->CollisionMutex->lock();
//////               for (n=0 ; n<Collision.Polygons[0].size() ; ++n)
//               Ogre::Real shortestDistance = 100.0f;
//               Ogre::Vector3 polyVertexA;
//               Ogre::Vector3 polyVertexB;
//               Ogre::Vector3 polyVertexC;
//               for (n=0 ; n<Collision.VertexA.size() ; ++n)
//               {
//                  std::pair<bool, Ogre::Real> result = Ogre::Math::intersects(collisionRay, Collision.VertexA[n], Collision.VertexB[n], Collision.VertexC[n], false, true);
//                  if (result.first)
//                  {
//                     if (result.second < shortestDistance)
//                     {
//                        polyVertexA = Collision.VertexA[n];
//                        polyVertexB = Collision.VertexB[n];
//                        polyVertexC = Collision.VertexC[n];
//
//                        shortestDistance = result.second;
//                     }
//
////                     shortestDistance = std::min(shortestDistance, result.second);
//                     
////                     vertex -= collisionNormal * result.second;
//////                     vertex += vertexNormal * 0.06f;
////                     break;
////
//                  //   Ogre::LogManager::getSingleton().logMessage("collision distance: " + Ogre::StringConverter::toString(result.second));
////                     break;
//                  }
//               }
////               if (shortestDistance != 100.0f)
//               if (shortestDistance < 0.1f)
//               {
//                  Ogre::Vector3 polyNormal = Ogre::Math::calculateBasicFaceNormal(polyVertexA, polyVertexB, polyVertexC);
//                  vertex += polyNormal * shortestDistance;
////                  vertex += polyNormal * 0.06f;
//                  Ogre::LogManager::getSingleton().logMessage("collision distance: " + Ogre::StringConverter::toString(shortestDistance));
//               }
//
//               Thread->CollisionMutex->unlock();


////               if (VerticesData[k].NormalPlaneCollision)
////               {
//               Ogre::Vector3 vertexOriginalNormal(ClothVerticesFixed[k*ClothVertexSize]+3, ClothVerticesFixed[k*ClothVertexSize]+4, ClothVerticesFixed[k*ClothVertexSize]+5);
////               Ogre::Plane collisionPlane(vertexOriginalNormal, vertexOriginal + vertexOriginalNormal * VerticesData[k].NormalPlaneCollisionDistance);
//               Ogre::Plane collisionPlane(vertexOriginalNormal, vertexOriginal);
               if (VerticesData[k].NormalPlaneCollision && collisionPlane.getSide(vertex) == Ogre::Plane::NEGATIVE_SIDE)
               {
                  Ogre::Real distance = collisionPlane.getDistance(vertex);

                  vertex.x -= vertexOriginalNormal.x * distance;
                  vertex.y -= vertexOriginalNormal.y * distance;
                  vertex.z -= vertexOriginalNormal.z * distance;

                  ClothVertices[k*ClothVertexSize] = vertex.x;
                  ClothVertices[k*ClothVertexSize+1] = vertex.y;
                  ClothVertices[k*ClothVertexSize+2] = vertex.z;

                  //ClothVertices[k*ClothVertexSize] = vertexOriginal.x;
                  //ClothVertices[k*ClothVertexSize+1] = vertexOriginal.y;
                  //ClothVertices[k*ClothVertexSize+2] = vertexOriginal.z;
               
//               Ogre::Plane collisionPlane(ClothNormals[k], vertexOriginal);
//               Ogre::Plane collisionPlane(Ogre::Vector3(0, 0, 1), vertexOriginal);
//               TotalForce.normalise();
//               Ogre::Plane collisionPlane(TotalForce, 0);
//
//               if (collisionPlane.getSide(vertex) == Ogre::Plane::NEGATIVE_SIDE && TotalForce.squaredLength() > 0.0001f)
//               {
//                  forceMultiplier = 1.0f;
//               }
               } else
               {
                  if (isAffectedByForce || applyforce)
                  {
                     forceMultiplier *= VerticesData[k].RigidnessMultiplier;
//                     forceMultiplier *= 0.2f;
                  }

   //            if (TotalForce.squaredLength() < 0.00001f && !applyforce)
                  vertex += dist * forceMultiplier;
   //               else
   //                  ClothVertices[k*ClothVertexSize+1] -= 1.5f * time;
               

                  size_t n;
                  Thread->CollisionMutex->lock();
                  Ogre::Real shortestDistance = 100.0f;
                  bool collision = false;
                  Ogre::Vector3 polyVertexA;
                  Ogre::Vector3 polyVertexB;
                  Ogre::Vector3 polyVertexC;

                  size_t maxPolys = std::min(Collision.VertexA.size(), (size_t)2000);
//                  for (n=0 ; n<Collision.VertexA.size() ; ++n)
                  for (n=0 ; n<maxPolys ; ++n)
                  {
                     // Calculate triangle normal and plane
                     // We should cache these for speed
                     Ogre::Vector3 polyNormal = Ogre::Math::calculateBasicFaceNormal(Collision.VertexA[n], Collision.VertexB[n], Collision.VertexC[n]);
                     Ogre::Plane collisionPlane(Collision.VertexA[n], Collision.VertexB[n], Collision.VertexC[n]);
//                     Ogre::Sphere sphere(vertex, 0.06f);
//                     Ogre::Sphere sphere(vertex, 0.016f + vertexPrevFrame.distance(vertex) * 1.5f);

                     // We use sphere as collision object for cloth vertices. The size of the sphere changes dynamically
                     // based on the speed of the cloth vertex, to avoid penetration at high velocities.
                     // Also setting the size of the collision sphere higher avoids clipping
                     Ogre::Real collisionForceMultiplier = pow(vertexPrevFrame.distance(vertex), 0.95f);
//                     collisionForceMultiplier = collisionForceMultiplier > 0.02f ? collisionForceMultiplier : 0.0f;
                     if (collisionForceMultiplier < 0.02f)
                        collisionForceMultiplier = 0;
                     else
                        Ogre::LogManager::getSingleton().logMessage("not zero: " + Ogre::StringConverter::toString(collisionForceMultiplier));

                     Ogre::Real sphereRealRadius = 0.018f;
                     Ogre::Sphere sphere(vertex, sphereRealRadius + collisionForceMultiplier);
//                     Ogre::Sphere sphere(vertex, 0.088f);

//                     collisionRay.setDirection(polyNormal);
//                     std::pair<bool, Ogre::Real> result = Ogre::Math::intersects(collisionRay, Collision.VertexA[n], Collision.VertexB[n], Collision.VertexC[n], true, true);
                     std::pair<bool, Ogre::Real> result;
                     result.first = Ogre::Math::intersects(sphere, collisionPlane);

                     // Get distance from sphere edge to triangle
                     int side = collisionPlane.getSide(sphere.getCenter()) == Ogre::Plane::NEGATIVE_SIDE ? -1 : 1;
                     result.second = sphere.getRadius() - side * collisionPlane.getDistance(sphere.getCenter());
                     if (result.first)
                     {
                        // We only check collision against the inside of triangle, we should also test for triangle edge collisions...
                        Ogre::Vector3 projectedPoint = collisionPlane.projectVector(sphere.getCenter());
                        if (Ogre::Math::pointInTri3D(projectedPoint, Collision.VertexA[n], Collision.VertexB[n], Collision.VertexC[n], side * polyNormal))
                        {
                           if (result.second < shortestDistance)
                           {
                              polyVertexA = Collision.VertexA[n];
                              polyVertexB = Collision.VertexB[n];
                              polyVertexC = Collision.VertexC[n];

                              shortestDistance = result.second;
                              collision = true;
                           }
                           // We do actually need to check for collision against all possible triangles, but we can just use the first
                           // one for now since we don't handle it yet properly
                           break; 
                        }
                     }
                  }
//                  if (shortestDistance != 100.0f)
                  if (collision)
                  {
                     // To avoid z fighting, otherwise the vertex ends up right on the surface of the collision polygon. (Not really needed anymore.)
                  //   shortestDistance += 0.0001f;
                     Ogre::Vector3 polyNormal = Ogre::Math::calculateBasicFaceNormal(polyVertexA, polyVertexB, polyVertexC);
                     vertex += polyNormal * shortestDistance;

                  //   Ogre::LogManager::getSingleton().logMessage("collision distance: " + Ogre::StringConverter::toString(shortestDistance));
                  }

                  Thread->CollisionMutex->unlock();

              //    vertex = HeadCollision.collide(vertex, vertexPrevFrame - vertex);

            
                  ClothVertices[k*ClothVertexSize] = vertex.x;
                  ClothVertices[k*ClothVertexSize+1] = vertex.y;
                  ClothVertices[k*ClothVertexSize+2] = vertex.z;

                  ClothVerticesPrevFrame[k*ClothVertexSize] = vertex.x;
                  ClothVerticesPrevFrame[k*ClothVertexSize+1] = vertex.y;
                  ClothVerticesPrevFrame[k*ClothVertexSize+2] = vertex.z;
               }
            }

//            Ogre::Vector3 vertex(ClothVertices[k*ClothVertexSize], ClothVertices[k*ClothVertexSize+1], ClothVertices[k*ClothVertexSize+2]);
//            if (VerticesData[k].CollisionPlane.getSide(vertex) == Ogre::Plane::NEGATIVE_SIDE)
////            if (VerticesData[k].CollisionPlane.getSide(vertex) == Ogre::Plane::POSITIVE_SIDE)
//            {
//               vertex = VerticesData[k].CollisionPlane.projectVector(vertex);
//               ClothVertices[k*ClothVertexSize] = vertex.x;
//               ClothVertices[k*ClothVertexSize+1] = vertex.y;
//               ClothVertices[k*ClothVertexSize+2] = vertex.z;
//            }
         }
      }
   }

   void Cloth::recalculateNormals()
   {
      size_t n;
      for (n=0 ; n<VertexCount ; ++n)
      {
//         ClothVertices[n*ClothVertexSize+3] = 0.f;
//         ClothVertices[n*ClothVertexSize+4] = 0.f;
//         ClothVertices[n*ClothVertexSize+5] = 0.f;

         
         ClothVertices[n*ClothVertexSize+3] = ClothVerticesFixed[n*ClothVertexSize+3] * 6.f;
         ClothVertices[n*ClothVertexSize+4] = ClothVerticesFixed[n*ClothVertexSize+4] * 6.f;
         ClothVertices[n*ClothVertexSize+5] = ClothVerticesFixed[n*ClothVertexSize+5] * 6.f;
      }

      for (n=0 ; n<IndexCount; n += 3)
      {
         Ogre::Vector3 vertexA = Ogre::Vector3(ClothVertices[ClothIndices[n]*ClothVertexSize], ClothVertices[ClothIndices[n]*ClothVertexSize+1], ClothVertices[ClothIndices[n]*ClothVertexSize+2]);
         Ogre::Vector3 vertexB = Ogre::Vector3(ClothVertices[ClothIndices[n+1]*ClothVertexSize], ClothVertices[ClothIndices[n+1]*ClothVertexSize+1], ClothVertices[ClothIndices[n+1]*ClothVertexSize+2]);
         Ogre::Vector3 vertexC = Ogre::Vector3(ClothVertices[ClothIndices[n+2]*ClothVertexSize], ClothVertices[ClothIndices[n+2]*ClothVertexSize+1], ClothVertices[ClothIndices[n+2]*ClothVertexSize+2]);
      
         Ogre::Vector3 normal = (vertexB - vertexA).crossProduct(vertexC - vertexA);
         normal.normalise();
//         Ogre::Vector3 normal = Ogre::Math::calculateBasicFaceNormalWithoutNormalize(vertexA, vertexB, vertexC);
//         Ogre::Vector3 normal = Ogre::Math::calculateBasicFaceNormal(vertexA, vertexB, vertexC);
         ClothVertices[ClothIndices[n]*ClothVertexSize+3] += normal.x;
         ClothVertices[ClothIndices[n]*ClothVertexSize+4] += normal.y;
         ClothVertices[ClothIndices[n]*ClothVertexSize+5] += normal.z;

         ClothVertices[ClothIndices[n+1]*ClothVertexSize+3] += normal.x;
         ClothVertices[ClothIndices[n+1]*ClothVertexSize+4] += normal.y;
         ClothVertices[ClothIndices[n+1]*ClothVertexSize+5] += normal.z;

         ClothVertices[ClothIndices[n+2]*ClothVertexSize+3] += normal.x;
         ClothVertices[ClothIndices[n+2]*ClothVertexSize+4] += normal.y;
         ClothVertices[ClothIndices[n+2]*ClothVertexSize+5] += normal.z;
      }

      for (n=0 ; n<VertexCount ; ++n)
      {
         Ogre::Vector3 normal = Ogre::Vector3(ClothVertices[n*ClothVertexSize+3], ClothVertices[n*ClothVertexSize+4], ClothVertices[n*ClothVertexSize+5]);
         normal.normalise();

         ClothVertices[n*ClothVertexSize+3] = normal.x;
         ClothVertices[n*ClothVertexSize+4] = normal.y;
         ClothVertices[n*ClothVertexSize+5] = normal.z;
      }
   }

   void Cloth::generateCollisionData()
   {
      if (CollisionMesh.isNull() && CollisionEntity == 0)
         return;

//      assert(CollisionMesh.isNull() == false);

//      bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
//    size_t next_offset = 0;
    size_t index_offset = 0;

    size_t vertex_count = 0;
    size_t index_count = 0;


      Thread->CollisionMutex->lock();

      Collision.VertexA.clear();
      Collision.VertexB.clear();
      Collision.VertexC.clear();
//      Collision.Polygons[0].clear();
//      Collision.Polygons[1].clear();
//      Collision.Polygons[2].clear();

      size_t numSubmeshes = CollisionEntity == 0 ? CollisionMesh->getNumSubMeshes() : CollisionEntity->getNumSubEntities();
      
      size_t n;
      for (n=0 ; n<numSubmeshes ; ++n)
//      for (n=0 ; n<1 ; ++n)
      {
         Ogre::VertexData* vertex_data = 0;
         Ogre::IndexData* index_data = 0;
         if (CollisionEntity)
         {
            vertex_data = CollisionEntity->getSubEntity((unsigned short)n)->_getSkelAnimVertexData();
            index_data = CollisionEntity->getSubEntity((unsigned short)n)->getSubMesh()->indexData;
         } else
         {
            Ogre::SubMesh *subMesh = CollisionMesh->getSubMesh((unsigned short)n);
            vertex_data = subMesh->vertexData;
            index_data = subMesh->indexData;
         }
         
         size_t vertex_count = 0;
         vertex_count += vertex_data->vertexCount;

//         if(subMesh->useSharedVertices)
//            vertex_count += CollisionMesh->sharedVertexData->vertexCount;
//	      else
//            vertex_count += subMesh->vertexData->vertexCount;
   	   
//	      Ogre::VertexData* vertex_data = subMesh->useSharedVertices ? CollisionMesh->sharedVertexData : subMesh->vertexData;


	      const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
   //      const Ogre::VertexElement* normalElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
//         Ogre::LogManager::getSingleton().logMessage("Vertex elements: " + Ogre::StringConverter::toString(vertex_data->vertexDeclaration->getElementCount()));
//         Ogre::LogManager::getSingleton().logMessage("Vertex element size: " + Ogre::StringConverter::toString(posElem->getSize()));
//         Ogre::LogManager::getSingleton().logMessage("Vertex element type cnt: " + Ogre::StringConverter::toString(posElem->getTypeCount(Ogre::v)));
//         Ogre::LogManager::getSingleton().logMessage("Vertex element type size: " + Ogre::StringConverter::toString(posElem->getTypeSize(Ogre::VES_POSITION)));
//         
         
         Ogre::HardwareVertexBufferSharedPtr vbuffer = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
//         Ogre::LogManager::getSingleton().logMessage("Vertex size: " + Ogre::StringConverter::toString(vbuffer->getVertexSize()));

         size_t collisionVertexSize = vbuffer->getVertexSize() / 4;
         
//         vbuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY);
//   //      Ogre::HardwareVertexBufferSharedPtr vbufferNormal = vertex_data->vertexBufferBinding->getBuffer(normalElem->getSource());
//
//         unsigned char* vertex =
//               static_cast<unsigned char*>(vbuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
//
//
//         size_t bufferSize = vertex_count * vbuffer->getVertexSize();
         size_t bufferSize = vbuffer->getSizeInBytes();
//      Ogre::LogManager::getSingleton().logMessage("Buffer size: " + Ogre::StringConverter::toString(bufferSize));
//      Ogre::LogManager::getSingleton().logMessage("Buffer size2: " + Ogre::StringConverter::toString(bufferSize2));
//   //      size_t bufferSizeNormals = vertex_count * vbufferNormal->getVertexSize();
         assert(bufferSize == vbuffer->getSizeInBytes());

         float *vertices = new float[bufferSize];
//         vbuffer->suppressHardwareUpdate(true);
////   //      float *normals = new float[bufferSizeNormals];
//         if (vbuffer->hasShadowBuffer())
            vbuffer->readData(0, bufferSize, vertices);
////   //      vbufferNormal->readData(0, bufferSizeNormals, normals);
//         vbuffer->unlock();
//         vbuffer->suppressHardwareUpdate(false);
//         delete [] vertices;
//         return;
//         float *vertices;

//         Ogre::IndexData* index_data = subMesh->indexData;

         index_count = index_data->indexCount;
         size_t numTris = index_data->indexCount / 3;
         unsigned long *indices = new unsigned long[index_count];

         Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

         bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

         unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
         unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
         index_offset = 0;


//         size_t offset = (subMesh->useSharedVertices)? shared_offset : current_offset;
         size_t offset = current_offset;

         if ( use32bitindexes )
         {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
              indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
         }
         else
         {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
              indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
                                        static_cast<unsigned long>(offset);
            }
         }

         ibuf->unlock();

      Collision.VertexA.reserve(numTris);
      Collision.VertexB.reserve(numTris);
      Collision.VertexC.reserve(numTris);
//         Collision.Polygons[0].resize(index_count / 3);
//         Collision.Polygons[1].resize(index_count / 3);
//         Collision.Polygons[2].resize(index_count / 3);
      size_t idx;
      for (idx=0 ; idx<index_count ; idx += 3)
      {
         Ogre::Vector3 vertexA(vertices[indices[idx]*collisionVertexSize], vertices[indices[idx]*collisionVertexSize+1], vertices[indices[idx]*collisionVertexSize+2]);
         Ogre::Vector3 vertexB(vertices[indices[idx+1]*collisionVertexSize], vertices[indices[idx+1]*collisionVertexSize+1], vertices[indices[idx+1]*collisionVertexSize+2]);
         Ogre::Vector3 vertexC(vertices[indices[idx+2]*collisionVertexSize], vertices[indices[idx+2]*collisionVertexSize+1], vertices[indices[idx+2]*collisionVertexSize+2]);

//            Collision.Polygons[0][n/3] = Ogre::Vector3(vertices[clothIndices[n]*8], vertices[clothIndices[n]*8+1], vertices[clothIndices[n]*8+2]);
//            Collision.Polygons[1][n/3] = Ogre::Vector3(vertices[clothIndices[n+1]*8], vertices[clothIndices[n+1]*8+1], vertices[clothIndices[n+1]*8+2]);
//            Collision.Polygons[2][n/3] = Ogre::Vector3(vertices[clothIndices[n+2]*8], vertices[clothIndices[n+2]*8+1], vertices[clothIndices[n+2]*8+2]);
         Collision.VertexA.push_back(vertexA);
         Collision.VertexB.push_back(vertexB);
         Collision.VertexC.push_back(vertexC);

//            Ogre::LogManager::getSingleton().logMessage("vecA: " + Ogre::StringConverter::toString(vertexA) + 
//                  "  vecB: " + Ogre::StringConverter::toString(vertexB) + 
//                  "  vecC: " + Ogre::StringConverter::toString(vertexC));
      }

//         Ogre::LogManager::getSingleton().logMessage("index_count: " + Ogre::StringConverter::toString(index_count) + 
//            "vertex_count: " + Ogre::StringConverter::toString(vertex_count));
      
      
      delete [] vertices;
      delete [] indices;

      Thread->CollisionMutex->unlock();

//         Ogre::LogManager::getSingleton().logMessage(" ");
//         Ogre::LogManager::getSingleton().logMessage("next mesh");
      }
   }

   Ogre::Vector3 Cloth::getVertex(size_t index) const
   {
      if (ClothVertices && index >= 0 && index < VertexCount)
      {
         Ogre::Vector3 vertex(ClothVertices[index*ClothVertexSize], ClothVertices[index*ClothVertexSize+1], ClothVertices[index*ClothVertexSize+2]);
         return vertex;
      }
      return Ogre::Vector3::ZERO;
   }

   //Ogre::Vector3 Cloth::getVertexWorld(size_t index) const
   //{
   //   return (getVertex(index));
   //}

   size_t Cloth::getVertexCount() const
   {
      return VertexCount;
   }
   
   void Cloth::enablePhysics(size_t index, bool enable)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].PhysicsEnabled = enable;
   }

   void Cloth::setFixed(size_t index, bool fixed)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].Fixed = fixed;
   }

   void Cloth::setAffectedByForces(size_t index, bool affected)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].AffectedByForces = affected;
   }

   void Cloth::setRigidness(size_t index, Ogre::Real rigidness)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].Rigidness = rigidness;
   }

   void Cloth::setRigidnessMultiplier(size_t index, Ogre::Real multiplier)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].RigidnessMultiplier = multiplier;
   }

   void Cloth::setGravity(size_t index, const Ogre::Vector3 &gravity)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].Gravity = gravity;
   }

   void Cloth::setForceMultiplier(size_t index, Ogre::Real forceMultiplier)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].ForceMultiplier = forceMultiplier;
   }
   
   void Cloth::setNormalPlaneCollision(size_t index, bool collision)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].NormalPlaneCollision = collision;
   }

   void Cloth::setNormalPlaneCollisionDistance(size_t index, Ogre::Real distance)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].NormalPlaneCollisionDistance = distance;
   }

   void Cloth::setWindFlutter(size_t index, bool wind)
   {
      if (index >= 0 && index < VerticesData.size())
         VerticesData[index].WindFlutter = wind;
   }

   const VertexData &Cloth::getVertexData(size_t index) const
   {
      assert(index >= 0 && index < VerticesData.size());

      return VerticesData[index];
   }

   const Constraint &Cloth::getConstraint(size_t index) const
   {
      assert(index >= 0 && index < ConstraintsList.size());

      return ConstraintsList[index];
   }

   size_t Cloth::getConstraintCount() const
   {
      return ConstraintsList.size();
   }

   void Cloth::setWeldVertices(bool weld)
   {
      WeldVertices = weld;
   }

   bool Cloth::getWeldVertices() const
   {
      return WeldVertices;
   }

   void Cloth::setIterations(unsigned int iterations)
   {
      SimIterations = iterations;
   }

   unsigned int Cloth::getIterations() const
   {
      return SimIterations;
   }

   Ogre::Real Cloth::getWindAmplitude() const
   {
      return WindAmplitude;
   }

   void Cloth::setWindAmplitude(Ogre::Real amplitude)
   {
      WindAmplitude = amplitude;
   }

   Ogre::Real Cloth::getWindWaveLength() const
   {
      return WindWaveLength;
   }

   void Cloth::setWindWaveLength(Ogre::Real wavelength)
   {
      WindWaveLength = wavelength;
   }

   Ogre::Real Cloth::getWindForceMultiplier() const
   {
      return WindForceMultiplier;
   }

   void Cloth::setWindForceMultiplier(Ogre::Real windforce)
   {
      WindForceMultiplier = windforce;
   }

   // static
   void Cloth::disableAllPhysics()
   {
      setPhysicsEnabled(!smRenderPhysics);
//      smRenderPhysics = !smRenderPhysics;
//
      //if (!smRenderPhysics)
      //{
      //   Thread->pause();
      //} else
      //{
      //   Thread->unpause();
      //}
   }

   // static
   void Cloth::setPhysicsEnabled(bool enabled)
   {
      smRenderPhysics = enabled;

      if (!smRenderPhysics)
      {
         Thread->pause();
      } else
      {
         Thread->unpause();
      }
   }

   // static
   void Cloth::simulate(Ogre::Real time)
   {
    //  time *= 0.001f;
      Rex::ClothManager *manager = Rex::ClothManager::getSingletonPtr();
      Ogre::ResourceManager::ResourceMapIterator iter = manager->getResourceIterator();
      while(iter.hasMoreElements())
      {
         static_cast<ClothPtr>(iter.getNext())->addTime(time, false);
      }
   }
}


