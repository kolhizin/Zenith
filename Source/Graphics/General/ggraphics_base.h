#pragma once

#include <Utils\log_config.h>
#include <Utils\bitenum.h>
#include "ggUtil_Enums.h"

namespace zenith
{
	namespace gengraphics
	{
		class GenGraphicsException : public zenith::util::LoggedException
		{
		public:
			GenGraphicsException() : zenith::util::LoggedException() {}
			GenGraphicsException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
			{
			}
			GenGraphicsException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
			{
			}
			virtual ~GenGraphicsException() {}
		};
	}
}