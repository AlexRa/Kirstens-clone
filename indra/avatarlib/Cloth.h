// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __Cloth_h__
#define __Cloth_h__

#include "CollisionEllipsoid.h"
#include "CollisionCapsule.h"

#include <OGRE/Ogre.h>
#include <vector>

namespace Rex
{
   class ClothThread;
   class ClothPtr;

   //! Constaint for the physics simulation
   struct Constraint
   {
      //! Start and end points for phycics particle
      Ogre::uint32 particleA, particleB;

      //! Distance between the particles on rest
      float restlength;

      bool operator == (const Constraint &other) const
      {
         if ((particleA == other.particleA && particleB == other.particleB) || (particleA == other.particleB && particleB == other.particleA))
            return true;

         return false;
      }

      bool operator != (const Constraint &other) const
      {
         return (!(*this == other));
      }
   };

   //! Physics properties for a vertex
   struct VertexData
   {
      //! default constructor
      VertexData();

      //! Vertex is affected by physics
      bool PhysicsEnabled;

      //! Vertex is fixed
      bool Fixed;

      //! Is this vertex affected by forces
      bool AffectedByForces;

      //! How well the cloth stays in shape
      Ogre::Real Rigidness;

      //! Multiplier for rigidness when cloth is affected by forces
      Ogre::Real RigidnessMultiplier;

      //! Gravity
      Ogre::Vector3 Gravity;

      //! Do normal plane collision
      bool NormalPlaneCollision;

      //! Distance for normal plane collision
      Ogre::Real NormalPlaneCollisionDistance;

      //! Fake wind flutter affects the cloth
      bool WindFlutter;

      //! Multiplier for force effect
      Ogre::Real ForceMultiplier;

      //! Is the vertex welded to another vertex (true for only one of the two vertices)
      bool Welded;

      //! Index of the vertex this vertex is welded to
      Ogre::uint32 WeldedTo;

      //! Calculates size of this struct for cloth export/import
      static size_t calculateSize()
      {
         size_t size = sizeof(bool);
         size += sizeof(bool);
         size += sizeof(bool);

         size += sizeof(Ogre::Real);
         size += sizeof(Ogre::Real);

         size += sizeof(Ogre::Real);
         size += sizeof(Ogre::Real);
         size += sizeof(Ogre::Real);

         size += sizeof(bool);
         size += sizeof(Ogre::Real);

         size += sizeof(bool); // windflutter
         size += sizeof(Ogre::Real);

         return size;
      }



      //! Default values for per-vertex properties
      static const bool          DEFAULT_PHYSICSENABLED;
      static const bool          DEFAULT_FIXED;
      static const bool          DEFAULT_AFFECTEDBYFORCES;
      static const Ogre::Real    DEFAULT_RIGIDNESS;
      static const Ogre::Real    DEFAULT_RIGIDNESSMULTIPLIER;
      static const Ogre::Vector3 DEFAULT_GRAVITY;
      static const bool          DEFAULT_NORMALPLANECOLLISION;
      static const Ogre::Real    DEFAULT_NORMALPLANECOLLISIONDISTANCE;
      static const bool          DEFAULT_WINDFLUTTER;
      static const Ogre::Real    DEFAULT_FORCEMULTIPLIER;

    //! Plane for collision handling
    //  Ogre::Plane CollisionPlane;
   };

   //! Contains various collision data for cloth
   struct CollisionData
   {
      //! Polygons.
      // each vertex of the polygon in separate vector
      //std::vector<Ogre::Vector3> Polygons[3]; // relatively speaking bad, since polygon vertices are not sequentially in memory
      std::vector<Ogre::Vector3> VertexA;
      std::vector<Ogre::Vector3> VertexB;
      std::vector<Ogre::Vector3> VertexC;
   };



