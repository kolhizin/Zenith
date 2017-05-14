#pragma once


#include "alloc_config.h"
#include "log_config.h"
#include "str_cast.h"
#include <string>
#include <map>


namespace zenith
{
	namespace util
	{
		enum class ObjectMapValueHint { UNDEF, VALUE, ATTR };
		enum class ObjectMapPresence { ZERO_PLUS, ONE, ONE_OR_ZERO, ONE_PLUS};

		class ObjectMapException : public LoggedException
		{
		public:
			ObjectMapException() : LoggedException() {}
			ObjectMapException(const char * p, LogType type = LogType::ERROR) : LoggedException(p, type)
			{
			}
			ObjectMapException(const std::string &str, LogType type = LogType::ERROR) : LoggedException(str, type)
			{
			}
			virtual ~ObjectMapException() {}
		};

		template<class CharKey = char, class CharValue = char>
		class ObjectMap
		{
			using ObjectMapLocalPool = zstdLocalPool3<zstdAllocatorMainStatic,
				8, 128, /*128 4-byte chunks*/
				64, 512, /*128 32-byte chunks*/
				256, 128>; /*128 128-byte chunks*/
		public:
			using ObjectMapType = ObjectMap<CharKey, CharValue>;
			using ObjectMapStrKey = std::basic_string<CharKey, std::char_traits<CharKey>, typename ObjectMapLocalPool::STLAllocatorType<CharKey>>;
			using ObjectMapStrValue = std::basic_string<CharValue, std::char_traits<CharValue>, typename ObjectMapLocalPool::STLAllocatorType<CharValue>>;

			
		private:
			
			using ObjectMapKey = ObjectMapStrKey;
			using ObjectMapValue = std::pair<ObjectMapStrValue, ObjectMapValueHint>;
			using ObjectMapPooledMap = std::multimap<ObjectMapKey, ObjectMapValue, std::less<ObjectMapKey>, typename ObjectMapLocalPool::STLAllocatorType<std::pair<ObjectMapKey, ObjectMapValue>>>;
			using ObjectMapObjMap = std::multimap<ObjectMapKey, ObjectMapType>;

			ObjectMapLocalPool localPool_;

			ObjectMapPooledMap data_;
			std::multimap<ObjectMapKey, ObjectMapType> objs_;

			mutable ObjectMapStrKey pooledKey_;
			mutable ObjectMapStrValue pooledValue_;
		public:
			ObjectMap() : localPool_(), data_(localPool_.stl_allocator()), pooledKey_(localPool_.stl_allocator()), pooledValue_(localPool_.stl_allocator())
			{
			}
			ObjectMap(const ObjectMap &rhs) : localPool_(), data_(localPool_.stl_allocator()), pooledKey_(localPool_.stl_allocator()), pooledValue_(localPool_.stl_allocator())
			{
				data_.insert(rhs.data_.cbegin(), rhs.data_.cend());
			}
			ObjectMap &operator =(const ObjectMap &rhs)
			{
				data_.clear();
				data_.insert(rhs.data_.cbegin(), rhs.data_.cend());
			}

			inline void addValue(const CharKey * key, const CharValue * val, ObjectMapValueHint hint = ObjectMapValueHint::UNDEF)
			{
				pooledKey_ = key;
				pooledValue_ = val;
				data_.insert(std::make_pair(pooledKey_, std::make_pair(pooledValue_, hint)));
			}

			inline ObjectMapType &addObject(const CharKey * key)
			{
				pooledKey_ = key;
				auto it = objs_.insert(std::make_pair(pooledKey_, ObjectMap()));
				return it->second;
			}
			inline ObjectMapType &addObject(const CharKey * key, const ObjectMapType &obj)
			{
				pooledKey_ = key;
				auto it = objs_.insert(std::make_pair(pooledKey_, obj));
				return it->second;
			}

