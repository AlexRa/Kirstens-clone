/** 
 * @file rexpaneldata.h
 * @brief RexPanelData class header file
 *
 * Copyright (c) 2008 LudoCraft
 */

#ifndef LL_LLPANELREXDATA_H
#define LL_LLPANELREXDATA_H

#include <map>
#include "llpanel.h"
#include "lldispatcher.h"

class LLViewerObject;

// Dispatcher for RexData generic message.
class RexDataDispatchHandler : public LLDispatchHandler
{
public:
	// Handle dispatch
	virtual bool operator()(
		const LLDispatcher* dispatcher,
		const std::string& key,
		const LLUUID& invoice,
		const sparam_t& string);
	
	virtual LLString getKey() const { return key; }
	
	std::map<LLString, LLString> parseRexData(LLString data);

	static const LLString key;
};

//! An editor for freeform text data ("RexData") stored with prims.
class RexPanelData : public LLPanel
{
public:
    //! Constructor, called from LLFloaterTools's creation factory method
	RexPanelData(const std::string& name);
    //! Destructor
	virtual ~RexPanelData();
    //! Hooks up UI callbacks for controls.
	virtual BOOL postBuild();
    //! Deactivates controls
	virtual	void clearCtrls();
    //! Activates controls & refreshes data when a prim is selected.
	virtual void refresh();
    //! Static initialization. Hooks the genericmessage handler
	static void initClass();
    
    //! Handles sending data when "Set" button clicked
	static void onClickSetRexData(void* userdata);

    //! If RexData received when object does not yet exist, stores the data.
    static void queueRexData(const LLUUID& id, const std::string& src);
    //! Retrieves & clears queued data for an object.
    static bool getQueuedRexData(const LLUUID& id, std::string& dest);

	static BOOL processRexParameterMap(std::map<LLString, LLString> parameter_map, const LLVector3d& obj_pos_global);

private:
	static RexDataDispatchHandler smDispatchRexData;

    //! Queued RexData for objects whose data has been received before the objects themselves.
    static std::map<LLUUID, std::string> sQueuedRexData;

    //! Currently selected object in the panel, null for none
	LLViewerObject* mObject;
};

#endif // LL_LLPANELREXDATA_H
