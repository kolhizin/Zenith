#pragma once

#include <exception>
#include <map>
#include "macro_foreach.h"


#define ZENITH_ENUM2STR_START(type)\
		template<class Str>\
		class zenith::util::enum2str_init<Str, type>\
		{\
		public:\
			inline static Str name_()\
			{\
				return #type;\
			}\
			inline static bool check_name_(const Str &in)\
			{\
				return in == #type;\
			}\
			inline static std::pair<std::map<Str, type>, std::map<type, Str>> init_conv_()\
			{\
				std::map<Str, type> str2enum;\
				std::map<type, Str> enum2str;\
				typedef type enum_type;

#define ZENITH_ENUM2STR_END\
				return std::make_pair(str2enum, enum2str);\
			}\
		};

#define ZENITH_ENUM2STR_ELEM(x) str2enum[#x] = enum_type::x; enum2str[enum_type::x] = #x;

#define ZENITH_ENUM2STR(type,...)\
	ZENITH_ENUM2STR_START(type)\
		ZENITH_FOR_EACH(ZENITH_ENUM2STR_ELEM, __VA_ARGS__)\
	ZENITH_ENUM2STR_END


namespace zenith
{

	namespace util
	{
		template<class S, class T>
		class enum2str_init;		

		template<class Str, class E>
		class enum2str_converter
		{
		public:
			inline static const Str &to_string(E e)
			{
				static const std::map<E, Str> enum2str = enum2str_init<Str, E>::init_conv_().second;
				return enum2str.at(e);
			}
			inline static E to_enum(const Str &in)
			{
				static const std::map<Str, E> str2enum = enum2str_init<Str, E>::init_conv_().first;
				return str2enum.at(in);
			}
			inline static bool to_enum_safe(const Str &in, E &out, E def = E())
			{
				static const std::map<Str, E> str2enum = enum2str_init<Str, E>::init_conv_().first;
				auto it = str2enum.find(in);
				if (it == str2enum.end())
				{
					out = def;
					return false;
				}
				out = it.second;
				return true;
			}
		};

		template<class Str, class E>
		inline void enum2str(E in, Str &out)
		{
			out = enum2str_converter<Str, E>::to_string(in);
		}
		template<class Str, class E>
		inline void str2enum(const Str &in, E &out)
		{
			out = enum2str_converter<Str, E>::to_enum(in);
		}
		template<class Str, class E>
		inline bool str2enum_safe(const Str &in, E &out, E def = E())
		{
			return enum2str_converter<Str, E>::to_enum_safe(in, out, def);
		}
		
		template<class Str, class E>
		inline void str2bitenum(const Str &in, E &out)
		{
			out = static_cast<E>(0);
			std::string r;
			size_t off = 0;
			while (off < in.length())
			{
				E tmp;
				size_t t_off = in.find('|', off);
				str2enum(in.substr(off, t_off - off), tmp);
				off = t_off;
				if (off < in.length())
					off++;

				out = static_cast<E>(static_cast<uint64_t>(out) | static_cast<uint64_t>(tmp));
			}
		}
		template<class Str, class E>
		inline void bitenum2str(E in, Str &out)
		{
			uint64_t res = static_cast<uint64_t>(in);
			uint64_t msk = 1;
			bool empty = true;
			while (res)
			{
				uint64_t tmp = res & msk;
				if (tmp > 0)
				{
					if (empty)
					{
						enum2str(static_cast<E>(tmp), out);
						empty = false;
					}
					else
					{
						Str tmps;
						enum2str(static_cast<E>(tmp), tmps);
						out += "|";
						out += tmps;
					}
					res ^= tmp;
				}
				msk <<= 1;
			}
		}


		enum class ExtendedBitMask { NO = 0, YES = 1, ANY = -1 };
		ZENITH_ENUM2STR_START(zenith::util::ExtendedBitMask)
			ZENITH_ENUM2STR_ELEM(NO)
			ZENITH_ENUM2STR_ELEM(YES)
			ZENITH_ENUM2STR_ELEM(ANY)
		ZENITH_ENUM2STR_END
	}
}
