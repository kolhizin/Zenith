#pragma once
#include "ioconv_base.h"
#include "io_handlers.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{			
			template<class It, class T>
			inline void output_single(const T &val, It &iter)
			{
				io_handler<T>::output(val, iter);
			}
			template<class It, class Cont>
			inline void output_multiple(const Cont &cont, It &iter, const char * name)
			{
				typedef typename Cont::value_type val_type;
				
				for(auto it = cont.begin(); it != cont.end(); ++it)
					io_handler<val_type>::output(*it, io_add_node<io_handler_impl<val_type, It, It::internal_type>::node_type>::add(iter, name));
			}
			template<NodeType n>
			class output_multiple_map_
			{
			public:
			};
			template<>
			class output_multiple_map_<NodeType::VALUE>
			{
			public:
				template<class It, class Cont>
				inline static void output_(const Cont &cont, It &iter, const char * name, const char * key, const char * value)
				{
					typedef typename Cont::mapped_type val_type;
					typedef typename Cont::key_type key_type;
					if (!value)
						throw IOConvException("value-name is not specified for output_multiple_map!");
					for (auto it = cont.begin(); it != cont.end(); ++it)
					{
						auto iter0 = io_add_node<io_handler<val_type>::node_type>::add(iter, name);
						io_handler<key_type>::output(it->first, io_add_node<io_handler_impl<key_type, It, It::internal_type>::node_type>::add(iter0, key));
						io_handler<val_type>::output(it->second, iter0.append_value(value));
					}
				}
			};
			template<>
			class output_multiple_map_<NodeType::COMPLEX>
			{
			public:
				template<class It, class Cont>
				inline static void output_(const Cont &cont, It &iter, const char * name, const char * key, const char * value)
				{
					typedef typename Cont::mapped_type val_type;
					typedef typename Cont::key_type key_type;
					for (auto it = cont.begin(); it != cont.end(); ++it)
					{
						auto iter0 = io_add_node<io_handler<val_type>::node_type>::add(iter, name);
						io_handler<key_type>::output(it->first, io_add_node<io_handler_impl<val_type, It, It::internal_type>::node_type>::add(iter0, key));
						io_handler<val_type>::output(it->second, iter0);
					}
				}
			};

			template<class It, class Cont>
			inline void output_multiple_map(const Cont &cont, It &iter, const char * name, const char * key, const char * value = nullptr)
			{
				typedef typename Cont::mapped_type val_type;
				typedef typename Cont::key_type key_type;

				output_multiple_map_<io_handler<val_type>::node_type>::output_(cont, iter, name, key, value);
			}
			
			/*
			template<class It, class T>
			inline void input_named_optional(T &val, const It &iter, const char * key)
			{
				auto p = iter.children(key);
				auto it = ensure_at_most_one(std::move(p.first), std::move(p.second));
				if (check_at_least_one(std::move(p.first), std::move(p.second)))
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
			*/
		}
	}
}