// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __Avatar_h__
#define __Avatar_h__

#include "AvatarAttachment.h"
#include "Cloth.h"
#include <OGRE/Ogre.h>
#include <string>
#include <vector>

namespace Rex
{
//   class OgreAvatarAttachment;

   //! Describes an animation to be played by the avatar
   struct AnimationDef
   {           
       //! Most likely a UUID
       std::string Id;
       //! Actual animation name in the mesh/skeleton
       std::string InternalName;
       //! Should play looped?
       bool Looped;
       //! Exclusive; override (stop) other animations
       bool Exclusive;
       //! Speed scaled with avatar movement speed?
       bool UseVelocity;
       //! Always restart animation when it starts playing?
       bool AlwaysRestart;
       //! Blend-in period in seconds
       float FadeIn;
       //! Blend-out period in seconds
       float FadeOut;
       //! Speed modification (1.0 original)
       float SpeedFactor;
   };

   typedef std::map<std::string, AnimationDef> AnimationDefMap;

   //! Appearance modifier types
   enum AppearanceModifierType
   {
      MODIFIER_BONE = 0,
      MODIFIER_MORPH,
      MODIFIER_TEXTURE
   };

   //! Appearance modifier operation modes
   enum AppearanceModifierMode
   {
      MODE_CUMULATIVE = 0,
      MODE_AVERAGE,
   };

   //! Maps appearance modifier position to the master modifier position for interpolation
   struct PositionMapping
   {
      float MasterPosition;
      float Position;

      bool operator < (const PositionMapping &rhs) const
      {
         return (MasterPosition < rhs.MasterPosition);
      }
   };

   typedef std::vector<PositionMapping> PositionMappingVector;

   //! Describes an appearance modifier (currently either bone or morph) and how it is driven by a master modifier
   struct AppearanceModifier
   {
      //! Type
      AppearanceModifierType Type;
      //! Name
      std::string Name;
      //! Mode when driven by multiple master modifiers (cumulative, average)
      AppearanceModifierMode Mode;
      //! Positions at certain master modifier positions for interpolation
      std::vector<PositionMapping> Positions;
   };

   //! Accumulation data for appearance modifier, if driven by several master modifiers
   struct ModifierAccumulationData
   {
      //! Last stored position
      float Position;
      //! Sum of data accumulated so far
      float Sum;
      //! Number of samples accumulated
      int Samples;

      ModifierAccumulationData() : Position(0), Sum(0), Samples(0) {}
      void addAverage(float newPosition) { Sum += newPosition; Samples++; Position = Sum / Samples; }
      void addCumulative(float newPosition) { Sum += newPosition; Samples++; Position = Sum < 1.0f ? Sum : 1.0f; }
   };

   typedef std::map<std::string, ModifierAccumulationData> ModifierAccumulationMap;

   typedef std::vector<AppearanceModifier> AppearanceModifierVector;
   
   //! Describes a master modifier that controls several appearance modifiers
   struct MasterModifier
   {
      //! Current position (0.0 - 1.0)
      float Position;
      //! Category description (for UI)
      std::string Category;
      //! Modifiers controlled by this
      std::vector<AppearanceModifier> Modifiers;
   };

   typedef std::map<std::string, MasterModifier> MasterModifierMap;

   //! Types of bone constraints
   enum BoneConstraintType
   {
      CONSTRAINT_POSITION = 0,
      CONSTRAINT_LOOKAT
   };

   //! Describes a bone constraint
   struct BoneConstraint
   {
      std::string Target;
      BoneConstraintType Type;

      bool operator < (const BoneConstraint &rhs) const
      {
         if ((rhs.Type == CONSTRAINT_LOOKAT) && (Type == CONSTRAINT_POSITION))
            return true;
         else 
            return false;
      }
   };

   //! Vector that holds constraints of one bone
   typedef std::vector<BoneConstraint> BoneConstraintVector;
   //! Map for holding all bone constraints of a skeleton
   typedef std::map<std::string, BoneConstraintVector> BoneConstraintMap;