			inline size_t getNumValues(const CharKey * key) const
			{
				pooledKey_ = key;
				return data_.count(pooledKey_);
			}
			inline size_t getNumValues() const
			{
				return data_.size();
			}

			inline size_t getNumObjects(const CharKey * key) const
			{
				pooledKey_ = key;
				return objs_.count(pooledKey_);
			}
			inline size_t getNumObjects() const
			{
				return objs_.size();
			}


			inline std::pair<typename ObjectMapPooledMap::const_iterator, typename ObjectMapPooledMap::const_iterator> getValues(const CharKey * key) const
			{
				pooledKey_ = key;
				return data_.equal_range(pooledKey_);
			}
			inline std::pair<typename ObjectMapPooledMap::const_iterator, typename ObjectMapPooledMap::const_iterator> getValues(const CharKey * key, ObjectMapPresence flg) const
			{
				pooledKey_ = key;
				auto rng = data_.equal_range(pooledKey_);
				if (rng.first == rng.second)
				{
					if (flg == ObjectMapPresence::ONE || flg == ObjectMapPresence::ONE_PLUS)
						throw ObjectMapException(std::string("ObjectMapException: expected value one or more value, but none found - <") + key + ">");
					if (flg == ObjectMapPresence::ONE_OR_ZERO)
						return rng;
				}
				auto fst = rng.first;
				fst++;
				if (fst != rng.second)
					if (flg == ObjectMapPresence::ONE || flg == ObjectMapPresence::ONE_OR_ZERO)
						throw ObjectMapException(std::string("ObjectMapException: expected value one or less value, but multiple found - <") + key + ">");
				return rng;
			}
			inline std::pair<typename ObjectMapPooledMap::const_iterator, typename ObjectMapPooledMap::const_iterator> getValues() const
			{
				return std::make_pair(data_.cbegin(), data_.cend());
			}

			inline std::pair<typename ObjectMapObjMap::const_iterator, typename ObjectMapObjMap::const_iterator> getObjects(const CharKey * key) const
			{
				pooledKey_ = key;
				return objs_.equal_range(pooledKey_);
			}
			inline std::pair<typename ObjectMapObjMap::const_iterator, typename ObjectMapObjMap::const_iterator> getObjects(const CharKey * key, ObjectMapPresence flg) const
			{
				pooledKey_ = key;
				auto rng = objs_.equal_range(pooledKey_);
				if (rng.first == rng.second)
				{
					if (flg == ObjectMapPresence::ONE || flg == ObjectMapPresence::ONE_PLUS)
						throw ObjectMapException(std::string("ObjectMapException: expected value one or more value, but none found - <") + key + ">");
					if (flg == ObjectMapPresence::ONE_OR_ZERO)
						return rng;
				}
				auto fst = rng.first;
				fst++;
				if (fst != rng.second)
					if (flg == ObjectMapPresence::ONE || flg == ObjectMapPresence::ONE_OR_ZERO)
						throw ObjectMapException(std::string("ObjectMapException: expected value one or less value, but multiple found - <") + key + ">");
				return rng;
			}
			inline std::pair<typename ObjectMapObjMap::const_iterator, typename ObjectMapObjMap::const_iterator> getObjects() const
			{
				return std::make_pair(objs_.cbegin(), objs_.cend());
			}

			template<class T>
			inline static void setValue(T &dst, const typename ObjectMapPooledMap::const_iterator &iter)
			{				
				str_cast(iter->second.first, dst, 0);
			}

#define OBJMAP_GET_ONE_VALUE(objmap,field,name)objmap.setValue(field, objmap.getValues(name, zenith::util::ObjectMapPresence::ONE).first);
#define OBJMAP_GET_ONE_VALUE_DEFAULT(objmap,field,name,def){ auto r = objmap.getValues(name, zenith::util::ObjectMapPresence::ONE_OR_ZERO);\
	if(r.first == r.second)field = def;\
	else objmap.setValue(field, r.first);}

		};
	}
}
