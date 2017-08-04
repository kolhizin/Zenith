#pragma once
#include "ioconv_base.h"
#include "../FileFormat/ff_zds_utils.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			class output_zds_value
			{
				zfile_format::zDataStorage::iterator iter_;
			public:
				static const InternalType internal_type = InternalType::BINARY;
				
				inline output_zds_value &as_value() { return *this; }
				inline const output_zds_value &as_value() const { return *this; }

				output_zds_value(const zfile_format::zDataStorage::iterator &n) : iter_(n){}
				inline void reset_size(uint64_t sz)	{iter_.reset_size(sz);}
				inline void copy_data(const uint8_t * src, uint64_t sz){	iter_.copy_data_to_chunk(src, sz);}
				template<class T>
				inline void write(const T &val)	{iter_.write(val);}
				inline uint8_t * data() { return iter_.data(); }
			};
			class output_zds
			{
				zfile_format::zDataStorage::iterator iter_;
				inline zfile_format::zDataStorage::iterator &checked_()
				{
					if (!iter_.valid())
						throw OConvInvalidIteratorException("output_zds is empty!");
					return iter_;
				}
				//output_xml(pugi::xml_node &i) : node_(i) {}
			public:
				typedef output_zds unnamed_iterator;
				typedef output_zds named_iterator;
				static const IteratorType iterator_type = IteratorType::OUTPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::NONE;
				static const InternalType internal_type = InternalType::BINARY;

				output_zds() {}
				output_zds(const zfile_format::zDataStorage::iterator &iter) : iter_(iter) {}
				output_zds(const output_zds &i) : iter_(i.iter_) {}

				inline output_zds_value as_value() { return output_zds_value(iter_); }


				inline zfile_format::zDataStorage::iterator &impl_checked() { return checked_(); }
				inline zfile_format::zDataStorage::iterator &impl_raw() { return iter_; }

				inline output_zds append_complex(const char * key, NodeComplexHint hint = NodeComplexHint::NONE)
				{
					return output_zds(checked_().append(key, 4));
				}
				inline output_zds_value append_value(const char * key, NodeValueHint hint = NodeValueHint::NONE)
				{
					return output_zds_value(iter_.append(key, 4, (hint == NodeValueHint::ATTRIBUTE ? true : false)));
				}
			};

		}
	}
}