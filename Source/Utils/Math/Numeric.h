#pragma once

#include "../log_config.h"
namespace zenith
{
	namespace util
	{
		namespace math
		{
			class MathException : public zenith::util::LoggedException
			{
				uint64_t errorCode_ = 0;
			public:
				MathException() : zenith::util::LoggedException() {}
				MathException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
				{
				}
				MathException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
				{
				}
				virtual ~MathException() {}
			};
			class NumericException : public MathException
			{
				uint64_t errorCode_ = 0;
			public:
				NumericException() : MathException() {}
				NumericException(uint64_t errorCode) : errorCode_(errorCode) {}
				template<class E>
				NumericException(E errorCode) : errorCode_(static_cast<uint64_t>(errorCode)) {}
				NumericException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : MathException(p, type)
				{
				}
				NumericException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : MathException(str, type)
				{
				}
				NumericException(uint64_t errorCode, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : MathException(p, type), errorCode_(errorCode)
				{
				}
				NumericException(uint64_t errorCode, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : MathException(str, type), errorCode_(errorCode)
				{
				}
				inline uint64_t errorCode() const { return errorCode_; }
				template<class E>
				inline E errorCode() const { return static_cast<E>(errorCode_); }
				virtual ~NumericException() {}
			};
		}
	}
}