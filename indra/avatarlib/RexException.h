
#ifndef __RexException_H__
#define __RexException_H__

#include <exception>
#include <string>

namespace Rex
{
   class RexException : public std::exception
   {
   public:
      //! Error codes.
      enum ExceptionCodes
      {
//         ERR_INVALIDPARAMS,
//         ERR_DUPLICATE_ITEM,
         ERR_ITEM_NOT_FOUND,
         ERR_INTERNAL_ERROR
      };

      //! default constuctor
      RexException(int number, const std::string& description, const std::string& source );

      RexException(int number, const std::string& description, const std::string& source, const char* type, const char* file, long line );

      //! copy constructor
      RexException(const RexException& rhs);

      //! destructor
      ~RexException() throw() {}

      void operator = (const RexException& rhs);

      //! Returns a string with the full description of this error.
      virtual const std::string& getFullDescription(void) const;

      //! returns error code
      virtual int getNumber(void) const throw();

      //! returns source string
      virtual const std::string &getSource() const { return source; }

      //! returns source file name.
      virtual const std::string &getFile() const { return file; }

      //! returns line number.
      virtual long getLine() const { return line; }

		/*! Returns a string with only the 'description' field of this exception. Use 
			getFullDescriptionto get a full description of the error including line number,
			error number and what function threw the exception.
      */
      virtual const std::string &getDescription(void) const { return description; }

		/// Override std::exception::what
		const char* what() const throw() { return getFullDescription().c_str(); }

   protected:
      long line;
      int number;
      std::string typeName;
      std::string description;
      std::string source;
      std::string file;
      mutable std::string fullDesc;
   };


	/** Template struct which creates a distinct type for each exception code.
	@note
	This is useful because it allows us to create an overloaded method
	for returning different exception types by value without ambiguity. 
	From 'Modern C++ Design' (Alexandrescu 2001).
	*/
	template <int num>
	struct ExceptionCodeType
	{
		enum { number = num };
	};

	// Specialised exceptions allowing each to be caught specifically
	class ItemIdentityException : public RexException
	{
	public:
      ItemIdentityException(int number, const std::string& description, const std::string& source, const char* file, long line)
			: RexException(number, description, source, "ItemIdentityException", file, line) {}
	};
	class InternalErrorException : public RexException
	{
	public:
      InternalErrorException(int number, const std::string& description, const std::string& source, const char* file, long line)
			: RexException(number, description, source, "InternalErrorException", file, line) {}
	};

	//! Class implementing dispatch methods in order to construct by-value
	//! exceptions of a derived type based just on an exception code.
	class ExceptionFactory
	{
	private:
		/// Private constructor, no construction
		ExceptionFactory() {}
	public:
		static ItemIdentityException create(ExceptionCodeType<RexException::ERR_ITEM_NOT_FOUND> code, 
         const std::string& desc, 
         const std::string& src, const char* file, long line)
		{
			return ItemIdentityException(code.number, desc, src, file, line);
		}
		static InternalErrorException create(ExceptionCodeType<RexException::ERR_INTERNAL_ERROR> code, 
         const std::string& desc, 
         const std::string& src, const char* file, long line)
		{
		   return InternalErrorException(code.number, desc, src, file, line);
		}
	};
	
	
#ifndef REX_EXCEPT
#define REX_EXCEPT(num, desc, src) throw Rex::ExceptionFactory::create( \
	            Rex::ExceptionCodeType<num>(), desc, src, __FILE__, __LINE__ );
#endif

}
#endif // __RexException_H__
