#pragma once
#include "buffer.h"
#include <string.h>

namespace zenith
{
	namespace util
	{
		class HDictException : public std::exception
		{
		public:
			HDictException() : std::exception("HDictException: unknown cause") {}
			HDictException(const char * p) : std::exception(p) {}
			virtual ~HDictException() {}
		};

		class HDictInvalidUse : public HDictException
		{
		public:
			HDictInvalidUse() : HDictException("HDictInvalidUse: unknown cause") {}
			HDictInvalidUse(const char * p) : HDictException(p) {}
			virtual ~HDictInvalidUse() {}
		};

		class HDictDuplicateKey : public HDictException
		{
		public:
			HDictDuplicateKey() : HDictException("HDictDuplicateKey: unknown cause") {}
			HDictDuplicateKey(const char * p) : HDictException(p) {}
			virtual ~HDictDuplicateKey() {}
		};
		class HDictMissingKey : public HDictException
		{
		public:
			HDictMissingKey() : HDictException("HDictMissingKey: unknown cause") {}
			HDictMissingKey(const char * p) : HDictException(p) {}
			virtual ~HDictMissingKey() {}
		};
		class HDictInvalidIterator : public HDictException
		{
		public:
			HDictInvalidIterator() : HDictException("HDictInvalidIterator: unknown cause") {}
			HDictInvalidIterator(const char * p) : HDictException(p) {}
			virtual ~HDictInvalidIterator() {}
		};

		template<class T>
		class hdict
		{
			struct record_
			{
				static const uint32_t NoValue = 0xFFFFFFFF;
				static const uint32_t NoName = 0xFFFFFFFF;
				uint32_t value_offset; //offset from left buffer side in bytes
				uint32_t name_offset; //offset from right buffer side (after records) in bytes
				inline void invalidate() { value_offset = NoValue; name_offset = NoName; }
				inline bool valid() const { return (value_offset != NoValue) && (name_offset != NoName); }
			};
			static const uint32_t NoOffset = 0xFFFFFFFF;
			static const uint32_t NoID = 0xFFFFFFFF;
			static const uint32_t RecordSize = sizeof(record_);
		public:

			typedef const char * key_type;
			typedef char key_elem_type;
			typedef T value_type;
			typedef T mapped_type;
			
			static const uint32_t ValueSize = sizeof(T);
			
			/*
			left side of buffer:
			val-0, val-1, ..., val-n
			
			right side of buffer (fixed size):
			name-0, name-1, ..., name-n,
			0 bucket: 0-elem-name-offset, 0-elem-val-index, 1-elem-name-offset, 1-elem-val-index, ..
			1 bucket: 0-elem-name-offset, 0-elem-val-index, 1-elem-name-offset, 1-elem-val-index, ..
			*/
		private:
			/*
			implementation: n-buckets, where n is ~ number of elements
			hash -> bucket
			*/
			static const uint32_t CombinedSize = RecordSize + ValueSize;
			


			static const uint32_t CombinedPOTSize = (1 +
				(((CombinedSize - 1) >> 1)
					| ((CombinedSize - 1) >> 2)
					| ((CombinedSize - 1) >> 4)
					| ((CombinedSize - 1) >> 8)
					| ((CombinedSize - 1) >> 16))) << 1;

			dynamic_dual_buffer buffer_;
			uint32_t numElements_, numBuckets_;
			uint32_t maxBucketElements_;

			enum class _TaskResult {UNDEF = 0, SUCCESS = 1, BUCKET_OVERFLOW, OUT_OF_MEM, DUP_KEY};
			enum class _Task{ UNDEF = 0, ADD_ONE, INIT };

			inline uint32_t sizeof_records_() const { return maxBucketElements_ * numBuckets_ * RecordSize; }
			inline record_ * ptr_records_() {
				return reinterpret_cast<record_ *>(buffer_.right_end() - sizeof_records_());
			}
			inline const record_ * ptr_records_() const {
				return reinterpret_cast<const record_ *>(buffer_.right_end() - sizeof_records_());
			}
			inline const char * ptr_names_end_() const { return reinterpret_cast<const char *>(buffer_.right_end() - sizeof_records_()); }



