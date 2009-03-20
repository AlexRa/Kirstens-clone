/** 
 * @file rexprimdata.h
 * @brief RexPrimData class header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#ifndef REXPRIMDATA_H
#define REXPRIMDATA_H

#include <map>
#include "llprimitive.h"

// Dispatcher for RexPrimData generic message.
class RexPrimDataDispatchHandler : public LLDispatchHandler
{
public:
	// Handle dispatch
	virtual bool operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
		const sparam_t& string);
	
	virtual LLString getKey() const { return key; }
	static const LLString key;
};

//! A material definition, referring either to a plain texture or a material script
struct RexMaterialDef
{
    //! Use material script -flag. If true, mAssetID refers to a material script asset, otherwise a texture asset
    bool mUseMaterialScript;
    //! Materialscript or texture asset ID
    LLUUID mAssetID;

    //! Default constructor
    RexMaterialDef() :
        mUseMaterialScript(false),
        mAssetID(LLUUID::null)
    {
    }

    //! Parameter constructor
    RexMaterialDef(bool useMaterialScript, const LLUUID& assetID) : 
        mUseMaterialScript(useMaterialScript),
        mAssetID(assetID)
    {
    }

    bool operator == (const RexMaterialDef& rhs) const
    {
        return (mUseMaterialScript == rhs.mUseMaterialScript) && (mAssetID == rhs.mAssetID);
    }

    bool operator != (const RexMaterialDef& rhs) const
    {
        return (mUseMaterialScript != rhs.mUseMaterialScript) || (mAssetID != rhs.mAssetID);
    }
};

//! Defines Rex custom prim attributes, such as mesh/material related ones. Replaces deprecated PARAMS_REX-extradata field.
class RexPrimData
{
public:
    //! Object drawtype enumeration
    enum RexDrawType
    {
        DRAWTYPE_PRIM = 0,
        DRAWTYPE_MESH = 1,
        DRAWTYPE_BILLBOARD = 2
    };

    //! Constructor
    RexPrimData();

    //! Destructor
    ~RexPrimData();

    //! Returns if data changed and should be sent to server
    bool isChanged() const { return mChanged; }

    //! Resets changed flag
    void resetChanged() { mChanged = false; }

    //! Sends data block to server as a generic message
    void send(const LLUUID& primID);

    //! Unpacks parameters from a binary data block
    void unpack(LLDataPacker& dp);

    //! Packs parameters to a binary data block
    void pack(LLDataPacker& dp) const;

    //! Sets contents from the old PARAMS_REX datablock
    void setFromRexParams(const LLRexParams& params);

    //! Sets PARAMS_REX block
    /*! deprecated and not used, but provided for completeness
     */
    void setRexParams(LLRexParams& params);

    //! Sets drawtype
    void setDrawType(RexDrawType drawType);

    //! Sets visible flag
    void setIsVisible(bool enabled);

    //! Sets castshadows flag
    void setCastShadows(bool enabled);

    //! Sets lightcastsshadows-flag. Currently unused, as Ogre does not like shadowmaps with point lights
    void setLightCastsShadows(bool enabled);

    //! Sets show description texture-flag
    void setShowText(bool enabled);

    //! Sets mesh scaling flag
    void setScaleMesh(bool enabled);

    //! Sets solid alpha flag
    /*! Cannot be edited under the new system
     */
    void setSolidAlpha(bool enabled);
    
    //! Sets Python class name
    void setClassName(const std::string& className);

    //! Sets object selection priority (0 = default, can be positive or negative)
    void setSelectionPriority(int selectionPriority);

    //! Sets draw distance (0.0 = draw always)
    void setDrawDistance(F32 drawDistance);

    //! Sets mesh LOD bias (positive values, 1.0 = default)
    void setLOD(F32 lod);

    //! Sets fixed material type
    /*! Cannot be edited under the new system
     */
    void setFixedMaterial(FixedOgreMaterial fixedMaterial);

    //! Sets mesh asset ID to use
    void setMeshID(const LLUUID& id);

    //! Sets collision mesh asset ID to use
    void setCollisionMeshID(const LLUUID& id);

    //! Sets particle script asset ID to use. Null = disable particle effect
    void setParticleScriptID(const LLUUID& id);

    //! Sets animation pack (skeleton) asset ID
    void setAnimationPackID(const LLUUID& id);

    //! Sets animation name
    void setAnimationName(const std::string& name);

    //! Sets animation speed
    void setAnimationRate(F32 rate);

    //! Sets number of materials.
    /*! This affects how many material/texture ID's RexPrimData will hold & send across network
     */
    void setNumMaterials(U32 num);

    //! Sets a whole material definition
    bool setMaterialDef(U32 index, const RexMaterialDef& def);

    //! Replaces material/texture asset ID only for a single material index
    bool setMaterialID(U32 index, const LLUUID& id);

    //! Replaces usematerialscript-flag only for a single material index
    bool setMaterialUseScript(U32 index, bool enable);

    //! Sets sound asset ID
    void setSoundID(const LLUUID& id);
	
	//! Sets sound source ID
	void setSoundSourceID(LLUUID& id);
	
    //! Sets sound volume
    void setSoundVolume(F32 volume);

    //! Sets sound radius
    void setSoundRadius(F32 radius);
    
	//! Returns drawtype
    RexDrawType getDrawType() const { return mDrawType; }

    //! Returns isvisible flag
    bool getIsVisible() const { return mIsVisible; }

    //! Returns castshadows flag
    bool getCastShadows() const { return mCastShadows; }

    //! Returns lightcastsshadows flag
    bool getLightCastsShadows() const { return mLightCastsShadows; }

    //! Returns show description texture-flag
    bool getShowText() const { return mShowText; }

    //! Returns mesh scaling flag
    bool getScaleMesh() const { return mScaleMesh; }

    //! Returns solid alpha flag
    bool getSolidAlpha() const { return mSolidAlpha; }

    //! Returns Python class name
    const std::string& getClassName() const { return mClassName; }

    //! Returns selection priority
    int getSelectionPriority() const { return mSelectionPriority; }

    //! Returns draw distance
    F32 getDrawDistance() const { return mDrawDistance; }

    //! Returns mesh LOD bias
    F32 getLOD() const { return mLOD; }

    //! Returns fixed material type
    FixedOgreMaterial getFixedMaterial() const { return mFixedMaterial; }

    //! Returns mesh asset ID
    const LLUUID& getMeshID() const { return mMeshID; }

    //! Returns collisionmesh asset ID
    const LLUUID& getCollisionMeshID() const { return mCollisionMeshID; }

    //! Returns particle script asset ID
    const LLUUID& getParticleScriptID() const { return mParticleScriptID; }

    //! Returns animation pack (skeleton) asset ID
    const LLUUID& getAnimationPackID() const { return mAnimationPackID; }

    //! Returns animation name
    const std::string& getAnimationName() const { return mAnimationName; }

    //! Returns animation rate
    F32 getAnimationRate() const { return mAnimationRate; }

    //! Returns number of material definitions held in RexPrimData
    U32 getNumMaterials() const { return mMaterialDefs.size(); }
    
    const RexMaterialDef& getMaterialDef(U32 index) const;

    //! Returns material/texture asset ID only for material index
    const LLUUID& getMaterialID(U32 index) const;

    //! Returns usematerialscript-flag only for material index
    const bool getMaterialUseScript(U32 index) const;

    //! Returns sound asset ID
    const LLUUID& getSoundID() const { return mSoundID; }

	//! Returns sound source ID
	const LLUUID& getSoundSourceID() const { return mSoundSourceID; }

    //! Returns sound volume
    F32 getSoundVolume() const { return mSoundVolume; }

    //! Returns sound radius
    F32 getSoundRadius() const { return mSoundRadius; }

    //! Class static initialization. Registers RexPrimData genericmessage handler
    static void initClass();

    //! Queues RexPrimData for later use if RexPrimData received before object itself
    static void queueRexPrimData(const LLUUID& id, const RexPrimData& src);

    //! Gets queued RexPrimData and erases it from queued map
    static bool getQueuedRexPrimData(const LLUUID& id, RexPrimData& dest);