   //! Describes the dynamic animation effects (bone deforms).
   struct DynamicAnimationModifier
   {
      //! Actual internal name of the dynamic animation
      std::string InternalName;

      //! Position for the animation
      float Position;
   };

   typedef std::map<std::string, DynamicAnimationModifier> AnimationModifierMap;
   typedef std::vector<DynamicAnimationModifier*> AnimationModifierVector;

   //! Describes a morph target modifier
   struct MorphModifier
   {
       //! Internal name of the morph target (actually a pose animation in the mesh)
       std::string InternalName;

       //! Influence of the morph, between 0.0 - 1.0 (1.0 = apply fully)
       float Influence;

       //! Manual state: if set then should not be overridden by master modifiers (default false)
       bool Manual;

       MorphModifier() : Manual(false) {}
   };

   typedef std::map<std::string, MorphModifier> MorphModifierMap;

   //! Describes a texture modifier
   struct TextureModifier
   {
       //! Name of texture to use
       std::string TextureName;
       //! Submesh index to use texture in
       int SubmeshIndex;
       //! Threshold of modifier where change takes place (no blending yet)
       float Threshold;
       //! Influence of the texture (for possible future blending support)
       float Influence;
   };

   typedef std::map<std::string, TextureModifier> TextureModifierMap;

    //! Generic properties for avatar
   typedef std::map<std::string, std::string> PropertyMap;

   //! Generic realXtend avatar. Does not rely on any specific implementation for the avatar representation (such as Ogre).
   class Avatar
   {
      friend class AvatarSerializer;
   public:
       static const float AVATAR_BASE_VERSION;

      //! Specifies different avatar textures
      enum TEXTURE
      {
         TEXTURE_BODY = 0,
         TEXTURE_FACE,
         TEXTURE_COUNT // number of textures
//         BODY,
//         LOWER_BODY,
//         UPPER_BODY
      };

      //! default constructor, takes an ogre entity
      /*!
          \param ent Ogre entity
      */
      Avatar();
      
      //! Constructor that takes uid
      Avatar(const std::string &uid);

      //! destructor
      virtual ~Avatar();

      //! Resets the avatar to basic state (no materials, attachment...)
      virtual void clear(bool deleteAttachments = true);

      //! Clears all attachments
      virtual void clearAttachments();

      //! Sets avatar data version
      void setVersion(float Version);

      //! Returns avatar data version
      virtual float getVersion() const;

      //! Returns avatar height in meters
      /*!
          \return Height in meters
      */
      virtual float getHeight() const;

      //! Sets avatar height in meters
      /*!
          \param height Avatar height in meters
      */
      virtual void setHeight(float height);

      //! sets avatar weight
      /*!
          \param weight Value from range [0,1], where 0 is thin and 1 is heavy
      */
      virtual void setWeight(float weight);

      //! Returns avatar weight
      /*!
          \return Avatar's weight in range [0,1], where 0 is thin and 1 is heavy
      */
      virtual float getWeight() const;

      //! Returns mesh
      /*!
          \return mesh name
      */
      virtual std::string getMesh() const;

      //! Sets mesh
      /*!
          \param mesh mesh
      */
      virtual void setMesh(const std::string &mesh);

      //! set skeleton
      virtual void setSkeleton(const std::string &skeleton);

      //! returns skeleton
      virtual const std::string &getSkeleton() const;

      //! Add attachment to avatar
      virtual AvatarAttachment *addAttachment(AvatarAttachment *attachment) = 0;

      //! Adds new material with name 'name'
      /*! This is more generic way of adding materials to avatar and doesn't
          depend on specific texture placement. These materials will not be serialized,
          unless directly applied to the avatar entity.

          \return The index of the added material
      */
      virtual size_t addMaterial(const std::string &name);

      //! Sets material to specific index position
      /*!
          This is the more specific way of adding material to avatar. If these material assignments
          exist, they are used in serializing instead of using the actual materials the avatar might
          be using.
      */
      virtual void setMaterial(size_t index, const std::string &name);

