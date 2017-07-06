#pragma once
#include "ioconv_base.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			template<class In, class Out>
			inline void io_transform(In &&inIt, Out &&outIt)
			{
				auto p = inIt.children();
				for (auto i = p.first; i != p.second; ++i)
					if (i.type() == NodeType::COMPLEX)
						io_transform(i, outIt.append_complex(i.key(), i.complex_hint()));
					else if (i.type() == NodeType::VALUE)
						outIt.append_value(i.key(), i.value(), i.value_hint());
			}
		}
	}
}