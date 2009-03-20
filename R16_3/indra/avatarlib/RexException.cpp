

#include "RexException.h"
#include <iostream>
#include <sstream>

namespace Rex
{
   RexException::RexException(int num, const std::string& desc, const std::string& src) :
     line(0),
     number(num),
     description(desc),
     source(src)
   {
   }

     RexException::RexException(int num, const std::string& desc, const std::string& src, 
   const char* typ, const char* fil, long lin) :
      line( lin ),
      number( num ),
      typeName(typ),
      description( desc ),
      source( src ),
      file( fil )
   {
   }

   RexException::RexException(const RexException& rhs)
        : line( rhs.line ), number( rhs.number ), description( rhs.description ), source( rhs.source ), file( rhs.file )
   {
   }

   void RexException::operator = ( const RexException& rhs )
   {
      description = rhs.description;
      number = rhs.number;
      source = rhs.source;
      file = rhs.file;
      line = rhs.line;
		typeName = rhs.typeName;
   }

   const std::string& RexException::getFullDescription(void) const
   {
      if (fullDesc.empty())
      {
         std::stringstream desc;

	      desc <<  "REX EXCEPTION(" << number << ":" << typeName << "): "
		      << description 
		      << " in " << source;

         if( line > 0 )
         {
            desc << " at " << file << " (line " << line << ")";
         }

         fullDesc = desc.str();
      }

	   return fullDesc;
   }

   int RexException::getNumber(void) const throw()
   {
      return number;
   }
}
