/** 
 * @file llbinarystream.h
 * @brief LLBinaryStream class header file
 *
 * Copyright (c) 2007-2008 LudoCraft
 */

#include "Ogre.h"

#ifndef LL_LLBINARYSTREAM_H
#define LL_LLBINARYSTREAM_H

//! Wrapper for LLSD::Binary to Ogre DataStream.
/*! To create a DataStream, create a DataStreamPtr and bind a LLBinaryStream
    object to it with DataStreamPtr::bind(&LLBinaryStream);
    Now you can feed the DataStreamPtr to any Ogre serializer.
*/
class LLBinaryStream : public Ogre::DataStream
{
public:
   //! constructs Ogre datastream from LLSD binary
   LLBinaryStream(const LLSD::Binary &data);

   virtual ~LLBinaryStream();

   // Copies bytes to buffer, returns the number of bytes read
   virtual size_t read(void *buf, size_t count);

   // Moves stream position count number of bytes ahead
   virtual void skip(long count);

   // Sets stream position
   virtual void seek(size_t pos);

   // Returns current stream position
   virtual size_t tell(void) const;

   // Returns true if position is at stream end, false otherwise
   virtual bool eof(void) const;

   // close stream
   virtual void close(void);

protected:
   //! actual data contained in the stream
   const LLSD::Binary &mData;

   //! current position
   size_t mPosition;
};

#endif // LL_LLBINARYSTREAM_H


