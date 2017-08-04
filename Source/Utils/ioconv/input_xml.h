#pragma once
#include "ioconv_base.h"
#include <pugixml\pugixml.hpp>

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			inline bool _is_pugixml_value_node(const pugi::xml_node &n)
			{
				if (n.attributes_begin() != n.attributes_end())
					return false;

				auto it0 = n.begin();
				auto it = it0; it++;
				if (it != n.end())
					return false;
								
				return (it0->type() == pugi::xml_node_type::node_cdata || it0->type() == pugi::xml_node_type::node_pcdata);
			}
			inline bool _is_pugixml_complex_node(const pugi::xml_node &n)
			{
				return !_is_pugixml_value_node(n);
			}

			class input_xml_wrap_named_
			{
				pugi::xml_named_node_iterator node_, intEnd_;
				pugi::xml_attribute attr_;
			public:
				input_xml_wrap_named_(const input_xml_wrap_named_ &iw) : node_(iw.node_), attr_(iw.attr_), intEnd_(iw.intEnd_){}
				input_xml_wrap_named_(const pugi::xml_named_node_iterator &n, const pugi::xml_named_node_iterator &end) : node_(n), intEnd_(end) {}
				input_xml_wrap_named_(const pugi::xml_attribute &a) : attr_(a) {}
				inline const char * key() const
				{
					if (attr_.empty())
						return node_->name();
					else
						return attr_.name();
				}
				inline const char * value() const
				{
					if (attr_.empty())
						return node_->child_value();
					else
						return attr_.value();
				}
				inline const pugi::xml_node &node() const
				{
					if (node_ != intEnd_)
						return *node_;
					throw IConvInvalidIteratorException("input_xml_wrap_named_::node(): invalid iterator!");
				}
				inline const pugi::xml_attribute &attribute() const
				{
					if (!attr_.empty())
						return attr_;
					throw IConvInvalidIteratorException("input_xml_wrap_named_::attribute(): invalid iterator!");
				}
				inline bool is_attribute() const { return !attr_.empty(); }
				inline bool is_node() const { return node_ != intEnd_; }
				inline bool is_node_value() const { return is_node() && _is_pugixml_value_node(*node_); }
				inline bool is_node_complex() const {return is_node() && _is_pugixml_complex_node(*node_);}
				inline static input_xml_wrap_named_ end()
				{
					pugi::xml_named_node_iterator it;
					input_xml_wrap_named_ res(it,it);
					return res;
				}
				inline input_xml_wrap_named_ next() const
				{
					if (!attr_.empty())
						return end();
					auto next = node_; next++;
					if (next == intEnd_)
						return input_xml_wrap_named_(node_->parent().attribute(node_->name()));
					else
						return input_xml_wrap_named_(next, intEnd_);
				}
				inline bool operator ==(const input_xml_wrap_named_ &n) const
				{
					return (node_ == n.node_) && (attr_ == n.attr_);
				}
			};
			class input_xml_wrap_unnamed_
			{
				pugi::xml_node_iterator node_, intEnd_;
				pugi::xml_attribute attr_;
			public:
				input_xml_wrap_unnamed_(const input_xml_wrap_unnamed_ &iw) : node_(iw.node_), attr_(iw.attr_), intEnd_(iw.intEnd_) {}
				input_xml_wrap_unnamed_(const pugi::xml_node_iterator &n, const pugi::xml_node_iterator &end) : node_(n), intEnd_(end){	}
				input_xml_wrap_unnamed_(const pugi::xml_attribute &a) : attr_(a) {}
				inline const char * key() const
				{
					if (attr_.empty())
						return node_->name();
					else
						return attr_.name();
				}
				inline const char * value() const
				{
					if (attr_.empty())
						return node_->child_value();
					else
						return attr_.value();
				}
				inline const pugi::xml_node &node() const
				{
					if (node_ != intEnd_)
						return *node_;
					throw IConvInvalidIteratorException("input_xml_wrap_unnamed_::node(): invalid iterator!");
				}
				inline const pugi::xml_attribute &attribute() const
				{
					if (!attr_.empty())
						return attr_;
					throw IConvInvalidIteratorException("input_xml_wrap_unnamed_::attribute(): invalid iterator!");
				}
				inline bool is_attribute() const { return !attr_.empty(); }
				inline bool is_node() const { return node_ != intEnd_; }
				inline bool is_node_value() const { return is_node() && _is_pugixml_value_node(*node_); }
				inline bool is_node_complex() const { return is_node() && _is_pugixml_complex_node(*node_); }

				inline static input_xml_wrap_unnamed_ end()
				{
					pugi::xml_node_iterator it;
					input_xml_wrap_unnamed_ res(it,it);
					return res;
				}
				inline input_xml_wrap_unnamed_ next() const
				{
					if (!attr_.empty())
						return input_xml_wrap_unnamed_(attr_.next_attribute());
					auto next = node_; next++;
					if (next == intEnd_)
						return input_xml_wrap_unnamed_(node_->parent().first_attribute());
					else
						return input_xml_wrap_unnamed_(next, intEnd_);
				}
				inline bool operator ==(const input_xml_wrap_unnamed_ &n) const
				{
					return (node_ == n.node_) && (attr_ == n.attr_);
				}
			};

			class input_xml_unnamed;
			class input_xml_root;

			class input_xml_named
			{
				friend class input_xml_unnamed;
				friend class input_xml_root;
				input_xml_wrap_named_ impl_;
				inline const input_xml_wrap_named_ &checked_() const
				{
					if (impl_ == input_xml_wrap_named_::end())
						throw IConvInvalidIteratorException("input_xml_named is empty!");
					return impl_;
				}
				input_xml_named(const input_xml_wrap_named_ &i) : impl_(i) {}
			public:
				typedef input_xml_unnamed unnamed_iterator;
				typedef input_xml_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::NAMED;
				static const InternalType internal_type = InternalType::STRING;

				input_xml_named(const pugi::xml_node &n, const char * key)  : impl_(input_xml_wrap_named_::end())
				{
					auto rng = n.children(key);
					impl_ = input_xml_wrap_named_(rng.begin(), rng.end());
				}
				input_xml_named(const pugi::xml_attribute &a) : impl_(a) {}
				input_xml_named() : impl_(pugi::xml_named_node_iterator(), pugi::xml_named_node_iterator()) {}
				input_xml_named(const input_xml_named &i) : impl_(i.impl_) {}
				//input_xml_named(const input_xml_unnamed &i);

				inline bool empty() const { return impl_ == input_xml_wrap_named_::end(); }
				inline NodeType type() const
				{
					if (empty())
						return NodeType::NONE;
					if (impl_.is_attribute())
						return NodeType::VALUE;
					if (impl_.is_node_value())
						return NodeType::VALUE;
					if (impl_.is_node_complex())
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

				inline input_xml_named next() const { return input_xml_named(impl_.next());	}
				inline input_xml_named &operator++() { impl_ = impl_.next(); return *this; }
				inline input_xml_named operator++(int) { auto tmp = *this; impl_ = impl_.next(); return tmp; }
				inline bool operator ==(const input_xml_named &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_xml_named &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_xml_named, input_xml_named> children(const char * key) const;
				inline std::pair<input_xml_unnamed, input_xml_unnamed> children() const;

				inline const char * value() const {	return impl_.value(); }
				inline const char * key() const { return impl_.key(); }
			};


			class input_xml_unnamed
			{
				friend class input_xml_named;
				friend class input_xml_root;
				input_xml_wrap_unnamed_ impl_;
				inline const input_xml_wrap_unnamed_ &checked_() const
				{
					if (impl_ == input_xml_wrap_unnamed_::end())
						throw IConvInvalidIteratorException("input_xml_unnamed is empty!");
					return impl_;
				}
				input_xml_unnamed(const input_xml_wrap_unnamed_ &i) : impl_(i) {}
			public:
				typedef input_xml_unnamed unnamed_iterator;
				typedef input_xml_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::UNNAMED;
				static const InternalType internal_type = InternalType::STRING;

				input_xml_unnamed(const pugi::xml_node &n) : impl_(n.begin(), n.end()) {}
				input_xml_unnamed(const pugi::xml_attribute &a) : impl_(a) {}
				input_xml_unnamed() : impl_(pugi::xml_node_iterator(), pugi::xml_node_iterator()) {}
				input_xml_unnamed(const input_xml_unnamed &i) : impl_(i.impl_) {}
				//input_xml_unnamed(const input_xml_named &i);

				inline bool empty() const { return impl_ == input_xml_wrap_unnamed_::end(); }
				inline NodeType type() const
				{
					if (empty())
						return NodeType::NONE;
					if (impl_.is_attribute())
						return NodeType::VALUE;
					if (impl_.is_node_value())
						return NodeType::VALUE;
					if (impl_.is_node_complex())
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

				inline input_xml_unnamed next() const { return input_xml_unnamed(impl_.next()); }
				inline input_xml_unnamed &operator++() { impl_ = impl_.next(); return *this; }
				inline input_xml_unnamed operator++(int) { auto tmp = *this; impl_ = impl_.next(); return tmp; }
				inline bool operator ==(const input_xml_unnamed &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_xml_unnamed &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_xml_named, input_xml_named> children(const char * key) const;
				inline std::pair<input_xml_unnamed, input_xml_unnamed> children() const;


				inline const char * value() const { return impl_.value(); }
				inline const char * key() const { return impl_.key(); }
			};

			class input_xml_root
			{
				pugi::xml_node impl_;
			public:
				typedef input_xml_unnamed unnamed_iterator;
				typedef input_xml_named named_iterator;
				static const IteratorType iterator_type = IteratorType::INPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::UNNAMED;
				static const InternalType internal_type = InternalType::STRING;

				input_xml_root(const pugi::xml_node &n) : impl_(n) {}
				input_xml_root() {}
				//input_xml_unnamed(const input_xml_named &i);

				inline bool empty() const { return impl_.empty(); }
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

				inline input_xml_root next() const { return input_xml_root(); }
				inline input_xml_root &operator++() { pugi::xml_node n; impl_ = n; return *this; }
				inline input_xml_root operator++(int) { pugi::xml_node n; auto tmp = *this; impl_ = n; return tmp; }
				inline bool operator ==(const input_xml_root &i) const { return impl_ == i.impl_; }
				inline bool operator !=(const input_xml_root &i) const { return !(impl_ == i.impl_); }

				inline std::pair<input_xml_named, input_xml_named> children(const char * key) const;
				inline std::pair<input_xml_unnamed, input_xml_unnamed> children() const;


				inline const char * value() const { return impl_.value(); }
				inline const char * key() const { return impl_.name(); }
			};

			inline std::pair<input_xml_named, input_xml_named> input_xml_root::children(const char * key) const
			{
				auto x = impl_.children(key);
				if (x.begin() == x.end())
				{
					const auto &a = impl_.attribute(key);
					input_xml_wrap_named_ begin(a);
					return std::make_pair(input_xml_named(begin), input_xml_named(input_xml_wrap_named_::end()));
				}
				input_xml_wrap_named_ begin(x.begin(), x.end());
				return std::make_pair(input_xml_named(begin), input_xml_named(input_xml_wrap_named_::end()));
			}
			inline std::pair<input_xml_unnamed, input_xml_unnamed> input_xml_root::children() const
			{
				auto x = impl_.children();
				if (x.begin() == x.end())
				{
					const auto &a = impl_.first_attribute();
					input_xml_wrap_unnamed_ begin(a);
					return std::make_pair(input_xml_unnamed(begin), input_xml_unnamed(input_xml_wrap_unnamed_::end()));
				}
				input_xml_wrap_unnamed_ begin(x.begin(), x.end());
				return std::make_pair(input_xml_unnamed(begin), input_xml_unnamed(input_xml_wrap_unnamed_::end()));
			}

			inline std::pair<input_xml_named, input_xml_named> input_xml_unnamed::children(const char * key) const
			{
				const auto &n = checked_().node();
				auto x = n.children(key);
				if (x.begin() == x.end())
				{
					const auto &a = n.attribute(key);
					input_xml_wrap_named_ begin(a);
					return std::make_pair(input_xml_named(begin), input_xml_named(input_xml_wrap_named_::end()));
				}
				input_xml_wrap_named_ begin(x.begin(), x.end());
				return std::make_pair(input_xml_named(begin), input_xml_named(input_xml_wrap_named_::end()));
			}
			inline std::pair<input_xml_unnamed, input_xml_unnamed> input_xml_unnamed::children() const
			{
				const auto &n = checked_().node();
				auto x = n.children();
				if (x.begin() == x.end())
				{
					const auto &a = n.first_attribute();
					input_xml_wrap_unnamed_ begin(a);
					return std::make_pair(input_xml_unnamed(begin), input_xml_unnamed(input_xml_wrap_unnamed_::end()));
				}
				input_xml_wrap_unnamed_ begin(x.begin(), x.end());
				return std::make_pair(input_xml_unnamed(begin), input_xml_unnamed(input_xml_wrap_unnamed_::end()));
			}

			inline std::pair<input_xml_named, input_xml_named> input_xml_named::children(const char * key) const
			{
				const auto &n = checked_().node();
				auto x = n.children(key);
				if (x.begin() == x.end())
				{
					const auto &a = n.attribute(key);
					input_xml_wrap_named_ begin(a);
					return std::make_pair(input_xml_named(begin), input_xml_named(input_xml_wrap_named_::end()));
				}
				input_xml_wrap_named_ begin(x.begin(), x.end());
				return std::make_pair(input_xml_named(begin), input_xml_named(input_xml_wrap_named_::end()));
			}
			inline std::pair<input_xml_unnamed, input_xml_unnamed> input_xml_named::children() const
			{
				const auto &n = checked_().node();
				auto x = n.children();
				if (x.begin() == x.end())
				{
					const auto &a = n.first_attribute();
					input_xml_wrap_unnamed_ begin(a);
					return std::make_pair(input_xml_unnamed(begin), input_xml_unnamed(input_xml_wrap_unnamed_::end()));
				}
				input_xml_wrap_unnamed_ begin(x.begin(), x.end());
				return std::make_pair(input_xml_unnamed(begin), input_xml_unnamed(input_xml_wrap_unnamed_::end()));
			}
		}
	}
}