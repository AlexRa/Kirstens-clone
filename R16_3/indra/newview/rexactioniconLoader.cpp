// Copyright (c) 2007-2008 adminotech 
// For conditions of distribution and use, see copyright notice in license.txt

#include "llviewerprecompiledheaders.h"
#include "rexactioniconLoader.h"
#include <llpumpio.h>
#include <llcurl.h>
#include <llsdserialize.h>
#include "llpanelactionicons.h"

#include "llimagebmp.h"
#include "llimagetga.h"
#include "llimagejpeg.h"
#include "llimagepng.h"
#include "llpanelactionicons.h"
#include "llavatar.h"

// A simple class for managing data returned from a curl http request.
class LLHTTPBuffer
{
public:
	LLHTTPBuffer() { }

	static size_t curl_write( void *ptr, size_t size, size_t nmemb, void *user_data)
	{
		LLHTTPBuffer* self = (LLHTTPBuffer*)user_data;
		
		size_t bytes = (size * nmemb);
		self->mBuffer.append((char*)ptr,bytes);
		return nmemb;
	}

	LLSD asLLSD()
	{
		LLSD content;

		if (mBuffer.empty()) return content;
		
		std::istringstream istr(mBuffer);
		LLSDSerialize::fromXML(content, istr);
		return content;
	}

	std::string asString()
	{
		return mBuffer;
	}

private:
	std::string mBuffer;
};


ActionIconResponder::ActionIconResponder() : LLHTTPClient::Responder()
{
}

ActionIconResponder::ActionIconResponder(LLPanelActionIcons *IconList) : LLHTTPClient::Responder(), mIconList(IconList)
{
   assert(mIconList);
}


ActionIconResponder::~ActionIconResponder()
{
}
   
void ActionIconResponder::error(U32 status, const std::string& reason)
{
	llwarns << "Failed to connect to avatar storage (ActionIcon):  Error " << Ogre::StringConverter::toString(status) << " " << reason << llendl;
//   exit(0);
//   llwarns << "Failed to connect to avatar storage: " << reason << llendl;
   //! \todo to error handling / reporting (popup to user about not being able to reach avatar storage server?)
}


void ActionIconResponder::result(const LLSD& content)
{
	llinfos << "Result succeeded: ActionIcons." << llendl;
//  llinfos << "Received reply from avatar storage server(ActionIcon)." << llendl;

    if (!gPanelIcons) return;

	 // DELETE OLD ICONS FROM SCREEN
	gPanelIcons->removeAllIcons();

	if(content.size()==0)
	{
		llinfos << "INFO: Empty ActionIcon packet. User have 0 actionicons " << llendl;
		return;
	}
	 
	if (!mIconList)
      return;
 
	/*
   //! \todo check for proper data
   if (content.has("generic xml") == false)
   {
      llwarns << "No generic xml field!" << llendl;
      return;
   }
*/





   LLSD::map_const_iterator iter = content.beginMap();

   iter = content.beginMap();
   std::string xmlData;
   for ( ; iter != content.endMap() ; ++iter)
   {
      if (iter->first.compare("generic xml") == 0)
      {
         LLSD element = iter->second;
		 llinfos << "No generic xml " << llendl;
      //   mAvatar->setImportDocument(element.asString());
      }
      else if (iter->first.compare("name") == 0)
      {

      } else
      {
         LLSD element = iter->second;

         // get asset type from extension for now
         std::string name = iter->first;
         
		LLUUID Id;
		S32 POSX=0;
		S32 POSY=0;
		U16 Wi = 0;
		U16 He = 0;
		S8  Co = 0;

		Id.set(element["ID"].asString());
		POSX = element["x"].asInteger();
		POSY = element["y"].asInteger();
	
		LLSD::Binary buffer = element["image"].asBinary();

		S32 bufferSize = buffer.size();

		if(buffer.size())
		{
			LLSD::Binary ImageData;
			LLSD::Binary InfoData;

			// RAWIMAGE DATA
			S32 ImageDataSize = bufferSize-(10*3);
			ImageData.resize(ImageDataSize);

			for (S32 i = 0; i < ImageDataSize; i++)
			{
			ImageData[i] = buffer[i];
			}

			// RAWIMAGE INFO  (width,height, component)
			char Width[10]; 
			char Height[10];
			char Component[10];

			for (U32 i=0; i < 10; i++)
			{
			 Width[i] = '\0';
			 Height[i] = '\0';
			 Component[i] = '\0';
			}


			InfoData.resize((10*3));
			for (S32 i=ImageDataSize; i < ImageDataSize+(10*3); i++)
			{
			InfoData[i-ImageDataSize] = buffer[i];
			}

				
	
			char * Data = (char*)&InfoData[0];
			S32 DataSize = (10*3);

			S32 a = 0;
			S32 Over = 0;
			for (S32 i = a; i < DataSize; i++)
			{

				if(((char)Data[i]) == 'X' || Over >= 10)
				{
				a += 1;	
				break;
				}
				
				if(((char)Data[i]) >= '0' && ((char)Data[i]) <= '9')
				{
					Width[Over] = (char)Data[i];
					Over +=1;
				}

				a += 1;					
			}

			Over = 0;
			for (S32 i = a; i <  DataSize; i++)
			{

				if(((char)Data[i]) == 'X' || Over >= 10)
				{
				a += 1;	
				break;
				}
				
				if(((char)Data[i]) >= '0' && ((char)Data[i]) <= '9')
				{			
					Height[Over] =  (char)Data[i];
					Over +=1;
				}

				a += 1;
					
			}
 
			Over = 0;
			for (S32 i = a; i < DataSize; i++)
			{
				if(((char)Data[i]) == 'X' || Over >= 10)
				{
				a += 1;	
				break;
				}
				
				if(((char)Data[i]) >= '0' && ((char)Data[i]) <= '9')
				{
					Component[Over] =  ((char)Data[i]);
					Over +=1;
				}
	

	
				a += 1;
			}

			// chars to number
 			Wi = (U16)atoi (Width);
			He = (U16)atoi (Height);
			Co = (S8)atoi (Component);

			
			// build image
			LLPointer<LLImageRaw> raw_image = new LLImageRaw(&ImageData[0], Wi, He, Co);

			// buil action icon
			gPanelIcons->NewIconButtonToPanel( raw_image, POSX, POSY, Wi, Id);
			gPanelIcons->DisableIcon();

		}
	  }
   }


}

