#pragma once

#include <fstream>
#include <exception>
#include "enum2str.h"
#undef ERROR
namespace zenith
{
	namespace util
	{
		class LogException : public std::exception
		{
		public:
			LogException() : std::exception("LogException: unknown cause") {}
			LogException(const char * p) : std::exception(p) {}
			virtual ~LogException() {}
		};

		enum class LogType { UNDEF = 0, DEBUG = 5, REGULAR = 10, WARNING = 20, ERROR = 50, FATAL = 100 };

		ZENITH_ENUM2STR(LogType, UNDEF, DEBUG, REGULAR, WARNING, ERROR, FATAL)

		class LogImpl_ofstream_
		{
			std::ofstream f_;
		protected:
			inline ~LogImpl_ofstream_()
			{
				f_.flush();
				f_.close();
			}
			inline bool isOpen() const
			{
				return f_.is_open();
			}
			inline void setOutput(const char * fname)
			{
				if (f_.is_open())
				{
					f_.flush();
					f_.close();
				}
				f_.open(fname, std::ios::app);
			}
			inline void flush()
			{
				if (f_.is_open())
					f_.flush();
				else
					throw LogException("Log file is not open (flush function)!");
			}
			inline void write(const char * str, size_t sz)
			{
				if (f_.is_open())
					f_.write(str, sz);
				else
					throw LogException("Log file is not open (write function)!");
			}
		};

		template<class BaseImpl>
		class Log : protected BaseImpl
		{
		public:
			inline void setOutput(const char * fname)
			{
				BaseImpl::setOutput(fname);
			}
			inline bool isOpen() const { return BaseImpl::isOpen(); }
			inline void flush() { BaseImpl::flush(); }
			template<class A>
			inline void log(LogType type, const std::basic_string<char, std::char_traits<char>, A> &str)
			{
				BaseImpl::write(str.c_str(), str.length());
			}
			inline void log(LogType type, const char * str)
			{
				BaseImpl::write(str, std::strlen(str));
			}
		};
	}
}