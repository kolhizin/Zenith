#pragma once
#include "object_mapper.h"
#include <glm\glm.hpp>

namespace zenith
{
	namespace util
	{
		template<class Obj, class Key, class Str, template<class, class> class Cont >
		class object_layout_maker;

		template<class Obj, class Key, class Str, template<class, class> class Cont >
		const object_layout<Key, Str, Cont> &get_object_layout(const Obj &obj)
		{
			return object_layout_maker<Obj, Key, Str, Cont>::layout();
		}

#define ZENITH_UTIL_OBJECT_LAYOUT_START(obj)\
		template<class Key, class Str, template<class, class> class Cont >\
		class object_layout_maker<obj, Key, Str, Cont>\
		{\
			static object_layout<Key, Str, Cont> get_layout()\
			{\
				object_layout<Key, Str, Cont> res(#obj)

#define ZENITH_UTIL_OBJECT_LAYOUT_ELEM(name, type)res.add_name(#name, type)

#define ZENITH_UTIL_OBJECT_LAYOUT_END\
				return res;\
			}\
		public:\
			static const object_layout<Key, Str, Cont> &layout()\
			{\
				static object_layout<Key, Str, Cont> layout = get_layout();\
				return layout;\
			}\
		}

#define ZENITH_UTIL_OBJECT_OBJ2MAP_START(object)\
		template<class Key, class Str, template<class, class> class Cont>\
		object_mapper<Key, Str, Cont> object2map(const object &v)\
		{\
			object_mapper<Key, Str, Cont> res(#object)

#define ZENITH_UTIL_OBJECT_OBJ2MAP_END\
		return res;\
		}

#define ZENITH_UTIL_OBJECT_OBJ2MAP_PLAIN(x)res.add_value(#x, v.x)


#define ZENITH_UTIL_OBJECT_MAP2OBJ_START(object)\
		template<class Key, class Str , template<class, class> class Cont >\
		void map2object(const object_mapper<Key, Str, Cont> &obj, object &v)\
		{\
			const auto &layout = get_object_layout<std::remove_reference<decltype(v)>::type, Key, Str, Cont>(v);\
			if (layout.layout_name() != obj.layout_name())\
				throw std::exception("Layout names mismatch in map2object!");\
			if (!layout.is_subset_of(obj.get_layout()))\
				throw std::exception("Layout is not subset of supplied object_mapper in map2object!")

#define ZENITH_UTIL_OBJECT_MAP2OBJ_END }


#define ZENITH_UTIL_OBJECT_MAP2OBJ_DOUBLE(x)v.x = obj.get_double(#x)
#define ZENITH_UTIL_OBJECT_MAP2OBJ_INT(x)v.x = obj.get_int(#x)
#define ZENITH_UTIL_OBJECT_MAP2OBJ_STRING(x)v.x = obj.get_string(#x)



		ZENITH_UTIL_OBJECT_LAYOUT_START(glm::vec4);
			ZENITH_UTIL_OBJECT_LAYOUT_ELEM(x, obj_type::DOUBLE);
			ZENITH_UTIL_OBJECT_LAYOUT_ELEM(y, obj_type::DOUBLE);
			ZENITH_UTIL_OBJECT_LAYOUT_ELEM(z, obj_type::DOUBLE);
			ZENITH_UTIL_OBJECT_LAYOUT_ELEM(w, obj_type::DOUBLE);
		ZENITH_UTIL_OBJECT_LAYOUT_END;

		ZENITH_UTIL_OBJECT_OBJ2MAP_START(glm::vec4);
			ZENITH_UTIL_OBJECT_OBJ2MAP_PLAIN(x);
			ZENITH_UTIL_OBJECT_OBJ2MAP_PLAIN(y);
			ZENITH_UTIL_OBJECT_OBJ2MAP_PLAIN(z);
			ZENITH_UTIL_OBJECT_OBJ2MAP_PLAIN(w);
		ZENITH_UTIL_OBJECT_OBJ2MAP_END;

		ZENITH_UTIL_OBJECT_MAP2OBJ_START(glm::vec4);
			ZENITH_UTIL_OBJECT_MAP2OBJ_DOUBLE(x);
			ZENITH_UTIL_OBJECT_MAP2OBJ_DOUBLE(y);
			ZENITH_UTIL_OBJECT_MAP2OBJ_DOUBLE(z);
			ZENITH_UTIL_OBJECT_MAP2OBJ_DOUBLE(w);
		ZENITH_UTIL_OBJECT_MAP2OBJ_END;
		
	}
}