			void move_values_(void * ptr)
			{
				T * pDst = reinterpret_cast<T *>(ptr);
				T * pArray = reinterpret_cast<T *>(buffer_.begin());
				for (uint32_t i = 0; i < numElements_; i++)
				{
					new (&pDst[i]) T (std::move(pArray[i]));
					pArray[i].~T();
				}
			}
			void copy_values_(void * ptr_to) const
			{
				T * pDst = reinterpret_cast<T *>(ptr_to);
				const T * pArray = reinterpret_cast<const T *>(buffer_.begin());
				for (uint32_t i = 0; i < numElements_; i++)
					new (&pDst[i]) T(pArray[i]);
			}
			inline uint32_t align_size_(uint32_t sz) const
			{
				return sz + ((sz & (CombinedPOTSize - 1)) > 0 ? (CombinedPOTSize - (sz & (CombinedPOTSize - 1))) : 0);
			}
			
			inline void resize_buffer_(uint32_t newSize)
			{
				dynamic_dual_buffer bf(newSize);
				bf.left_grow(buffer_.left_size());
				bf.right_grow(buffer_.right_size());

				if (buffer_.valid())
				{
					move_values_(bf.left_vbegin());
					memcpy_s(bf.right_vbegin(), bf.right_size(), buffer_.right_vbegin(), buffer_.right_size());
				}
				buffer_ = std::move(bf);
			}

			inline void manage_size_(uint32_t reqSize, _Task task)
			{
				resize_buffer_(align_size_((static_cast<uint32_t>(buffer_.size()) + reqSize) << 1));
			}

			void shrink_buckets_()
			{

			}

