/**
 * @file llmeshcache.cpp
 * @brief LLMeshCache class implementation
 *
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"
#include "llmeshcache.h"
#include <OGRE/Ogre.h>

using namespace Ogre;

MeshInformationCache::MeshInformationCache()
{
}

MeshInformationCache::~MeshInformationCache()
{
   clear();
}

void MeshInformationCache::clear()
{
   mCache.clear();
}

const MeshInformationCache::CacheMeshData *MeshInformationCache::getMeshInformation(
         Ogre::SubEntity *entity)
{
   MeshInformationCache::CacheMeshMap::const_iterator iter = mCache.find(entity);
   if (iter != mCache.end())
   {
      return (*iter).second.get();
   }
   
   
   boost::shared_ptr<MeshInformationCache::CacheMeshData> meshDataPtr = boost::shared_ptr<MeshInformationCache::CacheMeshData>(new MeshInformationCache::CacheMeshData);
   mCache[entity] = meshDataPtr;

   MeshInformationCache::CacheMeshData *meshData = meshDataPtr.get();

   SubMesh* mesh = entity->getSubMesh();
	Matrix4 vertexTransform;
	entity->getWorldTransforms(&vertexTransform);
   Quaternion normalRotation = vertexTransform.extractQuaternion();

   getVertexData(mesh, meshData, vertexTransform, normalRotation);
   getUVData(mesh, meshData);
   getIndexData(mesh, meshData);


   return meshData;
}

void MeshInformationCache::getVertexData(const Ogre::SubMesh *submesh, MeshInformationCache::CacheMeshData *data,
                                         const Ogre::Matrix4 &transform, const Ogre::Quaternion &rotation)
{
   Ogre::VertexData* vertex_data = submesh->useSharedVertices ? submesh->parent->sharedVertexData : submesh->vertexData;

	const VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
   Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
   
   const VertexElement* normalElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
   Ogre::HardwareVertexBufferSharedPtr nbuf = vertex_data->vertexBufferBinding->getBuffer(normalElem->getSource());
   
   bool normalsInBuffer = (vbuf.getPointer() == nbuf.getPointer());

	unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

	float* pReal;

	data->mVertices.reserve(vertex_data->vertexCount);
   if (normalsInBuffer)
   {
      data->mNormals.reserve(vertex_data->vertexCount);
   }

	for (size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
	{
		posElem->baseVertexPointerToElement(vertex, &pReal);
      data->mVertices.push_back(transform * Ogre::Vector3(pReal[0], pReal[1], pReal[2]));

      if (normalsInBuffer)
      {
         normalElem->baseVertexPointerToElement(vertex, &pReal);
         data->mNormals.push_back(rotation * Ogre::Vector3(pReal[0], pReal[1], pReal[2]));
      }
	}

	vbuf->unlock();

   if (!normalsInBuffer)
   {
      getNormalData(submesh, data, rotation);
   }
}

void MeshInformationCache::getNormalData(const Ogre::SubMesh *submesh, MeshInformationCache::CacheMeshData *data, 
                                         const Ogre::Quaternion &rotation)
{
   VertexData* vertex_data = submesh->useSharedVertices ? submesh->parent->sharedVertexData : submesh->vertexData;
	const VertexElement* normalElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
   Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(normalElem->getSource());
	unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

	float* pReal;

	data->mNormals.reserve(vertex_data->vertexCount);

	for (size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
	{
		normalElem->baseVertexPointerToElement(vertex, &pReal);
      data->mNormals.push_back(rotation * Ogre::Vector3(pReal[0], pReal[1], pReal[2]));
	}

	vbuf->unlock();
}

void MeshInformationCache::getUVData(const Ogre::SubMesh *submesh, MeshInformationCache::CacheMeshData *data)
{
   VertexData* vertex_data = submesh->useSharedVertices ? submesh->parent->sharedVertexData : submesh->vertexData;
   
   // Get last set of texture coordinates
   const VertexElement* texcoordElem;

   int uv_index = 0;
   texcoordElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES, uv_index);
   for ( ; texcoordElem != 0 ; texcoordElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES, ++uv_index));

   uv_index--;
   texcoordElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES, uv_index);

   if (texcoordElem == 0)
      return;

   // Get texture coordinates
   HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(texcoordElem->getSource());
   unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

   float* pReal;

   data->mTextureCoords.reserve(vertex_data->vertexCount);

   for (size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
   {
	   texcoordElem->baseVertexPointerToElement(vertex, &pReal);
      data->mTextureCoords2.push_back(Ogre::Vector2(pReal[0], pReal[1]));
   }

   vbuf->unlock();

   texcoordElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES, 0);

   // Get texture coordinates
   vbuf = vertex_data->vertexBufferBinding->getBuffer(texcoordElem->getSource());
   vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
   data->mTextureCoords2.reserve(vertex_data->vertexCount);

   for (size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
   {
	   texcoordElem->baseVertexPointerToElement(vertex, &pReal);
      data->mTextureCoords.push_back(Ogre::Vector2(pReal[0], pReal[1]));
   }

   vbuf->unlock();
}

void MeshInformationCache::getIndexData(const Ogre::SubMesh *submesh, MeshInformationCache::CacheMeshData *data)
{
   Ogre::IndexData* index_data = submesh->indexData;
   size_t indexCount = index_data->indexCount;
   Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

   bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

   unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
   unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

   data->mIndices.reserve(indexCount);
   if ( use32bitindexes )
   {
      for ( size_t k = 0; k < indexCount; ++k)
      {
          data->mIndices.push_back(pLong[k]);
      }
   }
   else
   {
      for ( size_t k = 0; k < indexCount; ++k)
      {
         data->mIndices.push_back(static_cast<unsigned long>(pShort[k]));
      }
   }

   ibuf->unlock();
}