protected:
    //! Changed flag
    bool mChanged;
    //! Visible flag
    bool mIsVisible;
    //! Castshadows flag
    bool mCastShadows;
    //! Lightcastshadowows flag
    bool mLightCastsShadows;
    //! Show description texture-flag
    bool mShowText;
    //! Mesh scaling flag
    bool mScaleMesh;
    //! Solid alpha-flag
    bool mSolidAlpha;
    //! Python classname
    std::string mClassName;
    //! Selection priority
    int mSelectionPriority;
    //! Draw distance (0.0 = not limited)
    F32 mDrawDistance;
    //! Mesh LOD bias (1.0 = default)
    F32 mLOD;
    //! Fixed material type
    FixedOgreMaterial mFixedMaterial;
    //! Object drawtype (prim, mesh, billboard)
    RexDrawType mDrawType;
    //! Mesh asset ID
    LLUUID mMeshID;
    //! Collisionmesh asset ID, null to disable mesh collision (default)
    LLUUID mCollisionMeshID;
    //! Particle script asset ID, null to disable particle effect (default)
    LLUUID mParticleScriptID;
    //! Animation pack (skeleton) asset ID
    LLUUID mAnimationPackID;
    //! Animation name
    std::string mAnimationName;
    //! Animation rate
    F32 mAnimationRate;
    //! Sound asset ID
    LLUUID mSoundID;
	//! Sound source ID
	LLUUID mSoundSourceID;
	//! Sound volume 
	F32 mSoundVolume;
    //! Sound radius
    F32 mSoundRadius;
    //! Material definitions
    std::vector<RexMaterialDef> mMaterialDefs;

    //! RexPrimData dispatch handler
    static RexPrimDataDispatchHandler smDispatchRexPrimData;
    //! Map of queued RexPrimData for objects where RexPrimData was received before object itself
    static std::map<LLUUID, RexPrimData> sQueuedRexPrimData;
};

#endif // REXPRIMDATA_H