   //! Avatar cloth item with physics
   /*!
       All functions marked threadsafe can be called from other thread
   */
   class Cloth : public Ogre::Resource
   {
   public:
      //! default constructor
      Cloth(Ogre::ResourceManager *creator, const Ogre::String &name, Ogre::ResourceHandle handle,
         const Ogre::String &group, bool isManual = false, Ogre::ManualResourceLoader *loader = 0);

//      //! constructor that takes a mesh from which the cloth is made
//      Cloth(const Ogre::MeshPtr &mesh, Ogre::SceneNode *node);

      //! destructor
      virtual ~Cloth();

      //! Cleans up data, removes cloth physics from mesh
      void clean();

      //! Clone this cloth
      /*!
          \note cloned cloths need to started separately
      */
      ClothPtr clone(const Ogre::String &newName, const Ogre::String &newGroup = Ogre::StringUtil::BLANK);

      //! Initializes class
      /*! Kicks of threads if threading is enabled
      */
      static void initClass();

      //! Deinitializes class
      /*! Cleans up threads if threading is enabled
      */
      static void deinitClass();

      //! Starts the simulation with this cloth, only important to call in threaded mode
      void startSimulation();

      //! Prepare the cloth for simulation. Only important to call in non-threaded mode (for efficiency)
      void prepare(bool includeSubmeshes = false);

      //! Sets mesh for the cloth, if one was not specified on creation time
      void setMesh(const Ogre::MeshPtr &mesh);

      //! sets mesh for the cloth by name
      void setMeshName(const Ogre::String &mesh);

      //! Returns mesh used by the cloth
      Ogre::MeshPtr &getMesh();

      //! Returns currently used mesh name
      const Ogre::String &getMeshName() const;

      //! Sets the parent scenenode for the cloth
      /*!
          \remark It is important to set this to the parent of the scenenode used by
                  the cloth entity, so that forces are transformed properly.
      */
      void setParentNode(const Ogre::SceneNode *node);

      //! Create collision shape and attach it to scenenode
      void createCollisionShape(Ogre::SceneManager *sceneManager, Ogre::SceneNode *node);

//      //! Create collision shape and attach it to bone in entity
//      void createCollisionShape(Ogre::SceneManager *sceneManager, Ogre::Entity *entity, const Ogre::String &bone);

      //! Sets collision mesh. For non-animated meshes
      /*!
          \note It is currently assumed the collision mesh is in the same object space as the cloth mesh
      */
      void setCollisionMesh(const Ogre::MeshPtr &mesh);

      //! Adds collision entity
      /*! This is the proper way to add collision for skeletally animated meshes
      */
      void setCollisionEntity(Ogre::Entity *entity);

      //! Prepare physics simulation. Call once before calling addTime(), unless the cloth is serialized.
      /*! 

          \note If the cloth is serialized, there's no need to call this as it's all pre-generated. See function isReady().
      */
      void preparePhysics();

      //! Returns true if cloth is ready for physics simulation and no preprocessing is needed.
      /*!
          \return true, if cloth is ready, false otherwise
      */
      bool isReady();

      //! Generates collision data from mesh
      void generateCollisionDataOld(const Ogre::MeshPtr &mesh);

      //! pregenerates collision data for cloth collision handling
      /*! \note Shoould be called once per frame for animated collision meshes. Clears old collision data.
      */
      void generateCollisionData();

      //! Reforms cloth back to it's original shape
      void reform();

      //! Set the (total) frametime-dependent force that affects the cloth. (Internally, the force gets multiplied with frametime)
      void setForce(const Ogre::Vector3 &force);

      //! Set the (total) frametime-independent force that affects the cloth
      void setAbsoluteForce(const Ogre::Vector3 &force);

      //! Advances cloth physics simulation by one frame
      void addTime(Ogre::Real time, bool applyforce);

      //! Applies constraints to cloth vertices
      /*!
          \note Threadsafe
      */
      void applyConstraints();

      //! Applies force and other modifiers to cloth vertices
      /*!
          \note Threadsafe
      */
      void applyForceAndModifiers(Ogre::Real time, bool applyforce);

      //! Recalculates normals for the cloth mesh
      /*!
          \note Threadsafe
      */
      void recalculateNormals();

