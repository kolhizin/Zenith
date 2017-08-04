#pragma once
#include "ioconv_base.h"
#include "../str_cast.h"
#include "../str_utils.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			template<class T, class It>
			class io_handler_impl_str_
			{
				//default for most types
				static const size_t BufferSize_ = 1024;
			public:
				typedef T value_type;
				static const NodeType node_type = NodeType::VALUE;

				inline static void input(T &val, const It &it)
				{
					val = zenith::util::str_cast<T>(ensure_type(it, NodeType::VALUE).value());
				}
				inline static void output(const T &val, It &it)
				{
					static char CharBuffer_[BufferSize_];
					zenith::util::str_cast<char[BufferSize_]>(val, CharBuffer_, BufferSize_);
					it.set_value(CharBuffer_);
				}
			};
			template<uint32_t SZ, class It>
			class io_handler_impl_str_<char[SZ], It>
			{
			public:
				typedef std::string value_type;
				static const NodeType node_type = NodeType::VALUE; //value by default

				inline static void input(char val[SZ], const It &it)
				{
					zenith::util::zstrcpy(val, ensure_type(it, NodeType::VALUE).value());
				}
				inline static void output(const char val[SZ], It &it)
				{
					it.set_value(val);
				}
			};

			template<class It>
			class io_handler_impl_str_<std::string, It>
			{
			public:
				typedef std::string value_type;
				static const NodeType node_type = NodeType::VALUE; //value by default

				inline static void input(std::string &val, const It &it)
				{
					val = zenith::util::str_cast<std::string>(ensure_type(it, NodeType::VALUE).value());
				}
				inline static void output(const std::string &val, It &it)
				{
					it.set_value(val.c_str());
				}
			};

			template<class T, class It>
			class io_handler_impl_bin_
			{
			public:
				typedef T value_type;
				static const NodeType node_type = NodeType::VALUE;

				inline static void input(T &val, const It &it)
				{
					ensure_type(it, NodeType::VALUE).read(val);
				}
				inline static void output(const T &val, It &it)
				{
					it.reset_size(sizeof(T));
					it.write(val);
				}
			};
			template<uint32_t SZ, class It>
			class io_handler_impl_bin_<char[SZ], It>
			{
			public:
				typedef std::string value_type;
				static const NodeType node_type = NodeType::VALUE; //value by default

				inline static void input(char val[SZ], const It &it)
				{
					zenith::util::zstrcpy(val, reinterpret_cast<const char *>(ensure_type(it, NodeType::VALUE).data()));
				}
				inline static void output(const char val[SZ], It &it)
				{
					it.reset_size(zenith::util::zstrlen(val)+1);
					it.copy_data(reinterpret_cast<const uint8_t *>(val), zenith::util::zstrlen(val) + 1);
				}
			};

			template<class It>
			class io_handler_impl_bin_<std::string, It>
			{
			public:
				typedef std::string value_type;
				static const NodeType node_type = NodeType::VALUE; //value by default

				inline static void input(std::string &val, const It &it)
				{
					val = reinterpret_cast<const char *>(ensure_type(it, NodeType::VALUE).data());
				}
				inline static void output(const std::string &val, It &it)
				{
					it.reset_size(val.size() + 1);
					it.copy_data(val.c_str(), val.size() + 1);
				}
			};


			template<class T, class It, InternalType type>
			class io_handler_impl_route_;

			template<class T, class It>
			class io_handler_impl_route_<T, It, InternalType::STRING> : public io_handler_impl_str_<T, It> {};

			template<class T, class It>
			class io_handler_impl_route_<T, It, InternalType::BINARY> : public io_handler_impl_bin_<T, It> {};

			template<class T, class It, InternalType internal_type>
			class io_handler_impl : public io_handler_impl_route_<T, It, internal_type> {};
			/*
			template<class T, class It>
			class io_handler_impl
			{				
				//default for most types
				static const size_t BufferSize_ = 1024;
			public:
				typedef T value_type;
				static const NodeType node_type = NodeType::VALUE;

				inline static void input(T &val, const It &it)
				{
					val = zenith::util::str_cast<T>(ensure_type(it, NodeType::VALUE).value());
				}
				inline static void output(const T &val, It &it)
				{
					static char CharBuffer_[BufferSize_];
					zenith::util::str_cast<char[BufferSize_]>(val, CharBuffer_, BufferSize_);
					it.set_value(CharBuffer_);
				}
			};
			*/

			

			template<class T>
			class io_handler
			{
			public:
				typedef T value_type;

				template<class It>
				inline static void input(T &val, const It &it)
				{
					io_handler_impl<T, It, It::internal_type>::input(val, it);
				}
				template<class It>
				inline static void output(const T &val, It &it)
				{
					io_handler_impl<T, It, It::internal_type>::output(val, it);
				}
			};

			

			
			template<NodeType nodeType>
			class io_add_node
			{
			public:
				template<class It>
				inline static void add(It &&it, const char * name) {}
			};
			template<>
			class io_add_node<NodeType::COMPLEX>
			{
			public:
				template<class It>
				inline static auto add(It &&it, const char * name, NodeComplexHint hint = NodeComplexHint::NONE)
				{ return it.append_complex(name, hint); }
			};
			template<>
			class io_add_node<NodeType::VALUE>
			{
			public:
				template<class It>
				inline static auto add(It &&it, const char * name, NodeValueHint hint = NodeValueHint::NONE)
				{
					return it.append_value(name, hint);
				}
			};
		}
	}
}