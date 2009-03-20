/**
 * @file rexprimdata.cpp
 * @brief RexPrimData class implementation
 *
 * Copyright (c) 2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"
#include "rexprimdata.h"
#include "llvovolume.h"
#include "llviewergenericmessage.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"

#define MAX_REXPRIMDATA_SIZE 3000

// Externs
extern LLDispatcher gGenericDispatcher;

// Statics
RexPrimDataDispatchHandler RexPrimData::smDispatchRexPrimData;
const LLString RexPrimDataDispatchHandler::key("RexPrimData");
RexMaterialDef emptyMaterialDef(false, LLUUID::null);
std::map<LLUUID, RexPrimData> RexPrimData::sQueuedRexPrimData;

bool RexPrimDataDispatchHandler::operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
		const LLDispatchHandler::sparam_t& string)
{
	U32 size = string.size();
	LLUUID uuid(string[0]);
    U32 pos = 0;
    U8 dataBlock[MAX_REXPRIMDATA_SIZE];

	if(size < 2)
	{
		llwarns << "Received too short RexPrimData message"<<llendl;
		return false;
	}
    else
	{	
		for(U32 i = 1; i < size; ++i)
        {
            for (U32 j = 0; j < string[i].size(); ++j)
            {
                dataBlock[pos++] = string[i][j];
                if (pos >= MAX_REXPRIMDATA_SIZE) break;
            }
            if (pos >= MAX_REXPRIMDATA_SIZE) break;
        }
	}
    LLDataPackerBinaryBuffer dp(dataBlock, pos);

	LLViewerObject *objectp = gObjectList.findObject(uuid);
	
	if(objectp)
	{
        if (objectp->getPCode() != LL_PCODE_VOLUME)
            return false;
        LLVOVolume *volobjp = (LLVOVolume *)objectp;

        volobjp->getRexPrimData().unpack(dp);
        volobjp->setRexPrimDataReceived();
        volobjp->onRexPrimDataChanged(false);
		volobjp->createRexSoundSource(TRUE);

	}
	else
	{
        llwarns << "RexPrimData received for object " << uuid << " before object, queuing" << llendl;
        RexPrimData temp;
        temp.unpack(dp);
        RexPrimData::queueRexPrimData(uuid, temp);
	}

	return true;
}

RexPrimData::RexPrimData() :
    mChanged(false),
    mDrawType(DRAWTYPE_PRIM),
    mIsVisible(true),
    mCastShadows(false),
    mLightCastsShadows(false),
    mShowText(false),
    mScaleMesh(false),
    mSolidAlpha(false),
    mSelectionPriority(0),
    mDrawDistance(0.0f),
    mLOD(1.0f),
    mSoundVolume(0.0f),
    mSoundRadius(0.0f),
    mAnimationRate(0.0f),
    mMeshID(LLUUID::null),
    mCollisionMeshID(LLUUID::null),
    mParticleScriptID(LLUUID::null),
    mAnimationPackID(LLUUID::null),
    mSoundID(LLUUID::null),
    mFixedMaterial(FIXEDMATERIAL_DEFAULT)
{
}

RexPrimData::~RexPrimData()
{
}

void RexPrimData::send(const LLUUID& primID)
{
    U8 dataBlock[MAX_REXPRIMDATA_SIZE];
    LLDataPackerBinaryBuffer dp(dataBlock, MAX_REXPRIMDATA_SIZE);
    pack(dp);
    U32 bytesLeft = dp.getCurrentSize();

    std::vector<std::string> strings;
    std::vector<std::vector<U8> > datas;

    strings.push_back(primID.asString());
    U32 pos = 0;
    while (bytesLeft)
    {
        U32 vecSize = bytesLeft;
        if (vecSize > 200) vecSize = 200;

        std::vector<U8> vec;
        vec.resize(vecSize);
        memcpy(&vec[0], &dataBlock[pos], vecSize);
        datas.push_back(vec);
        pos += vecSize;
        bytesLeft -= vecSize;
    }

    send_generic_message_binary("RexPrimData", strings, datas);
    mChanged = false;
}

void RexPrimData::pack(LLDataPacker& dp) const
{
    dp.packU8((U8)mDrawType, "drawtype"); 
    dp.packU8((U8)mIsVisible, "isvisible");
    dp.packU8((U8)mCastShadows, "castshadows");
    dp.packU8((U8)mLightCastsShadows, "lightcastsshadows");
    dp.packU8((U8)mShowText, "showtext");
    dp.packU8((U8)mScaleMesh, "scalemesh");
    dp.packF32(mDrawDistance, "drawdistance");
    dp.packF32(mLOD, "lod");
    dp.packUUID(mMeshID, "meshid");
    dp.packUUID(mCollisionMeshID, "collisionmeshid");
    dp.packUUID(mParticleScriptID, "particlescriptid");
    dp.packUUID(mAnimationPackID, "animationpackid");
    dp.packString(mAnimationName.c_str(), "animationname");
    dp.packF32(mAnimationRate, "animationrate");
    dp.packU8(mMaterialDefs.size(), "materialcount");

    for (U32 i = 0; i < mMaterialDefs.size(); ++i)
    {
        LLAssetType::EType assetType = LLAssetType::AT_TEXTURE;
        if (mMaterialDefs[i].mUseMaterialScript)
            assetType = LLAssetType::AT_MATERIAL;
        dp.packU8((U8)assetType, "materialassettype");
        dp.packUUID(mMaterialDefs[i].mAssetID, "materialassetid");
        dp.packU8(i, "materialindex");
    }

    dp.packString(mClassName.c_str(), "classname");
    dp.packUUID(mSoundID, "soundid");
    dp.packF32(mSoundVolume, "soundvolume");
    dp.packF32(mSoundRadius, "soundradius");

    // New parameters
    dp.packS32(mSelectionPriority, "selectionpriority");
}

void RexPrimData::unpack(LLDataPacker& dp)
{
    U8 drawType;
    dp.unpackU8(drawType, "drawtype");
    setDrawType((RexDrawType)drawType);

    U8 isVisible;
    dp.unpackU8(isVisible, "isvisible");
    setIsVisible((bool)isVisible);

    U8 castShadows;
    dp.unpackU8(castShadows, "castshadows");
    setCastShadows((bool)castShadows);

    U8 lightCastsShadows;
    dp.unpackU8(lightCastsShadows, "lightcastsshadows");
    setLightCastsShadows((bool)lightCastsShadows);

    U8 showText;
    dp.unpackU8(showText, "showtext");
    setShowText((bool)showText);

    U8 scaleMesh;
    dp.unpackU8(scaleMesh, "scalemesh");
    setScaleMesh((bool)scaleMesh);

    F32 drawDistance;
    dp.unpackF32(drawDistance, "drawdistance");
    setDrawDistance(drawDistance);

    F32 lod;
    dp.unpackF32(lod, "lod");
    setLOD(lod);

    LLUUID meshID;
    dp.unpackUUID(meshID, "meshid");
    setMeshID(meshID);

    LLUUID collisionMeshID;
    dp.unpackUUID(collisionMeshID, "collisionmeshid");
    setCollisionMeshID(collisionMeshID);

    LLUUID particleScriptID;
    dp.unpackUUID(particleScriptID, "particlescriptid");
    setParticleScriptID(particleScriptID);

    LLUUID animationPackageID;
    dp.unpackUUID(animationPackageID, "animationpackid");
    setAnimationPackID(animationPackageID);

    std::string animationName;
	dp.unpackString(animationName, "animationname");
	setAnimationName(animationName);

    F32 animationRate;
    dp.unpackF32(animationRate, "animationrate");
    setAnimationRate(animationRate);

    U8 numMaterials;
    dp.unpackU8(numMaterials, "materialcount");
    setNumMaterials(numMaterials);
    for (U32 i = 0; i < numMaterials; ++i)
    {
        U8 assetType;
        dp.unpackU8(assetType, "materialassettype");

        LLUUID assetID;
        dp.unpackUUID(assetID, "materialassetid");

        U8 index;
        dp.unpackU8(index, "materialindex");

        RexMaterialDef newDef((assetType != LLAssetType::AT_TEXTURE), assetID);
        setMaterialDef(index, newDef);
    }

    std::string className;
	dp.unpackString(className, "classname");
	setClassName(className);

    LLUUID soundID;
    dp.unpackUUID(soundID, "soundid");
    setSoundID(soundID);

    F32 soundVolume;
    dp.unpackF32(soundVolume, "soundvolume");
    setSoundVolume(soundVolume);

    F32 soundRadius;
    dp.unpackF32(soundRadius, "soundradius");
    setSoundRadius(soundRadius);

    // New parameters
    if (dp.hasNext())
    {
        S32 selectionPriority;
        dp.unpackS32(selectionPriority, "selectionpriority");
        mSelectionPriority = selectionPriority;
    }

    mChanged = false;
}

void RexPrimData::setFromRexParams(const LLRexParams& params)
{
    RexDrawType drawType = DRAWTYPE_PRIM;
    if (params.getIsBillboard()) drawType = DRAWTYPE_BILLBOARD;
    if (params.getIsMesh()) drawType = DRAWTYPE_MESH;
    setDrawType(drawType);

    setClassName(params.getClassName());
    setIsVisible(params.getIsVisible());
    setCastShadows(params.getCastShadows());
    setLightCastsShadows(false);
    setShowText(params.getShowText());
    setScaleMesh(params.getScaleMesh());
    setSolidAlpha(params.getSolidAlpha());
    setDrawDistance(params.getDrawDistance());
    setLOD(params.getLOD());
    setFixedMaterial(params.getFixedMaterial());
    setMeshID(params.getMeshID());
    setCollisionMeshID(params.getCollisionMeshID());
    setParticleScriptID(params.getParticleScriptID());
    setNumMaterials(params.getNumMaterials());
    setSelectionPriority(0);

    bool useMaterialScripts = params.getUseMaterialScripts();
    for (U32 i = 0; i < mMaterialDefs.size(); ++i)
    {
        setMaterialDef(i, RexMaterialDef(useMaterialScripts, params.getMaterialID(i)));
    }

    mChanged = false;
}

void RexPrimData::setRexParams(LLRexParams& params)
{
    params.setClassName(getClassName());
    params.setIsMesh(mDrawType == DRAWTYPE_MESH);
    params.setIsBillboard(mDrawType == DRAWTYPE_BILLBOARD);
    params.setIsVisible(getIsVisible());
    params.setCastShadows(getCastShadows());
    params.setShowText(getShowText());
    params.setScaleMesh(getScaleMesh());
    params.setSolidAlpha(getSolidAlpha());
    params.setUseParticleScript(mParticleScriptID != LLUUID::null);
    params.setDrawDistance(getDrawDistance());
    params.setLOD(getLOD());
    params.setFixedMaterial(getFixedMaterial());
    params.setMeshID(getMeshID());
    params.setCollisionMeshID(getCollisionMeshID());
    params.setParticleScriptID(getParticleScriptID());
    params.setNumMaterials(getNumMaterials());

    // This will be an imperfect conversion, set materialscripts enabled if first material uses them
    params.setUseMaterialScripts(getMaterialUseScript(0));
    for (U32 i = 0; i < mMaterialDefs.size(); ++i)
    {
        params.setMaterialID(i, mMaterialDefs[i].mAssetID);
    }
}

void RexPrimData::setDrawType(RexDrawType drawType)
{
    if (drawType != mDrawType)
    {
        mDrawType = drawType;
        mChanged = true;
    }
}

void RexPrimData::setIsVisible(bool enabled)
{
    if (enabled != mIsVisible)
    {
        mIsVisible = enabled;
        mChanged = true;
    }
}

void RexPrimData::setCastShadows(bool enabled)
{
    if (enabled != mCastShadows)
    {
        mCastShadows = enabled;
        mChanged = true;
    }
}

void RexPrimData::setLightCastsShadows(bool enabled)
{
    if (enabled != mLightCastsShadows)
    {
        mLightCastsShadows = enabled;
        mChanged = true;
    }
}

void RexPrimData::setShowText(bool enabled)
{
    if (enabled != mShowText)
    {
        mShowText = enabled;
        mChanged = true;
    }
}

void RexPrimData::setScaleMesh(bool enabled)
{
    if (enabled != mScaleMesh)
    {
        mScaleMesh = enabled;
        mChanged = true;
    }
}

void RexPrimData::setSolidAlpha(bool enabled)
{
    if (enabled != mSolidAlpha)
    {
        mSolidAlpha = enabled;
        mChanged = true;
    }
}

void RexPrimData::setClassName(const std::string& className)
{
    if (className != mClassName)
    {
        mClassName = className;
        mChanged = true;
    }
}

void RexPrimData::setSelectionPriority(int selectionPriority)
{
    if (selectionPriority < -999999) selectionPriority = 999999;
    if (selectionPriority > 999999) selectionPriority = 999999;

    if (selectionPriority != mSelectionPriority)
    {
        mSelectionPriority = selectionPriority;
        mChanged = true;
    }
}


void RexPrimData::setDrawDistance(F32 drawDistance)
{
	if (drawDistance < 0.0f) drawDistance = 0.0f;

    if (drawDistance != mDrawDistance)
    {
        mDrawDistance = drawDistance;
        mChanged = true;
    }
}

void RexPrimData::setLOD(F32 lod)
{
    if (lod < 0.0f) lod = 0.0f;

    if (lod != mLOD)
    {
        mLOD = lod;
        mChanged = true;
    }
}

void RexPrimData::setFixedMaterial(FixedOgreMaterial fixedMaterial)
{
    if (fixedMaterial != mFixedMaterial)
    {
        mFixedMaterial = fixedMaterial;
        mChanged = true;
    }
}

void RexPrimData::setMeshID(const LLUUID& id)
{
    if (id != mMeshID)
    {
        mMeshID = id;
        mChanged = true;
    }
}

void RexPrimData::setCollisionMeshID(const LLUUID& id)
{
    if (id != mCollisionMeshID)
    {
        mCollisionMeshID = id;
        mChanged = true;
    }
}

void RexPrimData::setParticleScriptID(const LLUUID& id)
{
    if (id != mParticleScriptID)
    {
        mParticleScriptID = id;
        mChanged = true;
    }
}

void RexPrimData::setAnimationPackID(const LLUUID& id)
{
    if (id != mAnimationPackID)
    {
        mAnimationPackID = id;
        mChanged = true;
    }
}

void RexPrimData::setAnimationName(const std::string& name)
{
    if (name != mAnimationName)
    {
        mAnimationName = name;
        mChanged = true;
    }
}

void RexPrimData::setAnimationRate(F32 rate)
{
    if (rate != mAnimationRate)
    {
        mAnimationRate = rate;
        mChanged = true;
    }
}

void RexPrimData::setSoundID(const LLUUID& id)
{
    if (id != mSoundID)
    {
        mSoundID = id;
        mChanged = true;
    }
}

void RexPrimData::setSoundSourceID(LLUUID& id)
{
	mSoundSourceID = id;
}

void RexPrimData::setSoundVolume(F32 volume)
{
    if (volume != mSoundVolume)
    {
        mSoundVolume = volume;
        mChanged = true;
    }
}

void RexPrimData::setSoundRadius(F32 radius)
{
    if (radius != mSoundRadius)
    {
        mSoundRadius = radius;
        mChanged = true;
    }
}

void RexPrimData::setNumMaterials(U32 num)
{
    U32 oldNum = mMaterialDefs.size();
    if (num != oldNum)
    {
        mMaterialDefs.resize(num);
        for (F32 i = oldNum; i < num; ++i)
        {
            mMaterialDefs[i] = emptyMaterialDef;
        }
        mChanged = true;
    }
}

bool RexPrimData::setMaterialDef(U32 index, const RexMaterialDef& def)
{
    // Enable at least 1 material when trying to set
    if ((index == 0) && (mMaterialDefs.size() == 0))
        setNumMaterials(1);

    if (index >= mMaterialDefs.size())
        return false;

    if (mMaterialDefs[index] != def)
    {
        mMaterialDefs[index] = def;
        mChanged = true;
    }
    return true;
}

bool RexPrimData::setMaterialID(U32 index, const LLUUID& id)
{
    // Enable at least 1 material when trying to set
    if ((index == 0) && (mMaterialDefs.size() == 0))
        setNumMaterials(1);

    if (index >= mMaterialDefs.size())
        return false;

    if (mMaterialDefs[index].mAssetID != id)
    {
        mMaterialDefs[index].mAssetID = id;
        mChanged = true;
    }
    return true;
}

bool RexPrimData::setMaterialUseScript(U32 index, bool enable)
{
    // Enable at least 1 material when trying to set
    if ((index == 0) && (mMaterialDefs.size() == 0))
        setNumMaterials(1);

    if (index >= mMaterialDefs.size())
        return false;

    if (mMaterialDefs[index].mUseMaterialScript != enable)
    {
        mMaterialDefs[index].mUseMaterialScript = enable;
        mChanged = true;
    }
    return true;
}

const RexMaterialDef& RexPrimData::getMaterialDef(U32 index) const
{
    if (index >= mMaterialDefs.size())
        return emptyMaterialDef;
    else
        return mMaterialDefs[index];
}

const bool RexPrimData::getMaterialUseScript(U32 index) const
{
    if (index >= mMaterialDefs.size())
        return false;
    else
        return mMaterialDefs[index].mUseMaterialScript;
}

const LLUUID& RexPrimData::getMaterialID(U32 index) const
{
    if (index >= mMaterialDefs.size())
        return LLUUID::null;
    else
        return mMaterialDefs[index].mAssetID;
}

// static
void RexPrimData::initClass()
{
	gGenericDispatcher.addHandler(smDispatchRexPrimData.getKey(), &smDispatchRexPrimData);
}

// static
void RexPrimData::queueRexPrimData(const LLUUID& id, const RexPrimData& src)
{
    sQueuedRexPrimData[id] = src;
}

// static
bool RexPrimData::getQueuedRexPrimData(const LLUUID& id, RexPrimData& dest)
{
    std::map<LLUUID, RexPrimData>::iterator i = sQueuedRexPrimData.find(id);
    if (i == sQueuedRexPrimData.end())
        return false;
    
    dest = i->second;
    sQueuedRexPrimData.erase(i);
    return true;
}