      //! Returns vertex with index
      /*! \note The returned vertex is in object-space

          \param index Index of the vertex to return
          \return Vertex
      */
      Ogre::Vector3 getVertex(size_t index) const;

      //! Returns vertex with index
      /*! \note The returned vertex is in world-space (still relative to it's parent though)

          \param index Index of the vertex to return
          \return Vertex
      */
//      Ogre::Vector3 getVertexWorld(size_t index) const;

      //! Returns the number of vertices in the cloth
      /*!
          \return Number of vertices
      */
      size_t getVertexCount() const;

      //! Returns constraint
      const Constraint &getConstraint(size_t index) const;

      //! Returns number of constraints
      size_t getConstraintCount() const;

      //! Enable or disable physics to specified vertex
      /*!
          \param index Vertex index
          \param enable True to enable physics, false to disable
      */
      void enablePhysics(size_t index, bool enable = true);

      //! Fixes vertex to position or frees it
      /*!
          \param index Vertex index
          \param enable True to fix vertex, false to free vertex
      */
      void setFixed(size_t index, bool fixed = true);

      //! Sets wether vertex is affected by forces or not
      void setAffectedByForces(size_t index, bool affected);

      //! Sets rigidness for vertex
      void setRigidness(size_t index, Ogre::Real rigidness);

      //! Set rigidness multiplier of vertex
      void setRigidnessMultiplier(size_t index, Ogre::Real multiplier);

      //! Set gravity for vertex
      void setGravity(size_t index, const Ogre::Vector3 &gravity);

      //! Sets force multiplier for vertex
      void setForceMultiplier(size_t index, Ogre::Real forceMultiplier);

      //! Enable/disable normal plane collision for vertex
      void setNormalPlaneCollision(size_t index, bool collision);

      //! Set normal plane collision distance for vertex
      void setNormalPlaneCollisionDistance(size_t index, Ogre::Real distance);

      //! Enable/disable fake wind flutter for vertex
      void setWindFlutter(size_t index, bool wind);

      //! Returns vertex data for the specified vertex
      const VertexData &getVertexData(size_t index) const;

      //! set Weld vertices
      void setWeldVertices(bool weld);

      //! get Weld vertices
      bool getWeldVertices() const;

      //! Set physics iterations
      void setIterations(unsigned int iterations);

      //! Returns number of iterations over constraints
      unsigned int getIterations() const;

      //! set wind amplitude
      Ogre::Real getWindAmplitude() const;

      //! Returns wind amplitude
      void setWindAmplitude(Ogre::Real amplitude);

      //! Returns wind wavelength
      Ogre::Real getWindWaveLength() const;

      //! Sets wind wavelength
      void setWindWaveLength(Ogre::Real wavelength);

      //! Returns wind force multiplier
      Ogre::Real getWindForceMultiplier() const;

      //! Sets wind force multiplier
      void setWindForceMultiplier(Ogre::Real windforce);


      //! Disables all physics processing, nice for perf testing
      static void disableAllPhysics();

      //! Enables all physics processing
      static void setPhysicsEnabled(bool enabled);

      //! Advance cloth simulation by one frame. Handles all cloth
      static void simulate(Ogre::Real time);

      static const bool DEFAULT_WELDVERTICES;
      static const int  DEFAULT_SIMITERATIONS;
      static const Ogre::Real DEFAULT_WINDAMPLITUDE;
      static const Ogre::Real DEFAULT_WINDWAVELENGTH;
      static const Ogre::Real DEFAULT_WINDFORCEMULTIPLIER;

   protected:

      void loadImpl();
      void unloadImpl();
      size_t calculateSize() const;

      //! Creates constraints between two vertices
      /*!
          \param v1 Index of the first vertex
          \param v2 index of the second vertex
      */
      void createConstraint(size_t v1, size_t v2);

      //! Finds and deals with seams in the mesh.
      void containSeams();

      //! Generates constraints and collision planes for physics simulation, gathers vertex data
      void preprocessPhysics(const Ogre::MeshPtr &mesh, bool includeSubmeshes);

