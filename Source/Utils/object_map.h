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

		template<class CharKey, class CharValue>
		class ObjectMap_Iterator;

		template<class CharKey = char, class CharValue = char>
		class ObjectMap
		{
			friend class ObjectMap_Iterator<CharKey, CharValue>;

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
			ObjectMapObjMap objs_;

			mutable ObjectMapStrKey pooledKey_;
			mutable ObjectMapStrValue pooledValue_;
		public:
			typedef typename ObjectMapObjMap::iterator obj_iterator;
			typedef typename ObjectMapObjMap::const_iterator obj_iterator_const;
			typedef typename ObjectMapPooledMap::iterator val_iterator;
			typedef typename ObjectMapPooledMap::const_iterator val_iterator_const;

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

			inline val_iterator addValue(const CharKey * key, const CharValue * val, ObjectMapValueHint hint = ObjectMapValueHint::UNDEF)
			{
				pooledKey_ = key;
				pooledValue_ = val;
				return data_.insert(std::make_pair(pooledKey_, std::make_pair(pooledValue_, hint)));
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

			std::pair<ObjectMap_Iterator<CharKey, CharValue>, ObjectMap_Iterator<CharKey, CharValue>> children() const;
			std::pair<ObjectMap_Iterator<CharKey, CharValue>, ObjectMap_Iterator<CharKey, CharValue>> children(const CharKey * key) const;
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



		template<class CharKey, class CharValue>
		class ObjectMap_Iterator
		{
			friend class ObjectMap<CharKey, CharValue>;
			typedef typename ObjectMap<CharKey, CharValue>::obj_iterator_const obj_iterator;
			typedef typename ObjectMap<CharKey, CharValue>::val_iterator_const val_iterator;

			obj_iterator objIt_, objEnd_;
			val_iterator valIt_, valEnd_;

			ObjectMap_Iterator(const obj_iterator &oi, const obj_iterator &oe,
				const val_iterator &vi, const val_iterator &ve)
				: objIt_(oi), objEnd_(oe), valIt_(vi), valEnd_(ve)
			{}
		public:
			ObjectMap_Iterator() : objIt_(obj_iterator()), objEnd_(obj_iterator()),
				valIt_(val_iterator()), valEnd_(val_iterator())
			{}
			ObjectMap_Iterator(const ObjectMap_Iterator<CharKey, CharValue> &it)
				: objIt_(it.objIt_), objEnd_(it.objEnd_),
				valIt_(it.valIt_), valEnd_(it.valEnd_)
			{}
			inline bool empty() const
			{
				return (objIt_ == objEnd_) && (valIt_ == valEnd_);
			}
			inline bool is_object() const
			{
				return (objIt_ != objEnd_);
			}
			inline bool is_value() const
			{
				return (objIt_ == objEnd_) && (valIt_ != valEnd_);
			}
			inline const CharKey * key() const
			{
				if (objIt_ != objEnd_)
					return objIt_->first.c_str();
				else if(valIt_ != valEnd_)
					return valIt_->first.c_str();
				else return nullptr;
			}

			inline const CharValue * value() const
			{
				if ((objIt_ == objEnd_) && (valIt_ != valEnd_))
					return valIt_->second.first.c_str();
				else return nullptr;
			}
			inline ObjectMapValueHint value_hint() const
			{
				if ((objIt_ == objEnd_) && (valIt_ != valEnd_))
					return valIt_->second.second;
				else return ObjectMapValueHint::UNDEF;
			}
			inline static ObjectMap_Iterator<CharKey, CharValue> end()
			{
				return ObjectMap_Iterator<CharKey, CharValue>();
			}
			inline ObjectMap_Iterator<CharKey, CharValue> next() const
			{
				if (objIt_ != objEnd_)
				{
					auto nextIt = objIt_;
					++nextIt;
					if ((nextIt != objEnd_) || (valIt_ != valEnd_))
						return ObjectMap_Iterator<CharKey, CharValue>(nextIt, objEnd_, valIt_, valEnd_);
					return end();
				}
				if (valIt_ != valEnd_)
				{
					auto nextIt = valIt_;
					++nextIt;
					if(nextIt != valEnd_)
						return ObjectMap_Iterator<CharKey, CharValue>(objIt_, objEnd_, nextIt, valEnd_);
					return end();
				}
			}
			inline bool operator ==(const ObjectMap_Iterator<CharKey, CharValue> &n) const
			{
				bool a = empty(), b = n.empty();
				if (a && b)
					return true;
				if (a || b)
					return false;
				return (objIt_ == n.objIt_) && (valIt_ == n.valIt_);
			}
			inline std::pair<ObjectMap_Iterator<CharKey, CharValue>, ObjectMap_Iterator<CharKey, CharValue>> children() const
			{
				if (objIt_ == objEnd_)
					throw ObjectMapException("ObjectMap_Iterator::children(): can not return children of non-object iterator!");
				return objIt_->second.children();
			}
			inline std::pair<ObjectMap_Iterator<CharKey, CharValue>, ObjectMap_Iterator<CharKey, CharValue>> children(const CharKey * key) const
			{
				if (objIt_ == objEnd_)
					throw ObjectMapException("ObjectMap_Iterator::children(): can not return children of non-object iterator!");
				return objIt_->second.children(key);
			}
		};

		template<class CharKey, class CharValue>
		inline std::pair<ObjectMap_Iterator<CharKey, CharValue>, ObjectMap_Iterator<CharKey, CharValue>>
			ObjectMap<CharKey, CharValue>::children() const
		{
			return std::pair<ObjectMap_Iterator<CharKey, CharValue>, ObjectMap_Iterator<CharKey, CharValue>>(
				ObjectMap_Iterator<CharKey, CharValue>(objs_.begin(), objs_.end(), data_.begin(), data_.end()),
				ObjectMap_Iterator<CharKey, CharValue>::end());
		}
		template<class CharKey, class CharValue>
		inline std::pair<ObjectMap_Iterator<CharKey, CharValue>, ObjectMap_Iterator<CharKey, CharValue>>
			ObjectMap<CharKey, CharValue>::children(const CharKey * key) const
		{
			pooledKey_ = key;
			auto po = objs_.equal_range(pooledKey_);
			auto pd = data_.equal_range(pooledKey_);
			return std::pair<ObjectMap_Iterator<CharKey, CharValue>, ObjectMap_Iterator<CharKey, CharValue>>(
				ObjectMap_Iterator<CharKey, CharValue>(po.first, po.second, pd.first, pd.second),
				ObjectMap_Iterator<CharKey, CharValue>::end());
		}
	}
}
