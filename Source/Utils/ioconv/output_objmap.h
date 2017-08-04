#pragma once
#include "ioconv_base.h"
#include "../object_map.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			class output_objmap_value
			{
				typename ObjectMap<char, char>::val_iterator it_;
			public:
				static const InternalType internal_type = InternalType::STRING;
				inline output_objmap_value(typename ObjectMap<char, char>::val_iterator &it) : it_(it) {}
				inline void set_value(const char * value)
				{
					it_->second.first = value;
				}
			};
			class output_objmap
			{
				ObjectMap<char, char> * impl_;
				inline ObjectMap<char, char> &checked_()
				{
					if (!impl_)
						throw OConvInvalidIteratorException("input_xml_unnamed is empty!");
					return *impl_;
				}
				output_objmap(ObjectMap<char, char> *i) : impl_(i) {}
			public:
				typedef output_objmap unnamed_iterator;
				typedef output_objmap named_iterator;
				static const IteratorType iterator_type = IteratorType::OUTPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::NONE;
				static const InternalType internal_type = InternalType::STRING;

				output_objmap() : impl_(nullptr) {}
				output_objmap(ObjectMap<char, char> &objmap) : impl_(&objmap) {}
				output_objmap(const output_objmap &i) : impl_(i.impl_) {}

				inline ObjectMap<char, char> *impl_checked() { return &checked_(); }
				inline ObjectMap<char, char> *impl_raw() { return impl_; }

				inline output_objmap append_complex(const char * key, NodeComplexHint hint = NodeComplexHint::NONE)
				{
					return output_objmap(&checked_().addObject(key));
				}
				inline output_objmap_value append_value(const char * key, const char * value, NodeValueHint hint = NodeValueHint::NONE)
				{
					ObjectMapValueHint h = ObjectMapValueHint::UNDEF;
					if (hint == NodeValueHint::ATTRIBUTE)
						h = ObjectMapValueHint::ATTR;
					return output_objmap_value(checked_().addValue(key, value, h));
				}
				inline output_objmap_value append_value(const char * key, NodeValueHint hint = NodeValueHint::NONE)
				{
					ObjectMapValueHint h = ObjectMapValueHint::UNDEF;
					if (hint == NodeValueHint::ATTRIBUTE)
						h = ObjectMapValueHint::ATTR;
					return output_objmap_value(checked_().addValue(key, "", h));
				}
			};

		}
	}
}