			//used only on collision or when exceeding max bucket capacity
			//it does not add new key
			void grow_buckets_(const char * newKey)
			{
				uint32_t maxBuckets1_ = (numBuckets_ << 2) + 1; //four times larger
				uint32_t maxBuckets2_ = numBuckets_ + 10;
				uint32_t maxBuckets_ = numElements_ + 1;
				if (maxBuckets1_ > maxBuckets_)
					maxBuckets_ = maxBuckets1_;
				if (maxBuckets2_ > maxBuckets_)
					maxBuckets_ = maxBuckets2_;

				uint64_t * data = new uint64_t[numElements_ + 1 + maxBuckets_];
				uint64_t * hashes = data;
				uint64_t * buckets = data + numElements_ + 1;
				
				//1 step: calculate hashes
				const char * ptr0 = reinterpret_cast<const char *>(buffer_.end() - sizeof_records_()); // get pointer to first string
				for (uint32_t i = 0; i < numElements_; i++)
				{
					const char * p = ptr0 - 2; //-1 points to '\0', -2 points to char
					while (p >= reinterpret_cast<const char *>(buffer_.right_begin()) && *p)
						p--;
					//now p points on '\0' or right before first character
					ptr0 = p + 1;
					hashes[i] = hash_fvn1_(ptr0);
				}
				hashes[numElements_] = hash_fvn1_(newKey);

				//2 step: find suitable number of buckets
				uint32_t finalNumBuckets = 0;
				for (uint32_t newNumBuckets = numBuckets_+1; newNumBuckets < maxBuckets_; newNumBuckets++)
				{
					for (uint32_t i = 0; i < maxBuckets_; i++)
						buckets[i] = 0;
					bool suits = true;
					for(uint32_t i = 0; i < numElements_ + 1; i++)
						if (++buckets[hashes[i] % newNumBuckets] > maxBucketElements_)
						{
							suits = false;
							break;
						}
					if (suits)
					{
						finalNumBuckets = newNumBuckets;
						break;
					}
				}
				if (finalNumBuckets == 0)
					throw HDictInvalidUse("hdict failed to find suitable number of buckets for current collection.");

				//3 step: update bucket system
				uint32_t numNewCapacity = 4;
				uint32_t sizeIncrease = (finalNumBuckets - numBuckets_) * maxBucketElements_ * RecordSize + ValueSize * numNewCapacity + 16 * numNewCapacity;
				dynamic_dual_buffer bf(align_size_(static_cast<uint32_t>(buffer_.size()) + sizeIncrease));
				bf.left_grow(buffer_.left_size());
				uint32_t curBucketsSize = maxBucketElements_ * RecordSize * numBuckets_;
				uint32_t newBucketSize = maxBucketElements_ * RecordSize * finalNumBuckets;
				uint32_t offsetIncrease = newBucketSize - curBucketsSize;
				uint32_t curNamesSize = static_cast<uint32_t>(buffer_.right_size()) - curBucketsSize;
				bf.right_grow(newBucketSize + curNamesSize);
				
				move_values_(bf.vbegin());
				memcpy_s(bf.right_vbegin(), curNamesSize, buffer_.right_vbegin(), curNamesSize);
				uint32_t finalNumRecords = finalNumBuckets * maxBucketElements_;
				record_ * oldRecs = ptr_records_();
				record_ * newRecs = reinterpret_cast<record_ *>(bf.right_end() - finalNumRecords * RecordSize);

				for (uint32_t i = 0; i < finalNumRecords; i++)
					newRecs[i].invalidate();

				for (uint32_t i = 0; i < numElements_; i++)
				{
					uint32_t buckId = hashes[i] % finalNumBuckets;
					uint32_t new_off_j = buckId * maxBucketElements_;
					uint32_t old_off_j = (hashes[i] % numBuckets_) * maxBucketElements_;
					uint32_t old_j = 0, new_j = 0;
					uint32_t value_offset = i * ValueSize;
					for (uint32_t j = 0; j < maxBucketElements_; j++)
						if (newRecs[new_off_j + j].valid())
							continue;
						else
						{
							new_j = new_off_j + j;
							break;
						}
					for(uint32_t j = 0; j < maxBucketElements_; j++)
						if (oldRecs[old_off_j + j].value_offset == value_offset)
						{
							old_j = old_off_j + j;
							break;
						}

					newRecs[new_j].value_offset = value_offset;
					newRecs[new_j].name_offset = oldRecs[old_j].name_offset;
				}

				delete[] data;
				buffer_ = std::move(bf);
				auto tmp = buffer_.right_size();
				numBuckets_ = finalNumBuckets;
			}


			static inline uint64_t hash_fvn1_(const char * p)
			{
				uint64_t hash = 0xcbf29ce484222325;
				uint64_t prime = 0x100000001b3;
				while (*p)
				{
					hash *= prime;
					hash ^= *p++;
				}
				return hash;
			}

