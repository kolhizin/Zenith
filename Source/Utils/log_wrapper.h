#pragma once

#include <chrono>
#include "str_cast.h"


namespace zenith
{
	namespace util
	{
		template<class Base, LogType MinType = LogType::REGULAR>
		class LogWrapper_Filter : public Base
		{
		public:
			template<class A>
			inline void log(LogType type, const std::basic_string<char, std::char_traits<char>, A> &str)
			{
				if(static_cast<unsigned>(type) >= static_cast<unsigned>(MinType))
					Base::log(type, str);
			}
			inline void log(LogType type, const char * str)
			{
				if (static_cast<unsigned>(type) >= static_cast<unsigned>(MinType))
					Base::log(type, str);
			}
		};

		template<class Base>
		class LogWrapper_PrefixTimeAndType : public Base
		{
			std::chrono::time_point<std::chrono::steady_clock> start_;
			inline std::string getPrefix_(LogType type) const
			{
				char buff[64];
				std::chrono::duration<double> d = 1000.0 * (std::chrono::steady_clock::now() - start_); //milliseconds from start

				str_cast(d.count(), buff, 64);

				return str_cast<std::string>(type) + "(" + buff + " ms): ";
			}
		public:
			LogWrapper_PrefixTimeAndType()
			{
				start_ = std::chrono::steady_clock::now();
			}
			template<class A>
			inline void log(LogType type, const std::basic_string<char, std::char_traits<char>, A> &str)
			{
				Base::log(type, getPrefix_(type) + str.c_str() + "\n");
			}
			inline void log(LogType type, const char * str)
			{
				Base::log(type, getPrefix_(type) + str + "\n");
			}
		};

		template<class Base, LogType MinType = LogType::ERROR>
		class LogWrapper_FlushOn : public Base
		{
		public:
			template<class A>
			inline void log(LogType type, const std::basic_string<char, std::char_traits<char>, A> &str)
			{
				Base::log(type, str);
				if (static_cast<unsigned>(type) >= static_cast<unsigned>(MinType))
					Base::flush();
			}
			inline void log(LogType type, const char * str)
			{
				Base::log(type, str);
				if (static_cast<unsigned>(type) >= static_cast<unsigned>(MinType))
					Base::flush();
			}
		};
	}
}
