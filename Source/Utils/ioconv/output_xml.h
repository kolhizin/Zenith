#pragma once
#include "ioconv_base.h"
#include <pugixml\pugixml.hpp>

namespace zenith
{
	namespace util
	{
		namespace ioconv
		{
			class output_xml_value
			{
				pugi::xml_node node_;
				pugi::xml_attribute attr_;
				bool isNode_, isAttr_;
			public:
				output_xml_value(const pugi::xml_node &n) : node_(n), isAttr_(false), isNode_(true) {}
				output_xml_value(const pugi::xml_attribute &a) : attr_(a), isAttr_(true), isNode_(false) {}
				inline void set_value(const char * val)
				{
					if (isNode_)
						node_.set_value(val);
					if (isAttr_)
						attr_.set_value(val);
				}
			};
			class output_xml
			{
				pugi::xml_node node_;
				inline pugi::xml_node &checked_()
				{
					if (node_.root().empty())
						throw OConvInvalidIteratorException("input_xml_unnamed is empty!");
					return node_;
				}
				//output_xml(pugi::xml_node &i) : node_(i) {}
			public:
				typedef output_xml unnamed_iterator;
				typedef output_xml named_iterator;
				static const IteratorType iterator_type = IteratorType::OUTPUT;
				static const IteratorCategory iterator_cat = IteratorCategory::NONE;

				output_xml(){}
				output_xml(pugi::xml_node &node) : node_(node) {}
				output_xml(const output_xml &i) : node_(i.node_) {}

				inline pugi::xml_node &impl_checked() { return checked_(); }
				inline pugi::xml_node &impl_raw() { return node_; }

				inline output_xml append_complex(const char * key, NodeComplexHint hint = NodeComplexHint::NONE)
				{
					return output_xml(checked_().append_child(key));
				}
				inline output_xml_value append_value(const char * key, const char * value, NodeValueHint hint = NodeValueHint::NONE)
				{
					if (hint == NodeValueHint::ATTRIBUTE)
					{
						pugi::xml_attribute attr = checked_().append_attribute(key);
						attr.set_value(value);
						return output_xml_value(attr);
					}
					else
					{
						auto n = checked_().append_child(key);
						auto nn = n.append_child(pugi::xml_node_type::node_pcdata);
						nn.set_value(value);
						return output_xml_value(nn);
					}
				}
				inline output_xml_value append_value(const char * key, NodeValueHint hint = NodeValueHint::NONE)
				{
					if (hint == NodeValueHint::ATTRIBUTE)
					{
						pugi::xml_attribute attr = checked_().append_attribute(key);
						return output_xml_value(attr);
					}
					else
					{
						auto n = checked_().append_child(key);
						auto nn = n.append_child(pugi::xml_node_type::node_pcdata);
						return output_xml_value(nn);
					}
				}
			};

		}
	}
}