			inline _TaskResult try_add_(const char * key, uint32_t &value_offset)
			{
				if (numBuckets_ == 0)
					return _TaskResult::BUCKET_OVERFLOW;
				uint32_t slen = static_cast<uint32_t>(strlen(key));

				uint64_t hash = hash_fvn1_(key);
				uint32_t off_j = (hash % numBuckets_) * maxBucketElements_;
				uint32_t rec_id = 0;
				record_ * recs = ptr_records_();
				for (; rec_id < maxBucketElements_; rec_id++)
				{
					if (!recs[off_j + rec_id].valid())
						break;
					const char * name = ptr_names_end_() - recs[off_j + rec_id].name_offset;
					if (strcmp(name, key) == 0)
						return _TaskResult::DUP_KEY;
				}
				if (rec_id >= maxBucketElements_)
					return _TaskResult::BUCKET_OVERFLOW;

				if ((slen + 1) + ValueSize > buffer_.mid_size())
					return _TaskResult::OUT_OF_MEM;
				
				value_offset = static_cast<uint32_t>(buffer_.left_size());
				buffer_.left_grow(ValueSize);
				buffer_.right_grow(slen + 1);

				char * dstPtr = reinterpret_cast<char *>(buffer_.right_begin());
				uint32_t off = static_cast<uint32_t>(ptr_names_end_() - dstPtr);
				for (uint32_t i = 0; i < slen+1; i++)
					dstPtr[i] = key[i];

				recs[off_j + rec_id].name_offset = off;
				recs[off_j + rec_id].value_offset = value_offset;
				return _TaskResult::SUCCESS;
			}
			inline T * add_(const char * name)
			{
				uint32_t offset = 0;
				_TaskResult res = try_add_(name, offset);
				if (res == _TaskResult::DUP_KEY)
					throw HDictDuplicateKey("hdict::add: duplicate key.");
				if (res == _TaskResult::OUT_OF_MEM)
				{
					manage_size_(static_cast<uint32_t>(strlen(name)) + 1 + ValueSize, _Task::ADD_ONE);
					res = try_add_(name, offset);
				}
				if (res == _TaskResult::BUCKET_OVERFLOW)
				{
					grow_buckets_(name);
					res = try_add_(name, offset);
				}
				if (res != _TaskResult::SUCCESS)
				{
					if(res == _TaskResult::BUCKET_OVERFLOW)
						throw HDictInvalidUse("hdict::add: failed to add key due to bucket overflow, failed to grow buckets.");
					else 
						throw HDictInvalidUse("hdict::add: failed to add key due to unknown reasons.");
				}
				return reinterpret_cast<T *>(buffer_.left_begin() + offset);
			}
			inline uint32_t find_(const char * key) const
			{
				if (numBuckets_ == 0 || maxBucketElements_ == 0)
					return NoOffset;
				uint64_t hash = hash_fvn1_(key);
				uint32_t off_j = (hash % numBuckets_) * maxBucketElements_;
				uint32_t rec_id = 0;
				const record_ * recs = ptr_records_();
				for (; rec_id < maxBucketElements_; rec_id++)
				{
					if (!recs[off_j + rec_id].valid())
						break;
					const char * name = ptr_names_end_() - recs[off_j + rec_id].name_offset;
					if (strcmp(name, key) == 0)
						return recs[off_j + rec_id].value_offset;
				}
				return NoOffset;
			}
			inline uint32_t find_rec_(const char * key) const
			{
				if (numBuckets_ == 0 || maxBucketElements_ == 0)
					return NoOffset;
				uint64_t hash = hash_fvn1_(key);
				uint32_t off_j = (hash % numBuckets_) * maxBucketElements_;
				uint32_t rec_id = 0;
				const record_ * recs = ptr_records_();
				for (; rec_id < maxBucketElements_; rec_id++)
				{
					if (!recs[off_j + rec_id].valid())
						break;
					const char * name = ptr_names_end_() - recs[off_j + rec_id].name_offset;
					if (strcmp(name, key) == 0)
						return off_j + rec_id;
				}
				return NoOffset;
			}
			inline void clear_()
			{
				if (!buffer_.valid())
					return;
				T * vals = reinterpret_cast<T *>(buffer_.begin());
				for (uint32_t i = 0; i < numElements_; i++)
					vals[i].~T();
				buffer_.clear();
				numBuckets_ = 0;
				numElements_ = 0;
				maxBucketElements_ = 0;
			}
			template<class O, class U>
			class iterator_t_
			{
				friend class hdict;
				O * obj_;
				uint32_t id_;
				iterator_t_(O * obj, uint32_t rec_id) : obj_(obj), id_(rec_id) {}
				inline bool valid_() const
				{
					if (!obj_ || id_ == NoID)
						return false;
					if (id_ >= obj_->numBuckets_ * obj_->maxBucketElements_)
						return false;
					return true;
				}
				inline void * get_val_() const
				{
					if (!valid_())
						return nullptr;
					uint32_t off = obj_->ptr_records_()[id_].value_offset;
					return reinterpret_cast<void *>(const_cast<uint8_t *>(obj_->buffer_.begin() + off));
				}
				inline const char * get_key_() const
				{
					if (!valid_())
						return nullptr;
					uint32_t off = obj_->ptr_records_()[id_].name_offset;
					return obj_->ptr_names_end_() - off;
				}
				inline uint32_t next_id_() const
				{
					uint32_t maxId = obj_->numBuckets_ * obj_->maxBucketElements_;
					auto recs = obj_->ptr_records_();
					uint32_t newId = id_ + 1;
					while (newId < maxId && !recs[newId].valid())
						newId++;
					if (newId >= maxId)
						return NoID;
					return newId;
				}
			public:
				inline iterator_t_() : obj_(nullptr), id_(NoID) {}
				inline iterator_t_(const iterator_t_ &o) : obj_(o.obj_), id_(o.id_) {}
				inline iterator_t_(iterator_t_ &&o) : obj_(o.obj_), id_(o.id_) { o.id_ = NoID; o.obj_ = nullptr; }

