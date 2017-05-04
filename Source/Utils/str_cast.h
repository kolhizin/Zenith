#pragma once
#include <exception>
#include <string>
#include "enum2str.h"
#include "nameid.h"

namespace zenith
{
	namespace util
	{
		class StrCastException : public std::exception
		{
		public:
			StrCastException() : std::exception("StrCastException: unknown cause") {}
			StrCastException(const char * p) : std::exception(p) {}
			virtual ~StrCastException() {}
		};

		template<class From, class To>
		class str_cast_impl;

		template<class Char, class Alloc1, class Alloc2>
		class str_cast_impl<std::basic_string<Char, std::char_traits<Char>, Alloc1>, std::basic_string<Char, std::char_traits<Char>, Alloc2>>
		{
		public:
			inline static void cast(const std::basic_string<Char, std::char_traits<Char>, Alloc1> &from, std::basic_string<Char, std::char_traits<Char>, Alloc2> &to, size_t maxSize = 0)
			{
				to = from.c_str();
			}
		};
		template<>
		class str_cast_impl<const char *, float>
		{
		public:
			inline static void cast(const char * from, float &to, size_t maxSize = 0)
			{
				char * end;
				to = std::strtof(from, &end);
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<const char *, double>
		{
		public:
			inline static void cast(const char * from, double &to, size_t maxSize = 0)
			{
				char * end;
				to = std::strtod(from, &end);
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<float, char *>
		{
		public:
			inline static void cast(float from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%g", double(from));
				if (res == 0)
					throw StrCastException("Failed to cast from float to char*");
			}
		};
		template<>
		class str_cast_impl<double, char *>
		{
		public:
			inline static void cast(double from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%g", double(from));
				if (res == 0)
					throw StrCastException("Failed to cast from double to char*");
			}
		};


		template<>
		class str_cast_impl<const char *, long long int>
		{
		public:
			inline static void cast(const char * from, long long int &to, size_t maxSize = 0)
			{
				char * end;
				to = std::strtoll(from, &end, 10);
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<long long int, char *>
		{
		public:
			inline static void cast(long long int from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%lld", from);
				if (res == 0)
					throw StrCastException("Failed to cast from long long int to char*");
			}
		};

		template<>
		class str_cast_impl<const char *, long int>
		{
		public:
			inline static void cast(const char * from, long int &to, size_t maxSize = 0)
			{
				char * end;
				to = std::strtol(from, &end, 10);
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<long int, char *>
		{
		public:
			inline static void cast(long int from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%ld", from);
				if (res == 0)
					throw StrCastException("Failed to cast from long int to char*");
			}
		};

		template<>
		class str_cast_impl<const char *, int>
		{
		public:
			inline static void cast(const char * from, int &to, size_t maxSize = 0)
			{
				char * end;
				to = std::strtol(from, &end, 10);
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<int, char *>
		{
		public:
			inline static void cast(int from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%d", from);
				if (res == 0)
					throw StrCastException("Failed to cast from int to char*");
			}
		};

		template<>
		class str_cast_impl<const char *, int8_t>
		{
		public:
			inline static void cast(const char * from, int8_t &to, size_t maxSize = 0)
			{
				char * end;
				to = static_cast<int8_t>(std::strtol(from, &end, 10));
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<int8_t, char *>
		{
		public:
			inline static void cast(int8_t from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%d", static_cast<int>(from));
				if (res == 0)
					throw StrCastException("Failed to cast from int to char*");
			}
		};

		
		template<>
		class str_cast_impl<const char *, unsigned long long int>
		{
		public:
			inline static void cast(const char * from, unsigned long long int &to, size_t maxSize = 0)
			{
				char * end;
				to = std::strtoull(from, &end, 10);
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<unsigned long long int, char *>
		{
		public:
			inline static void cast(unsigned long long int from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%llu", from);
				if (res == 0)
					throw StrCastException("Failed to cast from unsigned long long int to char*");
			}
		};

		template<>
		class str_cast_impl<const char *, unsigned long int>
		{
		public:
			inline static void cast(const char * from, unsigned long int &to, size_t maxSize = 0)
			{
				char * end;
				to = std::strtoul(from, &end, 10);
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<unsigned long int, char *>
		{
		public:
			inline static void cast(unsigned long int from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%lu", from);
				if (res == 0)
					throw StrCastException("Failed to cast from unsigned long int to char*");
			}
		};

		template<>
		class str_cast_impl<const char *, unsigned int>
		{
		public:
			inline static void cast(const char * from, unsigned int &to, size_t maxSize = 0)
			{
				char * end;
				to = std::strtoul(from, &end, 10);
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<unsigned int, char *>
		{
		public:
			inline static void cast(unsigned int from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%u", from);
				if (res == 0)
					throw StrCastException("Failed to cast from unsigned int to char*");
			}
		};

		template<>
		class str_cast_impl<const char *, unsigned short int>
		{
		public:
			inline static void cast(const char * from, unsigned short int &to, size_t maxSize = 0)
			{
				char * end;
				to = static_cast<unsigned short>(std::strtoul(from, &end, 10));
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<unsigned short int, char *>
		{
		public:
			inline static void cast(unsigned short int from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%hu", from);
				if (res == 0)
					throw StrCastException("Failed to cast from unsigned int to char*");
			}
		};


		template<>
		class str_cast_impl<const char *, bool>
		{
		public:
			inline static void cast(const char * from, bool &to, size_t maxSize = 0)
			{
				if (from[0] == '0')
				{
					to = false;
					return;
				}
				if (from[0] == '1')
				{
					to = true;
					return;
				}
				if ((from[0] == 'T') || (from[0] == 't'))
					if((from[1] == 'R') || (from[1] == 'r'))
						if ((from[2] == 'U') || (from[2] == 'u'))
							if ((from[3] == 'E') || (from[3] == 'e'))
							{
								to = true;
								return;
							}
				if ((from[0] == 'F') || (from[0] == 'f'))
					if ((from[1] == 'A') || (from[1] == 'a'))
						if ((from[2] == 'L') || (from[2] == 'l'))
							if ((from[3] == 'S') || (from[3] == 's'))
								if ((from[4] == 'E') || (from[4] == 'e'))
								{
									to = false;
									return;
								}
				throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<bool, char *>
		{
		public:
			inline static void cast(bool from, char * to, size_t maxSize)
			{
				if(maxSize <= 6)
					throw StrCastException("Not enough buffer!");
				if (from)
				{
					to[0] = 'T';
					to[1] = 'R';
					to[2] = 'U';
					to[3] = 'E';
					to[4] = '\0';
				}
				else
				{
					to[0] = 'F';
					to[1] = 'A';
					to[2] = 'L';
					to[3] = 'S';
					to[4] = 'E';
					to[5] = '\0';
				}				
			}
		};


		template<>
		class str_cast_impl<const char *, void *>
		{
		public:
			inline static void cast(const char * from, void * &to, size_t maxSize = 0)
			{
				char * end;
				to = reinterpret_cast<void*>(std::strtoull(from, &end, 10));
				if (end == from)
					throw StrCastException(from);
			}
		};
		template<>
		class str_cast_impl<void *, char *>
		{
		public:
			inline static void cast(void * from, char * to, size_t maxSize)
			{
				size_t res = std::snprintf(to, maxSize, "%p", from);
				if (res == 0)
					throw StrCastException("Failed to cast from void* to char*");
			}
		};





		template<class T, class Alloc>
		class str_cast_impl<T, std::basic_string<char, std::char_traits<char>, Alloc>>
		{
		public:
			inline static void cast(const T &from, std::basic_string<char, std::char_traits<char>, Alloc> &to, size_t maxSize = 0)
			{
				char alignas(64) buff[128];
				str_cast_impl<T, char *>::cast(from, buff, 128);
				to = buff;
			}
		};
		template<class T, class Alloc>
		class str_cast_impl<std::basic_string<char, std::char_traits<char>, Alloc>, T>
		{
		public:
			inline static void cast(const std::basic_string<char, std::char_traits<char>, Alloc> &from, T &to, size_t maxSize = 0)
			{
				str_cast_impl<const char *, T>::cast(from.c_str(), to);
			}
		};

		template<>
		class str_cast_impl<const char *, const char *>
		{
		public:
			inline static void cast(const char * from, const char * &to, size_t maxSize = 0)
			{
				to = from;
			}
		};

		template<class From, class ToEnum>
		class str_cast_impl_to_enum
		{
		public:
			inline static void cast(const From &from, ToEnum &to, size_t maxSize = 0)
			{
				to = enum2str_converter<From, ToEnum>::to_enum(from);
			}
		};
		template<class ToEnum>
		class str_cast_impl_to_enum<const char *, ToEnum>
		{
		public:
			inline static void cast(const char * from, ToEnum &to, size_t maxSize = 0)
			{
				to = enum2str_converter<std::string, ToEnum>::to_enum(std::string(from));
			}
		};
		template<class ToEnum, class A>
		class str_cast_impl_to_enum<std::basic_string<char, std::char_traits<char>, A>, ToEnum>
		{
		public:
			inline static void cast(const std::basic_string<char, std::char_traits<char>, A> &from, ToEnum &to, size_t maxSize = 0)
			{
				to = enum2str_converter<std::string, ToEnum>::to_enum(std::string(from.c_str(), from.length()));
			}
		};
		template<class FromEnum, class To>
		class str_cast_impl_from_enum
		{
		public:
			inline static void cast(const FromEnum &from, To &to, size_t maxSize = 0)
			{
				to = enum2str_converter<To, FromEnum>::to_string(from);
			}
		};
		template<class FromEnum>
		class str_cast_impl_from_enum<FromEnum, char *>
		{
		public:
			inline static void cast(const FromEnum &from, char *to, size_t maxSize = 0)
			{
				const std::string &res = enum2str_converter<std::string, FromEnum>::to_string(from);
				if (maxSize <= res.length())
					throw StrCastException("Not enought characters to convert enum to char*!");
				size_t i = 0;
				for (; i < res.length(); i++)
					to[i] = res[i];
				to[i] = '\0';
			}
		};
		template<class FromEnum, size_t N>
		class str_cast_impl_from_enum<FromEnum, char [N]>
		{
		public:
			inline static void cast(const FromEnum &from, char to[N], size_t maxSize = 0)
			{
				const std::string &res = enum2str_converter<std::string, FromEnum>::to_string(from);
				if (N <= res.length())
					throw StrCastException("Not enought characters to convert enum to char*!");
				size_t i = 0;
				for (; i < res.length(); i++)
					to[i] = res[i];
				to[i] = '\0';
			}
		};

		template<class From, class To, class = void>
		class str_cast_impl_s1 : public str_cast_impl<From, To> {};

		template<class From, class To>
		class str_cast_impl_s1<From, To, typename std::enable_if<std::is_enum<To>::value>::type> : public str_cast_impl_to_enum<From, To> {};		

		template<class From, class To>
		class str_cast_impl_s1<From, To, typename std::enable_if<std::is_enum<From>::value>::type> : public str_cast_impl_from_enum<From, To> {};






		template<class From, class To>
		class str_cast_impl_s2 : public str_cast_impl_s1<From, To> {};

		template<class To, size_t N>
		class str_cast_impl_s2<char[N],To> : public str_cast_impl_s1<const char *, To> {};

		template<class From, size_t N>
		class str_cast_impl_s2<From, char[N]> : public str_cast_impl_s1<From, char *> {};

		template<class From, class To>
		class str_cast_impl_s3 : public str_cast_impl_s2<From, To> {};

		template<class Char, class A1, class A2>
		class str_cast_impl_s3<std::basic_string<Char, std::char_traits<Char>, A1>, std::basic_string<Char, std::char_traits<Char>, A2>>
		{
		public:
			inline static void cast(const std::basic_string<Char, std::char_traits<Char>, A1> &from, std::basic_string<Char, std::char_traits<Char>, A2> &to, size_t maxSize)
			{
				to = from.c_str();
			}
		};

		template<class Char, class A>
		class str_cast_impl_s3<const Char *, std::basic_string<Char, std::char_traits<Char>, A>>
		{
		public:
			inline static void cast(const char *from, std::basic_string<Char, std::char_traits<Char>, A> &to, size_t maxSize)
			{
				to = from;
			}
		};

		template<class To>
		class str_cast_impl_s3<zenith::util::nameid, To>
		{
		public:
			inline static void cast(const zenith::util::nameid &from, To &to, size_t maxSize)
			{
				str_cast_impl_s3<const char *, To>::cast(from.c_str(), to, maxSize);
			}
		};
		template<class From>
		class str_cast_impl_s3<From, zenith::util::nameid>
		{
		public:
			inline static void cast(const From &from, zenith::util::nameid &to, size_t maxSize)
			{
				std::string str;
				str_cast_impl_s3<From, std::string>::cast(from, str, maxSize);
				to = str;
			}
		};
				

		template<class To, class From>
		inline To str_cast(const From &from, To def = To())
		{
			str_cast_impl_s3<From, To>::cast(from, def);
			return def;
		}
		template<class To, class From>
		inline void str_cast(const From &from, To &to, size_t maxSize=0)
		{
			str_cast_impl_s3<From, To>::cast(from, to, maxSize);
		}
	}
}
