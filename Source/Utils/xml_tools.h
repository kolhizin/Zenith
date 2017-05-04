#pragma once
#include <pugixml\pugixml.hpp>
#include "object_map.h"
namespace zenith
{
	namespace util
	{
		namespace xml
		{
			class XMLException : public std::exception
			{
			public:
				XMLException() : std::exception("XMLException: unknown cause") {}
				XMLException(const char * p) : std::exception(p) {}
				virtual ~XMLException() {}
			};

			class XMLParseException : public XMLException
			{
				pugi::xml_parse_result res_;
			public:
				XMLParseException() : XMLException("XMLException: parsing error") {}
				XMLParseException(const char * p) : XMLException(p) {}
				XMLParseException(const char * p, const pugi::xml_parse_result &r) : XMLException(p), res_(r) {}
				virtual ~XMLParseException() {}

				const pugi::xml_parse_result &getParseResult() const
				{
					return res_;
				}
			};


			template<class Char>
			inline void xml2objmap(const pugi::xml_node &node, ObjectMap<Char, Char> &obj)
			{
				for (pugi::xml_node n : node.children())
				{
					auto attr_rng = n.attributes();
					auto chld_rng = n.children();
					auto nxt_chld = chld_rng.begin();	

					bool chld_empty = (nxt_chld != chld_rng.end()) && (++nxt_chld == chld_rng.end());
					bool attr_empty = (attr_rng.begin() == attr_rng.end());
					
					bool type_ok = (chld_rng.begin() != chld_rng.end()) && (chld_rng.begin()->type() == pugi::xml_node_type::node_pcdata || chld_rng.begin()->type() == pugi::xml_node_type::node_cdata);

					if(attr_empty && chld_empty && type_ok)
						obj.addValue(n.name(), n.child_value());
					else
						xml2objmap(n, obj.addObject(n.name()));
				}
				for (pugi::xml_attribute a : node.attributes())
					obj.addValue(a.name(), a.value(), ObjectMapValueHint::ATTR);
			}
			template<class Char>
			inline void objmap2xml(const ObjectMap<Char, Char> &obj, pugi::xml_node &node)
			{
				auto iters = obj.getObjects();
				for (auto it = iters.first; it != iters.second; ++it)
					objmap2xml(it->second, node.append_child(it->first.c_str()));

				auto valits = obj.getValues();
				for (auto it = valits.first; it != valits.second; ++it)
				{
					if (it->second.second == ObjectMapValueHint::ATTR)
					{
						auto attr = node.append_attribute(it->first.c_str());
						attr.set_value(it->second.first.c_str());
					}
					else
					{
						auto n = node.append_child(it->first.c_str());
						n.append_child(pugi::xml_node_type::node_pcdata).set_value(it->second.first.c_str());
					}
				}
			}
		}
	}
}