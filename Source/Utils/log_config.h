#pragma once

#include "log_base.h"
#include "log_wrapper.h"

namespace zenith
{
	namespace util
	{
		using zstdLogImpl = LogImpl_ofstream_;
		using zstdLog = LogWrapper_FlushOn<
							LogWrapper_PrefixTimeAndType<
								LogWrapper_Filter<
									Log<zstdLogImpl>,
									LogType::DEBUG //log everything
								>
							>,
							LogType::DEBUG //always flush
						>;
							

		class zLOG
		{
			static inline bool initLog_(zstdLog &log)
			{
				std::remove("default.log");
				log.setOutput("default.log");
				log.log(LogType::REGULAR, "Started main log\n");
				return true;
			}
			static inline zstdLog &getLog_()
			{
				static zstdLog log;
				static bool init = initLog_(log);
				return log;
			}
		public:
			static inline void resetOutput(const char * fname)
			{
				getLog_().log(LogType::REGULAR, std::string("Changing output to ") + fname);
				std::remove(fname);
				getLog_().setOutput(fname);
				getLog_().log(LogType::REGULAR, "Restarted main log\n");
			}
			static inline bool isOpen()
			{
				return getLog_().isOpen();
			}

			template<class A>
			static inline void log(LogType type, const std::basic_string<char, std::char_traits<char>, A> &str)
			{
				getLog_().log(type, str);
			}
			static inline void log(LogType type, const char * str)
			{
				getLog_().log(type, str);
			}
		};

		class StringException_
		{
		private:
			std::string msg_;
		protected:
			StringException_() {}
			StringException_(const std::string &msg) : msg_(msg) {}
			StringException_(const char * p) : msg_(p) {}
			const std::string &str() const { return msg_; }
			const char * c_str() const { return msg_.c_str(); }
		};

		class LoggedException : protected StringException_, public std::exception
		{
			std::string msg_;
		public:
			LoggedException() : std::exception() {}
			LoggedException(const char * p, LogType type = LogType::ERROR) : StringException_(p), std::exception(StringException_::c_str())
			{
				zLOG::log(type, StringException_::str());
			}
			LoggedException(const std::string &str, LogType type = LogType::ERROR) : StringException_(str), std::exception(StringException_::c_str())
			{
				zLOG::log(type, StringException_::str());
			}
			virtual ~LoggedException() {}
		};

#define ZLOG_DEBUG(x) zenith::util::zLOG::log(zenith::util::LogType::DEBUG, x)
#define ZLOG_REGULAR(x) zenith::util::zLOG::log(zenith::util::LogType::REGULAR, x)
#define ZLOG_WARNING(x) zenith::util::zLOG::log(zenith::util::LogType::WARNING, x)

	}
}