//----------------------------------------------------------------------------------------

void ActionIconAssetResponder::error(U32 status, const std::string& reason)
{
//   llwarns << "Failed to connect to avatar storage: " << reason << llendl;
   //! \todo to error handling / reporting (popup to user about not being able to reach avatar storage server?)
}

void ActionIconAssetResponder::result(const LLSD& content)
{
//   if (!mAvatar || mType == LLAssetType::AT_NONE)
//      return;
  


   LLSD::Binary asset;
   if (content.has(mHash))
   {
//      llinfos << "Asset found" << llendl;
      asset = content[mHash].asBinary();
   } else
   {
//      llinfos << "Asset not found" << llendl;
      return;
   }



//   // save binary to cache 
//   if (asset.empty() == false)
//   {
////      llinfos << "creating binary asset" << mName << llendl;
//
//      LLAvatarCache::getSingleton().createAsset(mName, asset, mHash, mType);
////      llinfos << "getting asset from cache" << llendl;
//      CacheAvatarAsset &createdAsset = LLAvatarCache::getSingleton().getAsset(mHash, mType);
//
////      llinfos << "creating ogre asset" << llendl;
//      Ogre::String resourceName = LLAvatar::createOgreAsset(mName, createdAsset, mType);
//      createdAsset.setOgreResourceName(resourceName);
//      llinfos << "set asset ready" << resourceName << llendl;
//      mAvatar->assetReady(resourceName, mType);
//
//   }// else
//    //   llinfos << "received asset empty!" << llendl;
}

// ---------------------------------------------

bool ActionIconLoader::importAvatar(Rex::ClientAvatar *avatar)
{
   assert(avatar);


//   LLHTTPClient::ResponderPtr responder = new ActionIconResponder(avatar);
//   LLHTTPClient::get(avatar->getUri(), responder);



   return false;
}

LLSD ActionIconLoader::blockingGet(const std::string& url)
{
   llinfos << "blockingGet of " << url << llendl;

	// Returns an LLSD map: {status: integer, body: map}
	char curl_error_buffer[CURL_ERROR_SIZE];
	CURL* curlp = curl_easy_init();

	LLHTTPBuffer http_buffer;

	// Without this timeout, blockingGet() calls have been observed to take
	// up to 90 seconds to complete.  Users of blockingGet() already must 
	// check the HTTP return code for validity, so this will not introduce
	// new errors.  A 5 second timeout will succeed > 95% of the time (and 
	// probably > 99% of the time) based on my statistics. JC
	curl_easy_setopt(curlp, CURLOPT_NOSIGNAL, 1);	// don't use SIGALRM for timeouts
	curl_easy_setopt(curlp, CURLOPT_TIMEOUT, 7);	// seconds

	curl_easy_setopt(curlp, CURLOPT_WRITEFUNCTION, LLHTTPBuffer::curl_write);
	curl_easy_setopt(curlp, CURLOPT_WRITEDATA, &http_buffer);
	curl_easy_setopt(curlp, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curlp, CURLOPT_ERRORBUFFER, curl_error_buffer);
	curl_easy_setopt(curlp, CURLOPT_FAILONERROR, 1);

	LLSD response = LLSD::emptyMap();

	S32 curl_success = curl_easy_perform(curlp);

	S32 http_status = 499;
	curl_easy_getinfo(curlp,CURLINFO_RESPONSE_CODE, &http_status);

	response["status"] = http_status;

	if (curl_success != 0 
		&& http_status != 404)  // We expect 404s, don't spam for them.
	{
        llinfos << "CURL ERROR" << Ogre::String(curl_error_buffer) << llendl;
		
		response["body"] = http_buffer.asString();
	}
	else
	{
		response["body"] = http_buffer.asLLSD();
	}
	
	curl_easy_cleanup(curlp);

	return response;
}
