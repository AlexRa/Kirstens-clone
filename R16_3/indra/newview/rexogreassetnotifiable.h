/** 
 * @file rexogreassetnotifiable.h
 * @brief RexOgreAssetNotifiable class header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#ifndef REXOGREASSETNOTIFIABLE_H
#define REXOGREASSETNOTIFIABLE_H

class RexOgreAssetNotifiable
{
public:
   virtual void onOgreAssetLoaded(LLAssetType::EType assetType, const LLUUID& uuid) = 0;
};

#endif // REXOGREASSETNOTIFIABLE_H
