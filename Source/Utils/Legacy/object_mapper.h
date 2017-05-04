#pragma once
#include <map>
#include <string>
#include <exception>

namespace zenith
{
	namespace util
	{

		template<class K, class S>	using object_mapper_map = std::map<K, S>;

		enum class obj_type { NONE, DOUBLE, INT, STRING, STRUCT };

		template<class Key, class Str = std::string, template<class, class> class Cont = object_mapper_map >
		class object_layout
		{
			Cont<Key, obj_type> layout;
			Str name;
		public:
			object_layout(const Str &layout_name) : name(layout_name) {}
			inline bool name_exists(const Key &k) const
			{
				return layout.find(k) != layout.end();
			}
			inline obj_type get_type(const Key &k) const
			{
				auto iter = layout.find(k);
				if (iter == layout.end())
					return obj_type::NONE;
				return iter->second;
			}

			const Str &layout_name() const
			{
				return name;
			}

			inline void assert_name_exists(const Key &k) const
			{
				auto iter = layout.find(k);
				if (iter == layout.end())
					throw std::exception("Assert failed in zenith::util::object_mapper::object_layout::assert_name_exists()");
			}
			inline void assert_no_name(const Key &k) const
			{
				auto iter = layout.find(k);
				if (iter != layout.end())
					throw std::exception("Assert failed in zenith::util::object_mapper::object_layout::assert_no_name()");
			}
			inline void assert_type(const Key &k, obj_type type) const
			{
				auto iter = layout.find(k);
				if (iter == layout.end())
					throw std::exception("Name not found in zenith::util::object_mapper::object_layout::assert_type()");
				if (iter->second != type)
					throw std::exception("Type mismatch in zenith::util::object_mapper::object_layout::assert_type()");
			}

			inline void add_name(const Key &k, obj_type type)
			{
				if (type == obj_type::NONE)
					throw std::exception("Can not add NONE-type in zenith::util::object_mapper::object_layout::add_name()");
				assert_no_name(k);
				layout[k] = type;
			}

			inline void merge(const object_layout &rhs)
			{
				for (const auto &it : rhs.layout)
					assert_no_name(it.first);

				layout.insert(rhs.layout.begin(), rhs.layout.end());
			}
			inline bool merge_safe(const object_layout &rhs)
			{
				for (const auto &it : rhs.layout)
					if (name_exists(it.first))
						return false;

				layout.insert(rhs.layout.begin(), rhs.layout.end());
				return true;
			}
			inline bool is_subset_of(const object_layout &rhs) const
			{
				for (const auto &it : layout)
					if (rhs.get_type(it.first) != it.second)
						return false;
				return true;
			}
		};


		template<class Key, class Str=std::string, template<class,class> class Cont = object_mapper_map >
		class object_mapper
		{
			Cont<Key, object_mapper<Key, Str, Cont> > vals_complex;
			Cont<Key, Str> vals_string;
			Cont<Key, int64_t> vals_int;
			Cont<Key, double> vals_double;
		
			object_layout<Key, Str, Cont> layout;
						
		public:
			object_mapper(const Str &layout_name) : layout(layout_name) {}
			void add_value(const Key &name, int64_t v)
			{
				layout.add_name(name, obj_type::INT);
				vals_int[name] = v;
			}
			void add_value(const Key &name, double v)
			{
				layout.add_name(name, obj_type::DOUBLE);
				vals_double[name] = v;
			}
			void add_value(const Key &name, const Str &v)
			{
				layout.add_name(name, obj_type::STRING);
				vals_string[name] = v;
			}
			void add_value(const Key &name, const object_mapper<Key, Str, Cont> &v)
			{
				layout.add_name(name, obj_type::STRUCT);
				vals_complex[name] = v;
			}

			const Str &layout_name() const
			{
				return layout.layout_name();
			}

			const object_layout<Key, Str, Cont> &get_layout() const
			{
				return layout;
			}

			obj_type check_name(const Key &name) const
			{
				return layout.get_type(name);
			}
			int64_t get_int(const Key &name) const
			{
				layout.assert_type(name, obj_type::INT);
				return vals_int.at(name);
			}
			double get_double(const Key &name) const
			{
				layout.assert_type(name, obj_type::DOUBLE);
				return vals_double.at(name);
			}
			const Str &get_string(const Key &name) const
			{
				layout.assert_type(name, obj_type::STRING);
				return vals_string.at(name);
			}
			const object_mapper<Key, Str, Cont> &get_struct(const Key &name) const
			{
				layout.assert_type(name, obj_type::STRUCT);
				return vals_complex.at(name);
			}

			void merge(const object_mapper<Key, Str, Cont> &rhs)
			{
				layout.merge(rhs.layout);

				vals_string.insert(rhs.vals_string.begin(), rhs.vals_string.end());
				vals_int.insert(rhs.vals_int.begin(), rhs.vals_int.end());
				vals_double.insert(rhs.vals_double.begin(), rhs.vals_double.end());
				vals_complex.insert(rhs.vals_complex.begin(), rhs.vals_complex.end());
			}
		};
	}
}