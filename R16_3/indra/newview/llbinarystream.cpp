#include "llviewerprecompiledheaders.h"
#include "llbinarystream.h"



LLBinaryStream::LLBinaryStream(const LLSD::Binary &data) : mData(data), mPosition(0)
{
   mSize = data.size();
}

LLBinaryStream::~LLBinaryStream()
{
   close();
}

size_t LLBinaryStream::read(void *buf, size_t count)
{
   size_t end = llmin<size_t>(mPosition+count, mSize);
   size_t n;
   size_t k = 0;
   for (n=mPosition ; n<end ; ++n, ++k)
   {
      reinterpret_cast<U8*>(buf)[k] = mData[n];
   }
   mPosition = end;
   return k;
}

void LLBinaryStream::skip(long count)
{
   mPosition = llmax<size_t>(llmin<size_t>((size_t)(mPosition+count), mSize), 0);
}

void LLBinaryStream::seek(size_t pos)
{
   mPosition = llmax<size_t>(llmin<size_t>(pos, mSize), 0);
}

size_t LLBinaryStream::tell(void) const
{
   return mPosition;
}

bool LLBinaryStream::eof(void) const
{
   if (mPosition == mSize)
      return true;

   return false;
}

void LLBinaryStream::close(void)
{
}