      void getMeshData(const Ogre::MeshPtr &mesh, bool includeSubmeshes);

      //! Generates constraints from vertices that are physics enabled
      void generateConstraints();

      static bool smRenderPhysics;

      //! Mesh used for cloth
      Ogre::MeshPtr ClothMesh;

      //! Parent node for the cloth
      const Ogre::SceneNode *ParentNode;

      //! Collision mesh for the cloth
      Ogre::MeshPtr CollisionMesh;

      //! Collision entity
      Ogre::Entity *CollisionEntity;

      //! Ogre math class. Needs to be created for table sin to work
      static Ogre::Math *OgreMath;

      //! Name of the mesh
      Ogre::String MeshName;

      //! Collision data
      CollisionData Collision;

      //! The combined force that affects the cloth currently
      Ogre::Vector3 TotalForce;

      //! The combined force that affects the cloth currently
      Ogre::Vector3 TotalAbsoluteForce;

      //! Array of cloth mesh normals
      Ogre::Vector3 *ClothNormals;

      //! Array of cloth mesh vertices
//      std::vector<Ogre::Point3> Vertices;
      float *ClothVertices;

//      float *ClothVerticesDefaultPosition;

      //! Array of fixed cloth mesh vertices (the original positions of vertices)
      float *ClothVerticesFixed;

      //! Position of cloth vertices in the previous frame
      float *ClothVerticesPrevFrame;

      //! Array of cloth mesh indices
      unsigned long *ClothIndices;

      //! Number of cloth mesh indices
      size_t IndexCount;

      //! Number of cloth mesh vertices
      size_t VertexCount;

      //! HW buffer for vertices in the cloth
      Ogre::HardwareVertexBufferSharedPtr VertexBuffer;

      //! List of physical constraints for the simulation
      std::vector<Constraint> ConstraintsList;

      //! Number of physics iterations per frame
      /*! Higher values mean more accuracy in constraints, but overall slower simulation.
          Low values generally yield better results in cloth physics.
      */
      unsigned int SimIterations;

      //! Weld vertices
      bool WeldVertices;

      //! Wind angle in radians
      Ogre::Radian WindAngle;

      //! Wind amplitude
      Ogre::Real WindAmplitude;

      //! Wind wavelength
      Ogre::Real WindWaveLength;

      //! Multiplier for wind when cloth is affected by wind
      Ogre::Real WindForceMultiplier;

      //! Vertex data for physics simulation
      std::vector<VertexData> VerticesData;

      //! Size of vertex declaration
      size_t ClothVertexSize;

      //! Collision shape for head
//      CollisionEllipsoid HeadCollision;
      CollisionCapsule HeadCollision;

      //! Thread for cloth physics
      static ClothThread *Thread;

      friend class ClothSerializer;
   };

   class ClothPtr : public Ogre::SharedPtr<Cloth> 
   {
   public:
      ClothPtr() : Ogre::SharedPtr<Cloth>() {}
      explicit ClothPtr(Cloth *rep) : Ogre::SharedPtr<Cloth>(rep) {}
      ClothPtr(const ClothPtr &r) : Ogre::SharedPtr<Cloth>(r) {} 
      ClothPtr(const Ogre::ResourcePtr &r) : Ogre::SharedPtr<Cloth>()
      {
        // lock & copy other mutex pointer
        OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
            OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<Cloth*>(r.getPointer());
        pUseCount = r.useCountPointer();
        if (pUseCount)
        {
            ++(*pUseCount);
        }
      }

      /// Operator used to convert a ResourcePtr to a TextFilePtr
      ClothPtr& operator=(const Ogre::ResourcePtr& r)
      {
        if (pRep == static_cast<Cloth*>(r.getPointer()))
            return *this;
        release();
        // lock & copy other mutex pointer
        OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
            OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<Cloth*>(r.getPointer());
        pUseCount = r.useCountPointer();
        if (pUseCount)
        {
            ++(*pUseCount);
        }
        return *this;
      }
   };
}

#endif // __Cloth_h__


