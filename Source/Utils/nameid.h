#pragma once

#include <limits>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

namespace zenith
{
	namespace util
	{
		template<class IdT>
		class nameid_impl_map_
		{
			struct nameid_mapping
			{
				std::map<std::string, IdT> str2id;
				std::vector<std::string> id2str;
			};

			static bool init_mapping_(nameid_mapping &mapping)
			{
				mapping.id2str.reserve(1000);
				mapping.str2id["NULL"] = IdT(0);
				mapping.id2str.emplace_back("NULL");
				return true;
			}
			static nameid_mapping &get_mapping_()
			{
				static nameid_mapping nIds;
				static bool init = init_mapping_(nIds);
				return nIds;
			}
			static IdT getId_(const char * ptr)
			{
				nameid_mapping &nm = get_mapping_();
				auto iter = nm.str2id.find(ptr);
				if (iter == nm.str2id.end())
				{
					IdT id = static_cast<IdT>(nm.id2str.size());
					nm.id2str.emplace_back(ptr);
					nm.str2id[ptr] = id;
					return id;
				}
				else
					return iter->second;
			}
			static const char * getStr_(IdT id)
			{
				nameid_mapping &nm = get_mapping_();
				return nm.id2str.at(id).c_str();
			}

			IdT id_;
		public:
			inline nameid_impl_map_() : id_(0) {}
			inline nameid_impl_map_(const char * ptr) : id_(getId_(ptr)) {}
			inline nameid_impl_map_(const nameid_impl_map_<IdT> &nid) : id_(nid.id_) {}
			template<class A>
			inline nameid_impl_map_(const std::basic_string<char, std::char_traits<char>, A> &str) : id_(getId_(str.c_str())) {}

			inline nameid_impl_map_ &operator=(const char * ptr)
			{
				id_ = getId_(ptr);
				return *this;
			}
			inline nameid_impl_map_ &operator=(const nameid_impl_map_<IdT> &nid)
			{
				id_ = nid.id_;
				return *this;
			}
			template<class A>
			inline nameid_impl_map_ &operator=(const std::basic_string<char, std::char_traits<char>, A> &str)
			{
				id_ = getId_(str.c_str());
				return *this;
			}

			inline int compare(const nameid_impl_map_<IdT> &nid) const
			{
				if (id_ == nid.id_)
					return 0;
				return (id_ < nid.id_ ? -1 : +1);
			}
			
			inline const char * c_str() const
			{
				return getStr_(id_);
			}

			inline explicit operator char*() const
			{
				return getStr_(id_);
			}
			inline explicit operator IdT() const
			{
				return id_;
			}			
		};

		template<class IdT>
		inline bool operator <(const nameid_impl_map_<IdT> &lhs, const nameid_impl_map_<IdT> &rhs) { return lhs.compare(rhs) < 0; }
		template<class IdT>
		inline bool operator <=(const nameid_impl_map_<IdT> &lhs, const nameid_impl_map_<IdT> &rhs) { return lhs.compare(rhs) <= 0; }
		template<class IdT>
		inline bool operator >(const nameid_impl_map_<IdT> &lhs, const nameid_impl_map_<IdT> &rhs) { return lhs.compare(rhs) > 0; }
		template<class IdT>
		inline bool operator >=(const nameid_impl_map_<IdT> &lhs, const nameid_impl_map_<IdT> &rhs) { return lhs.compare(rhs) >= 0; }
		template<class IdT>
		inline bool operator ==(const nameid_impl_map_<IdT> &lhs, const nameid_impl_map_<IdT> &rhs) { return lhs.compare(rhs) == 0; }
		template<class IdT>
		inline bool operator !=(const nameid_impl_map_<IdT> &lhs, const nameid_impl_map_<IdT> &rhs) { return lhs.compare(rhs) != 0; }

		template<class IdT, class A>
		inline bool operator ==(const nameid_impl_map_<IdT> &lhs, const std::basic_string<char, std::char_traits<char>, A> &rhs) { return rhs.compare(lhs.c_str()) == 0; }
		template<class IdT, class A>
		inline bool operator !=(const nameid_impl_map_<IdT> &lhs, const std::basic_string<char, std::char_traits<char>, A> &rhs) { return rhs.compare(lhs.c_str()) != 0; }

