/** 
 * @file llmeshcache.h
 * @brief LLMeshCache class header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#ifndef LL_LLMESHCACHE_H
#define LL_LLMESHCACHE_H

#include "OGRE/OgrePrerequisites.h"
#include <boost/shared_ptr.hpp>

//! Class for caching (sub)mesh data. Usefull when large amounts of Ogre mesh data is needed to be accessed repeatedly.
/*! Don't forget to clear the cache after the data is no longer needed.

    \note Consumes large amount of memory so shouldn't be used under normal circumstances.
*/
class MeshInformationCache
{
public:
   //! Used for storing mesh data
   struct CacheMeshData
   {
      std::vector<Ogre::Vector3> mVertices;
      std::vector<Ogre::Vector3> mNormals;
      std::vector<unsigned long> mIndices;
      std::vector<Ogre::Vector2> mTextureCoords; // First set of texture coords
      std::vector<Ogre::Vector2> mTextureCoords2; // second set of texture coords
   };
   typedef std::map< const Ogre::SubEntity*, boost::shared_ptr<CacheMeshData> > CacheMeshMap;


   //! default constructor
   MeshInformationCache();

   //! destructor
   ~MeshInformationCache();

   //! Clears the cache
   void clear();

   //! Returns struct containing mesh data for the specified subentity.
   /*! If the mesh is not in the cache, the mesh data is retrieved from the subentity.
   */
   const CacheMeshData *getMeshInformation(Ogre::SubEntity *entity);

private:
   //! Retrieves vertex and normal data from the specified mesh
   /*! Retrieves both vertex and normal data if they reside in same buffer.
       Otherwise normal data is retrieved separately using getNormalData()

       \param mesh Ogre submesh
       \param data data structure where the mesh info is saved to
       \param transform transform to apply to vertices
       \param rotation rotation to apply to normals
   */
   void getVertexData(const Ogre::SubMesh *submesh, MeshInformationCache::CacheMeshData *data, const Ogre::Matrix4 &transform,
                      const Ogre::Quaternion &rotation);

   void getNormalData(const Ogre::SubMesh *submesh, MeshInformationCache::CacheMeshData *data, 
                      const Ogre::Quaternion &rotation);

   void getUVData(const Ogre::SubMesh *submesh, MeshInformationCache::CacheMeshData *data);

   //! Retrieves index data for submesh. Probably doesn't work, if submesh uses shared mesh data (in rex, meshes don't)
   void getIndexData(const Ogre::SubMesh *submesh, MeshInformationCache::CacheMeshData *data);

   //! The big bad cache
   CacheMeshMap mCache;
};

#endif // LL_LLMESHCACHE_H