				inline iterator_t_ &operator=(const iterator_t_ &o)
				{
					obj_ = o.obj_; id_ = o.id_;
					return *this;
				}
				inline iterator_t_ &operator=(iterator_t_ &&o)
				{
					obj_ = o.obj_; id_ = o.id_;					
					o.id_ = NoID; o.obj_ = nullptr;
					return *this;
				}

				inline iterator_t_<O, U> next()
				{
					if(!valid_())
						return iterator_t_<O, U>(obj_, NoID);
					return iterator_t_<O, U>(obj_, next_id_());
				}
				inline iterator_t_<const O, const U> next() const
				{
					if (!valid_())
						return iterator_t_<const O, const U>(obj_, NoID);
					return iterator_t_<const O, const U>(obj_, next_id_());
				}
				inline U &value()
				{
					auto tmp = reinterpret_cast<U *>(get_val_());
					if (!tmp)
						throw HDictInvalidIterator("iterator::value(): could not get value of iterator");
					return *tmp;
				}
				inline const U &value() const
				{
					auto tmp = reinterpret_cast<const U *>(get_val_());
					if (!tmp)
						throw HDictInvalidIterator("iterator::value(): could not get value of iterator");
					return *tmp;
				}
				inline U *ptr()
				{
					auto tmp = reinterpret_cast<U *>(get_val_());
					if (!tmp)
						throw HDictInvalidIterator("iterator::ptr(): could not get value of iterator");
					return tmp;
				}
				inline const U *ptr() const
				{
					auto tmp = reinterpret_cast<const U *>(get_val_());
					if (!tmp)
						throw HDictInvalidIterator("iterator::ptr(): could not get value of iterator");
					return tmp;
				}
				inline const char * key() const
				{
					auto tmp = get_key_();
					if(!tmp)
						throw HDictInvalidIterator("iterator::key(): could not get name of iterator");
					return tmp;
				}

				inline U &operator *() { return value(); }
				inline const U &operator *() const { return value(); }

				inline iterator_t_<O, U> &operator++() { id_ = next_id_(); return *this; }
				//postfix
				inline iterator_t_<O, U> operator++(int) { auto p = id_; id_ = next_id_(); return iterator_t_<O, U>(obj_, p); }

				template<class O2, class U2>
				inline bool operator ==(const iterator_t_<O2, U2> &o) const
				{
					if (!o.valid_() || !valid_())
					{
						if(!o.valid_() && !valid_())
							return true;
						return false;
					}
					return (obj_ == o.obj_) && (id_ == o.id_);
				}
				template<class O2, class U2>
				inline bool operator !=(const iterator_t_<O2, U2> &o) const
				{
					if (!o.valid_() || !valid_())
					{
						if (!o.valid_() && !valid_())
							return false;
						return true;
					}
					return (obj_ != o.obj_) || (id_ != o.id_);
				}
			};
			uint32_t begin_id_() const
			{
				const record_ * recs = ptr_records_();
				uint32_t id = 0;
				while (id < numBuckets_ * maxBucketElements_)
					if (recs[id].valid())
						return id;
					else
						id++;
				return NoID;
			}
		public:

			typedef iterator_t_<hdict<T>, T> iterator;
			typedef iterator_t_<const hdict<T>, const T> const_iterator;