      //! Returns material in specific index position. It is important to
      //! make sure index is smaller than number of materials.
      const std::string &getMaterial(size_t index) const;

      //! Returns number of materials
      size_t getNumMaterials() const;

      //! Returns number of attachments associated with this avatar. They may not actually be attached.
      virtual size_t getNumAttachments() const;

      //! Returns an attachment associated with this avatar. The attachment may not actually be attached.
//      virtual const OgreAvatarAttachment *getAttachment(size_t index) const;
      virtual const AvatarAttachment *getAttachment(size_t index) const;

      //! Sets texture for avatar
      /*!
          \param position placement of the texture, such as FACE or BODY
          \param name name of the texture
      */
      virtual void setTexture(TEXTURE position, const std::string &name);

      //! Returns texture at position
      virtual std::string getTexture(TEXTURE position);

      //! Sets new animation modifier effect (bonedeform)
      /*!
          \param name (human-readable) name of the dynamic animation
          \param animation Animation modifier to add
      */
      virtual void setAnimationModifier(const std::string &name, const DynamicAnimationModifier &animation);

      //! Returns animation modifier (bonedeform).
      /*! Returns empty animation modifier if one with specified name is not found.
      */
      virtual const DynamicAnimationModifier &getAnimationModifier(const std::string &name);

      //! Returns map (unordered) of dynamic animation modifiers for this avatar
      virtual const AnimationModifierMap &getAnimationModifierMap() const;

      //! Returns vector (ordered) of dynamic animation modifiers for this avatar
      /*! It is important to apply animation modifiers in order, as the end result depends on the order.
      */
      virtual const AnimationModifierVector &getAnimationModifierVector() const;

      //! Set position for animation modifier
      virtual void setAnimationModifierPosition(const std::string &name, float position = 0.5f);

      //! Sets new morph modifier effect
      /*!
          \param name (human-readable) name of the morph animation
          \param morph Morph modifier to add
      */
      virtual void setMorphModifier(const std::string &name, const MorphModifier& morph);

      //! Returns morph modifier.
      /*! Returns empty modifier if one with specified name is not found.
      */
      virtual const MorphModifier& getMorphModifier(const std::string &name);

      //! Returns map of morph modifiers 
      virtual const MorphModifierMap& getMorphModifierMap() const;

      //! Set morph modifier influence
      virtual void setMorphModifierInfluence(const std::string &name, float influence = 0.0f);

      //! Set morph modifier manual status
      virtual void setMorphModifierManual(const std::string &name, bool manual);

      //! Sets new texture modifier
      /*!
          \param name (human-readable) name of the texture modifier
          \param morph Texture modifier to add
      */
      virtual void setTextureModifier(const std::string &name, const TextureModifier& textureMod);

      //! Returns texture modifier.
      /*! Returns empty modifier if one with specified name is not found.
      */
      virtual const TextureModifier& getTextureModifier(const std::string &name);

      //! Returns map of texture modifiers 
      virtual const TextureModifierMap& getTextureModifierMap() const;

      //! Set texture modifier influence
      virtual void setTextureModifierInfluence(const std::string &name, float influence = 0.0f);

      //! Returns animation definition by name
      /*! Returns empty definition if one with specified name is not found.
      */
      virtual const AnimationDef& getAnimationDef(const std::string &name);

      //! Returns map of animation definitions
      virtual const AnimationDefMap& getAnimationDefMap() const;

      //! Returns modifiable map of animation definitions
      virtual AnimationDefMap& getAnimationDefMap();

      //! Set new animation definition
      /*! If one exists with same name, will replace it
       */
      virtual void setAnimationDef(const std::string &name, const AnimationDef& animationDef);

      //! Clear all animation definitions
      virtual void clearAnimationDefs();

      //! Sets new master appearance modifier
      /*!
          \param name (human-readable) name of the master appearance modifier
          \param modifier master appearance modifier to add
      */
      virtual void setMasterModifier(const std::string &name, const MasterModifier& modifier);

