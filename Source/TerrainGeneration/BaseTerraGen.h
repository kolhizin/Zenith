#pragma once
#include <cmath>
#include <Utils\log_config.h>

namespace zenith
{
	namespace terragen
	{
		const double TerraGenZeroTest = 1e-8;
		const double TerraGenInfty = 1e308;
		const double TerraGenInftyTest = 1e300;

		inline bool isZero(double d)
		{
			return fabs(d) <= TerraGenZeroTest;
		}
		inline bool isInfty(double d)
		{
			return fabs(d) >= TerraGenInftyTest;
		}
		inline bool isPosInfty(double d)
		{
			return d >= TerraGenInftyTest;
		}
		inline bool isNegInfty(double d)
		{
			return -d <= -TerraGenInftyTest;
		}

		class TerraGenException : public zenith::util::LoggedException
		{
		public:
			TerraGenException() : zenith::util::LoggedException() {}
			TerraGenException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
			{
			}
			TerraGenException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
			{
			}
			virtual ~TerraGenException() {}
		};
	}
}