			inline ~hdict()
			{
				clear_();
			}
			inline hdict(uint32_t maxBucketElements = 3) : numBuckets_(0), maxBucketElements_(3), numElements_(0)
			{
				manage_size_(1, _Task::INIT);
			}
			inline hdict(const hdict &h) : maxBucketElements_(h.maxBucketElements_), numBuckets_(h.numBuckets_), numElements_(h.numElements_), buffer_(h.buffer_)
			{
				h.copy_values_(buffer_.vbegin());
			}
			inline hdict(hdict &&h) : maxBucketElements_(h.maxBucketElements_), numBuckets_(h.numBuckets_), numElements_(h.numElements_),
				buffer_(std::move(h.buffer_))
			{
				h.numElements_ = 0;
				h.numBuckets_ = 0;
			}

			inline hdict &operator =(const hdict &h)
			{
				clear_();
				maxBucketElements_ = h.maxBucketElements_;
				numBuckets_ = h.numBuckets_;
				numElements_ = h.numElements_;
				buffer_ = h.buffer_;
				h.copy_values_(buffer_.vbegin());
				return *this;
			}
			inline hdict &operator =(hdict &&h) 
			{
				clear_();
				maxBucketElements_ = h.maxBucketElements_;
				numBuckets_ = h.numBuckets_;
				numElements_ = h.numElements_;
				buffer_ = std::move(h.buffer_);
				h.numElements_ = 0;
				h.numBuckets_ = 0;
				return *this;
			}

			inline iterator begin() { return iterator(this, begin_id_()); }
			inline iterator end() { return iterator(this, NoID); }
			inline const_iterator begin() const { return const_iterator(this, begin_id_()); }
			inline const_iterator end() const { return const_iterator(this, NoID); }

			inline iterator find(const char * name) { return iterator(this, find_rec_(name)); }
			inline const_iterator find(const char * name) const { return const_iterator(this, find_rec_(name)); }

			inline void add(const char * name, T &&value)
			{
				T * val = add_(name);
				new (val) T(std::move(value));
				numElements_++;
			}
			inline void add(const char * name, const T &value)
			{
				T * val = add_(name);
				new (val) T(value);
				numElements_++;
			}
			inline void insert(const std::pair<const char *, const T &> &p)	{ add(p.first, p.second);}
			inline void insert(const std::pair<const char *, T> &p) { add(p.first, p.second); }
			inline void insert(std::pair<const char *, T> &&p) { add(p.first, std::move(p.second)); }

			inline bool exists(const char * name) const
			{
				return find_(name) != NoOffset;
			}
			inline const T &get(const char * name) const
			{
				if (numElements_ == 0)
					throw HDictMissingKey("hdict::get: no elements in hdict");
				uint32_t off = find_(name);
				if(off == NoOffset)
					throw HDictMissingKey("hdict::get: missing specified key in hdict");
				return *reinterpret_cast<const T *>(buffer_.begin() + off);
			}
			inline T &get(const char * name)
			{
				if (numElements_ == 0)
					throw HDictMissingKey("hdict::get: no elements in hdict");
				uint32_t off = find_(name);
				if (off == NoOffset)
					throw HDictMissingKey("hdict::get: missing specified key in hdict");
				return *reinterpret_cast<T *>(buffer_.begin() + off);
			}
			inline const T &at(const char * name) const
			{
				if (numElements_ == 0)
					throw HDictMissingKey("hdict::get: no elements in hdict");
				uint32_t off = find_(name);
				if (off == NoOffset)
					throw HDictMissingKey("hdict::get: missing specified key in hdict");
				return *reinterpret_cast<const T *>(buffer_.begin() + off);
			}
			inline T &at(const char * name)
			{
				if (numElements_ == 0)
					throw HDictMissingKey("hdict::get: no elements in hdict");
				uint32_t off = find_(name);
				if (off == NoOffset)
					throw HDictMissingKey("hdict::get: missing specified key in hdict");
				return *reinterpret_cast<T *>(buffer_.begin() + off);
			}
			inline uint32_t size() const { return numElements_; }
			inline uint32_t capacity_bytes() const { return static_cast<uint32_t>(buffer_.size()); }
		};
	}
}