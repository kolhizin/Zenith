#pragma once
#include "../log_config.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{

			/*
			1) Preserve named ranges
			2) Named and unnamed iterators should be comparable
			3) Named and unnamed iterators should be convertible
			*/

			class IOConvException : public zenith::util::LoggedException
			{
			public:
				IOConvException() : zenith::util::LoggedException() {}
				IOConvException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
				{
				}
				IOConvException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
				{
				}
				virtual ~IOConvException() {}
			};

			class IOConvBadStructureException : public zenith::util::LoggedException
			{
			public:
				IOConvBadStructureException() : zenith::util::LoggedException() {}
				IOConvBadStructureException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
				{
				}
				IOConvBadStructureException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
				{
				}
				virtual ~IOConvBadStructureException() {}
			};

			class IConvBadStructureException : public IOConvException
			{
			public:
				IConvBadStructureException() : IOConvException() {}
				IConvBadStructureException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : IOConvException(p, type)
				{
				}
				IConvBadStructureException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : IOConvException(str, type)
				{
				}
				virtual ~IConvBadStructureException() {}
			};

			class IConvInvalidIteratorException : public IOConvException
			{
			public:
				IConvInvalidIteratorException() : IOConvException() {}
				IConvInvalidIteratorException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : IOConvException(p, type)
				{
				}
				IConvInvalidIteratorException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : IOConvException(str, type)
				{
				}
				virtual ~IConvInvalidIteratorException() {}
			};

			class OConvBadStructureException : public IOConvException
			{
			public:
				OConvBadStructureException() : IOConvException() {}
				OConvBadStructureException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : IOConvException(p, type)
				{
				}
				OConvBadStructureException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : IOConvException(str, type)
				{
				}
				virtual ~OConvBadStructureException() {}
			};

			class OConvInvalidIteratorException : public IOConvException
			{
			public:
				OConvInvalidIteratorException() : IOConvException() {}
				OConvInvalidIteratorException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : IOConvException(p, type)
				{
				}
				OConvInvalidIteratorException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : IOConvException(str, type)
				{
				}
				virtual ~OConvInvalidIteratorException() {}
			};

			enum class InternalType { NONE = 0, STRING, BINARY };
			enum class NodeType {NONE = 0, VALUE, COMPLEX};
			enum class NodeValueHint : uint32_t {NONE = 0, ATTRIBUTE};
			enum class NodeComplexHint : uint32_t {NONE = 0};

			enum class IteratorType {NONE = 0, INPUT = 1, OUTPUT = 2, INPUT_OUTPUT = 3};
			enum class IteratorCategory { NONE = 0, UNNAMED = 1, NAMED = 2 };

			enum class PresenceType : uint8_t {NONE = 0, ZERO = 1, ONE = 2, ONE_OR_ZERO = 3, ZERO_PLUS = 5, ONE_PLUS = 6};
			inline bool is_required(PresenceType p) { return((static_cast<uint8_t>(p) & 2) == 2); }
			inline bool is_optional(PresenceType p) { return((static_cast<uint8_t>(p) & 1) == 1); }
			inline bool is_multiple(PresenceType p) { return((static_cast<uint8_t>(p) & 4) == 4); }

			template<class It>
			inline bool check_type(It &&it, NodeType type)
			{
				return (it.type() == type)
			}
			template<class It>
			inline It ensure_type(It &&it, NodeType type)
			{
				if (it.type() == type)
					return it;
				throw IOConvBadStructureException("ensure_type: type check failed!");
			}
		}
	}
}