#pragma once
#include "ioconv_base.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			template<class In, class Out, InternalType itIn, InternalType itOut>
			class io_transform_impl_;

			template<class In, class Out>
			class io_transform_impl_<In, Out, InternalType::STRING, InternalType::STRING>
			{
			public:
				static void impl(const In &inIt, Out &outIt)
				{
					outIt.set_value(inIt.value());
				}
			};
			template<class In, class Out>
			class io_transform_impl_<In, Out, InternalType::BINARY, InternalType::BINARY>
			{
			public:
				static void impl(const In &inIt, Out &outIt)
				{
					uint64_t sz = inIt.size();
					outIt.reset_size(sz);
					outIt.copy_data(inIt.data(), sz);
				}
			};
			template<class In, class Out>
			class io_transform_impl_<In, Out, InternalType::STRING, InternalType::BINARY>
			{
			public:
				static void impl(const In &inIt, Out &outIt)
				{
					uint64_t sz = zenith::util::zstrlen(inIt.value()) + 1;
					outIt.reset_size(sz);
					outIt.copy_data(reinterpret_cast<const uint8_t *>(inIt.value()), sz);
				}
			};
			template<class In, class Out>
			class io_transform_impl_<In, Out, InternalType::BINARY, InternalType::STRING>
			{
			public:
				static void impl(const In &inIt, Out &outIt)
				{
					uint64_t sz = inIt.size();
					const char * ptr = reinterpret_cast<const char *>(inIt.data());
					if (ptr[sz - 1] != '\0')
						outIt.set_value("BINARY-DATA");
					else
						outIt.set_value(ptr);
				}
			};

			
			template<class In, class Out>
			inline void io_transform_(const In &inIt, Out &outIt)
			{
				io_transform_impl_<In, Out, In::internal_type, Out::internal_type>::impl(inIt, outIt);
			}

			template<class In, class Out>
			inline void io_transform(In &&inIt, Out &&outIt)
			{
				auto p = inIt.children();
				for (auto i = p.first; i != p.second; ++i)
					if (i.type() == NodeType::COMPLEX)
						io_transform(i, outIt.append_complex(i.key(), i.complex_hint()));
					else if (i.type() == NodeType::VALUE)
					{
						//i.value(),
						auto vout = outIt.append_value(i.key(), i.value_hint());
						io_transform_(i, vout);
					}
			}
		}
	}
}