		template<class IdT, class A>
		inline bool operator ==(const std::basic_string<char, std::char_traits<char>, A> &rhs, const nameid_impl_map_<IdT> &lhs) { return rhs.compare(lhs.c_str()) == 0; }
		template<class IdT, class A>
		inline bool operator !=(const std::basic_string<char, std::char_traits<char>, A> &rhs, const nameid_impl_map_<IdT> &lhs) { return rhs.compare(lhs.c_str()) != 0; }

		

		template<class NameidClass>
		class nameid_densemap_impl
		{
			std::vector<NameidClass> arr_;
			inline typename std::vector<NameidClass>::const_iterator findI_(const NameidClass &nm) const
			{
				return std::lower_bound(arr_.begin(), arr_.end(), nm);
			}
			inline typename std::vector<NameidClass>::const_iterator find_(const NameidClass &nm) const
			{
				auto it = findI_(nm);
				if (it == arr_.end())
					return it;
				if (*it == nm)
					return it;
				return arr_.end();
			}
			
		public:
			using const_iterator = typename std::vector<NameidClass>::const_iterator;

			void insert(const NameidClass &nm)
			{
				auto it = findI_(nm);
				if (it != arr_.end() && *it == nm)
					throw std::logic_error("nameid_map::insert(): Name already exists in <nameid_densemap_impl>!");
				arr_.insert(it, nm);
			}
			void remove(const NameidClass &nm)
			{
				auto it = find_(nm);
				if (it == arr_.end())
					throw std::logic_error("nameid_map::remove(): Name does not exist in <nameid_densemap_impl>!");
				arr_.erase(it);
			}
			size_t size() const
			{
				return arr_.size();
			}
			bool empty() const
			{
				return arr_.empty();
			}
			bool exist(const NameidClass &nm) const
			{
				return (find_(nm) != arr_.end());
			}
			size_t name2ind(const NameidClass &nm) const
			{
				auto it = find_(nm);
				if (it == arr_.end())
					throw std::out_of_range("nameid_map::name2ind(): Name does not exist in <nameid_densemap_impl>!");
				return std::distance(arr_.begin(), it);
			}
			const NameidClass &ind2name(size_t i) const
			{
				if(i >= arr_.size())
					throw std::out_of_range("nameid_map::ind2name(): Index is out of range in <nameid_densemap_impl>!");
			}
			void clear()
			{
				arr_.clear();
			}

			inline const_iterator begin() const
			{
				return arr_.cbegin();				
			}
			inline const_iterator end() const
			{
				return arr_.cend();
			}
		};
		template<class NameidClass, class Value>
		class nameid_map_impl
		{
			std::vector<Value> vals_;
			nameid_densemap_impl<NameidClass> darr_;
		public:
			void insert(const NameidClass &k, const Value &v)
			{
				darr_.insert(k);
				auto i = darr_.name2ind(k);
				vals_.insert(vals_.begin() + i, v);
			}
			void insert(const NameidClass &k, Value &&v)
			{
				darr_.insert(k);
				auto i = darr_.name2ind(k);
				vals_.insert(vals_.begin() + i, std::move(v));
			}
			void remove(const NameidClass &k)
			{
				auto ind = darr_.name2ind(k);
				darr_.remove(k);
				vals_.erase(vals_.begin() + ind);
			}
			size_t size() const
			{
				return darr_.size();
			}
			bool empty() const
			{
				return darr_.empty();
			}
			bool exist(const NameidClass &nm) const
			{
				return darr_.exist(nm);
			}
			Value &at(const NameidClass &k)
			{
				auto i = darr_.name2ind(k);
				return vals_[i];
			}
			const Value &at(const NameidClass &k) const
			{
				auto i = darr_.name2ind(k);
				return vals_[i];
			}
			void clear()
			{
				darr_.clear();
				vals_.clear();
			}
			const nameid_densemap_impl<NameidClass> &names() const
			{
				return darr_;
			}
		};

		using nameid = nameid_impl_map_<uint32_t>;
		using nameid_seq = nameid_densemap_impl<nameid>;
		template<class T>
		using nameid_map = nameid_map_impl<nameid, T>;
	}
}
