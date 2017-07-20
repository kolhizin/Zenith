#pragma once
#include "ioconv_base.h"
#include "io_handlers.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			template<class It>
			inline bool check_zero(It &&begin, It &&end) { return (begin == end); }
			template<class It>
			inline bool check_at_least_one(It &&begin, It &&end) { return (begin != end); }
			template<class It>
			inline bool check_at_most_one(It &&begin, It &&end)
			{
				if (begin == end)
					return true;
				auto it = begin; ++it;
				return (it == end);
			}
			template<class It>
			inline bool check_one(It &&begin, It &&end)
			{
				if (begin == end)
					return false;
				auto it = begin; ++it;
				return (it == end);
			}


			template<class It>
			inline It ensure_zero(It &&begin, It &&end)
			{
				if (check_zero(std::move(begin), std::end(end)))
					return begin;
				throw IOConvBadStructureException("ensure_zero: expected begin == end!");
			}
			template<class It>
			inline It ensure_at_least_one(It &&begin, It &&end)
			{
				if (check_at_least_one(std::move(begin), std::move(end)))
					return begin;
				throw IOConvBadStructureException("ensure_at_least_one: expected begin < end!");
			}
			template<class It>
			inline It ensure_at_most_one(It &&begin, It &&end)
			{
				if (check_at_most_one(std::move(begin), std::move(end)))
					return begin;
				throw IOConvBadStructureException("ensure_at_most_one: expected begin + 1 >= end!");
			}
			template<class It>
			inline It ensure_one(It &&begin, It &&end)
			{
				if (check_one(std::move(begin), std::move(end)))
					return begin;
				throw IOConvBadStructureException("ensure_one: expected begin + 1 = end!");
			}

			
			template<class It>
			inline bool check_presence(It &&begin, It &&end, PresenceType type)
			{
				switch (type)
				{
				case zenith::util::ioconv::PresenceType::NONE: return true;
				case zenith::util::ioconv::PresenceType::ZERO: return check_zero(std::move(begin), std::move(end));
				case zenith::util::ioconv::PresenceType::ONE: return check_one(std::move(begin), std::move(end));
				case zenith::util::ioconv::PresenceType::ONE_OR_ZERO: return check_at_most_one(std::move(begin), std::move(end));
				case zenith::util::ioconv::PresenceType::ZERO_PLUS: return true;
				case zenith::util::ioconv::PresenceType::ONE_PLUS: return check_at_least_one(std::move(begin), std::move(end));
				}
				return false;
			}
			template<class It>
			inline It ensure_single(It &&begin, It &&end, PresenceType type)
			{
				if (check_presence(std::move(begin), std::move(end), type))
					return begin;
				throw IOConvBadStructureException("ensure_presence: check failed!");
			}
			template<class It>
			inline std::pair<It, It> ensure_multiple(It &&begin, It &&end, PresenceType type)
			{
				if (check_presence(std::move(begin), std::move(end), type))
					return std::pair<It, It>(std::move(begin), std::move(end));
				throw IOConvBadStructureException("ensure_presence: check failed!");
			}

			template<class It>
			inline typename It::named_iterator get_named_single(It &&iter, const char * key, PresenceType type = PresenceType::ONE)
			{
				auto p = iter.children(key);
				return ensure_single(std::move(p.first), std::move(p.second), type);
			}
			template<class It>
			inline std::pair<typename It::named_iterator, typename It::named_iterator> get_named_multiple(It &&iter, const char * key, PresenceType type = PresenceType::ONE)
			{
				auto p = iter.children(key);
				return ensure_multiple(std::move(p.first), std::move(p.second), type);
			}

			template<class It>
			inline typename It::named_iterator get_named_single(const It &iter, const char * key, PresenceType type = PresenceType::ONE)
			{
				auto p = iter.children(key);
				return ensure_single(std::move(p.first), std::move(p.second), type);
			}

			template<class It>
			inline std::pair<typename It::named_iterator, typename It::named_iterator> get_named_multiple(const It &iter, const char * key, PresenceType type = PresenceType::ONE)
			{
				auto p = iter.children(key);
				return ensure_multiple(std::move(p.first), std::move(p.second), type);
			}
			template<class It, class T>
			inline void input(T &val, const It &iter)
			{
				io_handler<T>::input(val, iter);
			}
			template<class It, class T>
			inline void input_named_required(T &val, const It &iter, const char * key)
			{
				auto p = iter.children(key);
				auto it = ensure_one(std::move(p.first), std::move(p.second));
				io_handler<T>::input(val, it);
			}
			template<class It, class T>
			inline void input_named_optional(T &val, const It &iter, const char * key)
			{
				auto p = iter.children(key);
				auto it = ensure_at_most_one(std::move(p.first), std::move(p.second));
				if(check_at_least_one(std::move(p.first), std::move(p.second)))
					io_handler<T>::input(val, it);
			}

			template<class It, class T>
			inline void input_named_optional(T &val, const It &iter, const char * key, const T &def)
			{
				auto p = iter.children(key);
				auto it = ensure_at_most_one(std::move(p.first), std::move(p.second));
				if (check_at_least_one(std::move(p.first), std::move(p.second)))
					io_handler<T>::input(val, it);
				else
					val = def;
			}

			template<class It, class Cont>
			inline void input_named_multiple(Cont &cont, const It &iter, const char * key, PresenceType presence = PresenceType::ZERO_PLUS)
			{
				auto p = iter.children(key);
				ensure_multiple(std::move(p.first), std::move(p.second), presence);
				auto it = p.first;
				while (it != p.second)
				{
					typename Cont::value_type res;
					io_handler<typename Cont::value_type>::input(res, it);
					cont.emplace_back(std::move(res));
					++it;
				}
			}
			template<class It, class Cont>
			inline void input_named_multiple_map(Cont &cont, const It &iter, const char * key, const char * value_key, PresenceType presence = PresenceType::ZERO_PLUS)
			{
				auto p = iter.children(key);
				ensure_multiple(std::move(p.first), std::move(p.second), presence);
				auto it = p.first;
				while (it != p.second)
				{
					typename Cont::mapped_type res_value;
					typename Cont::key_type res_key;
					io_handler<typename Cont::key_type>::input(res_key, get_named_single(it, value_key));
					io_handler<typename Cont::mapped_type>::input(res_value, it);
					cont.insert(std::make_pair(std::move(res_key), std::move(res_value)));
					++it;
				}
			}
			template<class It, class Cont>
			inline void input_named_multiple_charmap(Cont &cont, const It &iter, const char * key, const char * value_key, PresenceType presence = PresenceType::ZERO_PLUS)
			{
				static const uint32_t BUFF_SIZE = 1024;
				char buff[BUFF_SIZE];
				auto p = iter.children(key);
				ensure_multiple(std::move(p.first), std::move(p.second), presence);
				auto it = p.first;
				while (it != p.second)
				{
					typename Cont::mapped_type res_value;
					typename Cont::key_type res_key = buff;
					io_handler<char[BUFF_SIZE]>::input(buff, get_named_single(it, value_key));
					io_handler<typename Cont::mapped_type>::input(res_value, it);
					cont.insert(std::make_pair(res_key, std::move(res_value)));
					++it;
				}
			}
		}
	}
}