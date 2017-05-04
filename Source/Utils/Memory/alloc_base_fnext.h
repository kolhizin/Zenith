#pragma once

namespace zenith
{
	namespace util
	{
		namespace memory
		{
			template<class Base>
			class MemAlloc_FnExt_ErrThrow : public Base
			{
			protected:
				inline void error_own()
				{
					log_error("Tried to perform actions on not own  memory allocation block!");
					throw MemAlloc_owner_exception("Tried to perform actions on not own  memory allocation block!");
				}
				inline void error_corrupt()
				{
					log_error("Corrupt memory allocation block!");
					throw MemAlloc_exception("Corrupt memory allocation block!");
				}
				inline void error_internal()
				{
					log_error("Internal allocator error!");
					throw MemAlloc_exception("Internal allocator error!");
				}
				inline MemAllocBlock error_cannot_alloc(const MemAllocBlock &blk)
				{					
					log_error("Cannot allocate block!");
					throw MemAlloc_alloc_exception("Cannot allocate block!");
				}
			};

			template<class Base, class Log>
			class MemAlloc_FnExt_LogFn : public Base
			{
			protected:
				inline void log_notify(const char * str)
				{
					Log::log(str);
				}
				inline void log_error(const char * str)
				{
					Log::log(str);
				}
			};
			template<class Base>
			class MemAlloc_FnExt_LogCOUT : public Base
			{
			protected:
				inline void log_notify(const char * str)
				{
					std::cout << str << "\n";
				}
				inline void log_error(const char * str)
				{
					std::cout << "\nERR: " << str << "\n\n";
				}
			};
		}
	}
}