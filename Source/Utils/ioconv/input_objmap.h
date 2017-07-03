#pragma once
#include "ioconv_base.h"
#include "../object_map.h"

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			class input_objmap_unnamed;
			class input_objmap_root;

			class input_objmap_named
			{
				friend class input_objmap_unnamed;
				friend class input_objmap_root;
				ObjectMap_Iterator<char, char> impl_;
				inline const ObjectMap_Iterator<char, char> &checked_() const
				{
					if (impl_.empty())
						throw IConvInvalidIteratorException("input_objmap_named is empty!");
					return impl_;
				}
				input_objmap_named(const ObjectMap_Iterator<char, char> &i) : impl_(i) {}
			public:
				typedef input_objmap_unnamed unnamed_iterator;
				typedef input_objmap_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::NAMED;

				input_objmap_named(const ObjectMap<char, char> &om, const char * key)
				{
					impl_ = om.children(key).first;
				}
				input_objmap_named(){}
				input_objmap_named(const input_objmap_named &i) : impl_(i.impl_) {}

				inline bool empty() const { return impl_.empty(); }
				inline NodeType type() const
				{
					if (empty())
						return NodeType::NONE;
					if (impl_.is_value())
						return NodeType::VALUE;
					if (impl_.is_object())
						return NodeType::COMPLEX;
					return NodeType::NONE;
				}
				inline NodeValueHint value_hint() const
				{
					if (!empty() && impl_.value_hint() == ObjectMapValueHint::ATTR)
						return NodeValueHint::ATTRIBUTE;
					return NodeValueHint::NONE;
				}
				inline NodeComplexHint complex_hint() const { return NodeComplexHint::NONE; }

				inline input_objmap_named next() const { return input_objmap_named(impl_.next()); }
				inline input_objmap_named &operator++() { impl_ = impl_.next(); return *this; }
				inline input_objmap_named operator++(int) { auto tmp = *this; impl_ = impl_.next(); return tmp; }
				inline bool operator ==(const input_objmap_named &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_objmap_named &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_objmap_named, input_objmap_named> children(const char * key) const;
				inline std::pair<input_objmap_unnamed, input_objmap_unnamed> children() const;

				inline const char * value() const { return impl_.value(); }
				inline const char * key() const { return impl_.key(); }
			};


			class input_objmap_unnamed
			{
				friend class input_objmap_named;
				friend class input_objmap_root;
				ObjectMap_Iterator<char, char> impl_;
				inline const ObjectMap_Iterator<char, char> &checked_() const
				{
					if (impl_.empty())
						throw IConvInvalidIteratorException("input_objmap_unnamed is empty!");
					return impl_;
				}
				input_objmap_unnamed(const ObjectMap_Iterator<char, char> &i) : impl_(i) {}
			public:
				typedef input_objmap_unnamed unnamed_iterator;
				typedef input_objmap_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::UNNAMED;

				input_objmap_unnamed(const ObjectMap<char, char> &n) : impl_(n.children().first) {}
				input_objmap_unnamed() {}
				input_objmap_unnamed(const input_objmap_unnamed &i) : impl_(i.impl_) {}
				//input_xml_unnamed(const input_xml_named &i);

				inline bool empty() const { return impl_.empty(); }
				inline NodeType type() const
				{
					if (empty())
						return NodeType::NONE;
					if (impl_.is_value())
						return NodeType::VALUE;
					if (impl_.is_object())
						return NodeType::COMPLEX;
					return NodeType::NONE;
				}
				inline NodeValueHint value_hint() const
				{
					if (!empty() && impl_.value_hint() == ObjectMapValueHint::ATTR)
						return NodeValueHint::ATTRIBUTE;
					return NodeValueHint::NONE;
				}
				inline NodeComplexHint complex_hint() const { return NodeComplexHint::NONE; }

				inline input_objmap_unnamed next() const { return input_objmap_unnamed(impl_.next()); }
				inline input_objmap_unnamed &operator++() { impl_ = impl_.next(); return *this; }
				inline input_objmap_unnamed operator++(int) { auto tmp = *this; impl_ = impl_.next(); return tmp; }
				inline bool operator ==(const input_objmap_unnamed &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_objmap_unnamed &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_objmap_named, input_objmap_named> children(const char * key) const;
				inline std::pair<input_objmap_unnamed, input_objmap_unnamed> children() const;
				
				inline const char * value() const { return impl_.value(); }
				inline const char * key() const { return impl_.key(); }
			};

			class input_objmap_root
			{
				friend class input_objmap_named;
				const ObjectMap<char, char> * impl_;
				inline const ObjectMap<char, char> * checked_() const
				{
					if (!impl_)
						throw IConvInvalidIteratorException("input_objmap_root is empty!");
					return impl_;
				}
			public:
				typedef input_objmap_unnamed unnamed_iterator;
				typedef input_objmap_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::UNNAMED;

				input_objmap_root(const ObjectMap<char, char> &n) : impl_(&n) {}
				input_objmap_root() : impl_(nullptr) {}
				input_objmap_root(const input_objmap_root &i) : impl_(i.impl_) {}

				inline bool empty() const { return !impl_; }
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

				inline input_objmap_root next() const { return input_objmap_root(); }
				inline input_objmap_root &operator++() { impl_ = nullptr; return *this; }
				inline input_objmap_root operator++(int) { auto tmp = *this; impl_ = nullptr; return tmp; }
				inline bool operator ==(const input_objmap_root &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_objmap_root &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_objmap_named, input_objmap_named> children(const char * key) const;
				inline std::pair<input_objmap_unnamed, input_objmap_unnamed> children() const;

				inline const char * value() const { return nullptr; }
				inline const char * key() const { return nullptr; }
			};
			inline std::pair<input_objmap_named, input_objmap_named> input_objmap_root::children(const char * key) const
			{
				auto n = checked_()->children(key);
				return std::make_pair(input_objmap_named(n.first), input_objmap_named(n.second));
			}
			inline std::pair<input_objmap_unnamed, input_objmap_unnamed> input_objmap_root::children() const
			{
				auto n = checked_()->children();
				return std::make_pair(input_objmap_unnamed(n.first), input_objmap_unnamed(n.second));
			}

			inline std::pair<input_objmap_named, input_objmap_named> input_objmap_unnamed::children(const char * key) const
			{
				auto n = checked_().children(key);
				return std::make_pair(input_objmap_named(n.first), input_objmap_named(n.second));
			}
			inline std::pair<input_objmap_unnamed, input_objmap_unnamed> input_objmap_unnamed::children() const
			{
				auto n = checked_().children();
				return std::make_pair(input_objmap_unnamed(n.first), input_objmap_unnamed(n.second));
			}
			inline std::pair<input_objmap_named, input_objmap_named> input_objmap_named::children(const char * key) const
			{
				auto n = checked_().children(key);
				return std::make_pair(input_objmap_named(n.first), input_objmap_named(n.second));
			}
			inline std::pair<input_objmap_unnamed, input_objmap_unnamed> input_objmap_named::children() const
			{
				auto n = checked_().children();
				return std::make_pair(input_objmap_unnamed(n.first), input_objmap_unnamed(n.second));
			}
		}
	}
}