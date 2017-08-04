#pragma once
#include "ioconv_base.h"
#include "../FileFormat/ff_zds_utils.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
		
			class input_zds_unnamed;
			class input_zds_root;

			class input_zds_named
			{
				friend class input_zds_unnamed;
				friend class input_zds_root;
				zfile_format::zDataStorage::const_fixed_iterator impl_;
				inline const zfile_format::zDataStorage::const_fixed_iterator &checked_() const
				{
					if (!impl_.valid())
						throw IConvInvalidIteratorException("input_zds_named is empty!");
					return impl_;
				}
				input_zds_named(const zfile_format::zDataStorage::const_fixed_iterator &i) : impl_(i) {}
			public:
				typedef input_zds_unnamed unnamed_iterator;
				typedef input_zds_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::NAMED;
				static const InternalType internal_type = InternalType::BINARY;

				template<class It>
				input_zds_named(const It &n, const char * key) : impl_(n.child_begin(key))
				{
				}
				input_zds_named() {}
				input_zds_named(const input_zds_named &i) : impl_(i.impl_) {}

				inline bool empty() const { return !impl_.valid(); }
				inline NodeType type() const
				{
					if (empty())
						return NodeType::NONE;
					if (!impl_.has_children())
						return NodeType::VALUE;
					else
						return NodeType::COMPLEX;
					return NodeType::NONE;
				}
				inline NodeValueHint value_hint() const
				{
					if (!empty() && impl_.is_attribute())
						return NodeValueHint::ATTRIBUTE;
					return NodeValueHint::NONE;
				}
				inline NodeComplexHint complex_hint() const { return NodeComplexHint::NONE; }

				inline input_zds_named next() const { return input_zds_named(impl_.next()); }
				inline input_zds_named &operator++() { impl_ = impl_.next(); return *this; }
				inline input_zds_named operator++(int) { auto tmp = *this; impl_ = impl_.next(); return tmp; }
				inline bool operator ==(const input_zds_named &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_zds_named &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_zds_named, input_zds_named> children(const char * key) const;
				inline std::pair<input_zds_unnamed, input_zds_unnamed> children() const;

				inline uint64_t size() const { return impl_.size(); }
				inline const uint8_t * data() const { return impl_.data(); }
				template<class T>
				inline void read(T &val) const { impl_.read(val); }
				inline void copy_data(uint8_t * dst, uint64_t sz) const { impl_.copy_data_from_chunk(dst, sz); }
				inline const char * key() const { return impl_.tag_name(); }
			};


			class input_zds_unnamed
			{
				friend class input_zds_named;
				friend class input_zds_root;
				zfile_format::zDataStorage::const_iterator impl_;
				inline const zfile_format::zDataStorage::const_iterator &checked_() const
				{
					if (!impl_.valid())
						throw IConvInvalidIteratorException("input_zds_unnamed is empty!");
					return impl_;
				}
				//input_zds_unnamed(const zfile_format::zDataStorage::const_iterator &i) : impl_(i) {}
			public:
				typedef input_zds_unnamed unnamed_iterator;
				typedef input_zds_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::UNNAMED;
				static const InternalType internal_type = InternalType::BINARY;

				input_zds_unnamed(const zfile_format::zDataStorage::const_iterator &n) : impl_(n) {}
				input_zds_unnamed()  {}
				input_zds_unnamed(const input_zds_unnamed &i) : impl_(i.impl_) {}
				//input_zds_unnamed(const input_zds_named &i);

				inline bool empty() const { return !impl_.valid(); }
				inline NodeType type() const
				{
					if (empty())
						return NodeType::NONE;
					if (!impl_.has_children())
						return NodeType::VALUE;
					else
						return NodeType::COMPLEX;
					return NodeType::NONE;
				}
				inline NodeValueHint value_hint() const
				{
					if (!empty() && impl_.is_attribute())
						return NodeValueHint::ATTRIBUTE;
					return NodeValueHint::NONE;
				}
				inline NodeComplexHint complex_hint() const { return NodeComplexHint::NONE; }

				inline input_zds_unnamed next() const { return input_zds_unnamed(impl_.next()); }
				inline input_zds_unnamed &operator++() { impl_ = impl_.next(); return *this; }
				inline input_zds_unnamed operator++(int) { auto tmp = *this; impl_ = impl_.next(); return tmp; }
				inline bool operator ==(const input_zds_unnamed &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_zds_unnamed &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_zds_named, input_zds_named> children(const char * key) const;
				inline std::pair<input_zds_unnamed, input_zds_unnamed> children() const;

				inline uint64_t size() const { return impl_.size(); }
				inline const uint8_t * data() const { return impl_.data(); }
				template<class T>
				inline void read(T &val) const { impl_.read(val); }
				inline void copy_data(uint8_t * dst, uint64_t sz) const { impl_.copy_data_from_chunk(dst, sz); }
				inline const char * key() const { return impl_.tag_name(); }
			};

			class input_zds_root
			{
				zfile_format::zDataStorage::const_iterator impl_;
			public:
				typedef input_zds_unnamed unnamed_iterator;
				typedef input_zds_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::UNNAMED;
				static const InternalType internal_type = InternalType::BINARY;

				input_zds_root(const zfile_format::zDataStorage::const_iterator &n) : impl_(n) {}
				input_zds_root() {}
				//input_zds_unnamed(const input_zds_named &i);

				inline bool empty() const { return !impl_.valid(); }
				inline NodeType type() const
				{
					if (empty())
						return NodeType::NONE;
					return NodeType::COMPLEX;
				}
				inline NodeValueHint value_hint() const
				{
					return NodeValueHint::NONE;
				}
				inline NodeComplexHint complex_hint() const { return NodeComplexHint::NONE; }

				inline input_zds_root next() const { return input_zds_root(impl_.next()); }
				inline input_zds_root &operator++() { impl_ = impl_.next(); return *this; }
				inline input_zds_root operator++(int) { auto tmp = *this; impl_ = impl_.next(); return tmp; }
				inline bool operator ==(const input_zds_root &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_zds_root &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_zds_named, input_zds_named> children(const char * key) const;
				inline std::pair<input_zds_unnamed, input_zds_unnamed> children() const;

				inline uint64_t size() const { return impl_.size(); }
				inline const uint8_t * data() const { return impl_.data(); }
				template<class T>
				inline void read(T &val) const { impl_.read(val); }
				inline void copy_data(uint8_t * dst, uint64_t sz) const { impl_.copy_data_from_chunk(dst, sz); }
				inline const char * key() const { return impl_.tag_name(); }
			};

			inline std::pair<input_zds_named, input_zds_named> input_zds_root::children(const char * key) const
			{
				return std::make_pair(input_zds_named(impl_.child_begin(key)), input_zds_named(impl_.child_end(key)));
			}
			inline std::pair<input_zds_unnamed, input_zds_unnamed> input_zds_root::children() const
			{
				return std::make_pair(input_zds_unnamed(impl_.child_begin()), input_zds_unnamed(impl_.child_end()));
			}

			inline std::pair<input_zds_named, input_zds_named> input_zds_unnamed::children(const char * key) const
			{
				return std::make_pair(input_zds_named(impl_.child_begin(key)), input_zds_named(impl_.child_end(key)));
			}
			inline std::pair<input_zds_unnamed, input_zds_unnamed> input_zds_unnamed::children() const
			{
				return std::make_pair(input_zds_unnamed(impl_.child_begin()), input_zds_unnamed(impl_.child_end()));
			}

			inline std::pair<input_zds_named, input_zds_named> input_zds_named::children(const char * key) const
			{
				return std::make_pair(input_zds_named(impl_.child_begin(key)), input_zds_named(impl_.child_end(key)));
			}
			inline std::pair<input_zds_unnamed, input_zds_unnamed> input_zds_named::children() const
			{
				return std::make_pair(input_zds_unnamed(impl_.child_begin()), input_zds_unnamed(impl_.child_end()));
			}
		}
	}
}