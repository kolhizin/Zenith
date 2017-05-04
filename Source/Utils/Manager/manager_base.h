#pragma once

#include "../log_config.h"

namespace zenith
{
	namespace util
	{
		namespace manager
		{
			class ManagerException : public zenith::util::LoggedException
			{
			public:
				ManagerException() : zenith::util::LoggedException() {}
				ManagerException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
				{
				}
				ManagerException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
				{
				}
				virtual ~ManagerException() {}
			};

			enum class ObjectStatus {UNDEF = 0, PENDING = 1, READY = 2};
		}
	}
}