      //! Returns master appearance modifier.
      /*! Returns empty modifier if one with specified name is not found.
      */
      virtual const MasterModifier& getMasterModifier(const std::string &name);

      //! Returns map of master appearance modifiers 
      virtual const MasterModifierMap& getMasterModifierMap() const;

      //! Set master modifier position
      virtual void setMasterModifierPosition(const std::string &name, float position = 0.0f);

      //! Clear all master modifiers
      virtual void clearMasterModifiers();

      //! Re-calculate all master modifiers
      virtual void calculateMasterModifiers();

      //! Returns map of bone constraints
      virtual const BoneConstraintMap& getBoneConstraintMap() const;

      //! Clears all bone constraints
      virtual void clearBoneConstraints();

      //! Sets constraints of a certain bone
      virtual void setBoneConstraints(const std::string &bone, const BoneConstraintVector &constraints);

      //! Add a single constraint to a certain bone
      virtual void addBoneConstraint(const std::string &bone, const BoneConstraint &constraint);

      //! Returns number of skeletons that contains extra animation data for the avatar
      virtual size_t getNumExtraAnimatedSkeletons() const;

      //! Returns name of a skeleton that contains extra animation data for the avatar
      virtual const std::string &getExtraAnimatedSkeleton(size_t index) const;

      //! Add skeleton that contains extra animation data for the avatar
      virtual void addExtraAnimatedSkeleton(const std::string &name);

      //! Returns read-only map of properties
      virtual const PropertyMap& getPropertyMap() const;

      //! Sets a generic property
      virtual void setProperty(const std::string& key, const std::string& value);

      //! Whether avatar has non-empty property by certain key
      virtual bool hasProperty(const std::string& key) const;

      //! Gets a property by key. Returns empty string if not found
      virtual const std::string& getProperty(const std::string& key);

      //! Unsets (clears) a generic property
      virtual void unsetProperty(const std::string& key);

      //! Clears all generic properties
      virtual void clearProperties();

      static float getBaseHeight() { return BaseHeight; }

   protected:
      //! Re-calculate parameters driven by a master modifier. Requires maps for accumulation data (in case modifiers driven by many master modifiers)
      virtual void calculateMasterModifier(const MasterModifier& modifier, ModifierAccumulationMap& boneMap, ModifierAccumulationMap& morphMap, ModifierAccumulationMap& textureMap);

      //! Return interpolated position for a modifier from master modifier's position
      float interpolateAppearanceModifier(const AppearanceModifier& modifier, float masterPosition);

      static const float BaseHeight;

      //! Avatar height in meters
      float Height;

      //! Avatar weight in range [0,1], where 0 is thin, 1 is heavy
      float Weight;

      //! Avatar data version
      float Version;

      //! Name of (Ogre) mesh
      std::string MeshName;

      //! Name of (Ogre) skeleton
      std::string SkeletonName;

      //! list of attachment on this avatar
      std::vector<AvatarAttachment*> AttachmentList;

      //! List of materials. The materials should be in same order as they will be in the entity.
      std::vector<std::string> MaterialList;

      //! List of textures used by the avatar, in proper order.
      std::map<TEXTURE, std::string> TextureList;
      
      //! Unique id for this avatar
      std::string Uid;

      //! Modifiers for animation (bone deform)
      AnimationModifierMap AnimationModifiers;

      //! Modifiers for animation in ordered (vector) form.
      AnimationModifierVector AnimationModifiersVector;

      //! Morph modifiers
      MorphModifierMap MorphModifiers;

      //! Texture modifiers
      TextureModifierMap TextureModifiers;

      //! Animation definitions
      AnimationDefMap AnimationDefs;

      //! Master appearance modifiers
      MasterModifierMap MasterModifiers;

      //! Bone constraints
      BoneConstraintMap BoneConstraints;

      //! Skeletons that contain extra animation data for the avatar
      std::vector<std::string> ExtraSkeletons;

      //! Generic avatar properties
      PropertyMap Properties;
   };
}

#endif // __Avatar_h__
