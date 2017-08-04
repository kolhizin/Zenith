#pragma once
#include "ff_zds.h"
#include "../hdict.h"

namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{
			class ZDataStructureException : public ZFileException
			{
			public:
				ZDataStructureException() : ZFileException() {}
				ZDataStructureException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : ZFileException(p, type)
				{
				}
				ZDataStructureException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : ZFileException(str, type)
				{
				}
				virtual ~ZDataStructureException() {}
			};

			class ZDataStructureIteratorException : public ZDataStructureException
			{
			public:
				ZDataStructureIteratorException() : ZDataStructureException() {}
				ZDataStructureIteratorException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : ZDataStructureException(p, type)
				{
				}
				ZDataStructureIteratorException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : ZDataStructureException(str, type)
				{
				}
				virtual ~ZDataStructureIteratorException() {}
			};

			class zDataStorage
			{
				static const uint32_t NoIDX = 0xFFFFFFFF;
				static const uint32_t NoTAG = 0xFFFF;

				struct ChunkLink_
				{
					uint32_t parentIdx_ = NoIDX; //up - index in <links_> vector

					uint32_t chunkIdx_ = NoIDX; //index in <chunks_> vector
					uint32_t prevIdx_ = NoIDX, nextIdx_ = NoIDX; // left - right - indices in <links_> vector

					uint32_t firstChildIdx_ = NoIDX, lastChildIdx_ = NoIDX; //down - index in <links_> vector
				};



				uint8_t * dataHeap_;
				uint64_t dataHeapSize_, dataHeapCapacity_;
				ZChunk16B_ZDSEntry * chunks_;
				uint32_t numChunks_, capacityChunks_;

				uint8_t * tmpTagHeap_;
				uint32_t tmpTagHeapSize_;

				//utility structures
				std::vector<ChunkLink_> chunksLinks_;
				std::vector<uint32_t> freeList_;
				zenith::util::hdict<uint32_t> tags_;
				mutable std::vector<const char *> tagNames_;
				uint32_t maxTagId_;
				bool isOwner_;

				enum TaskResult_ {
					UNDEF = 0, SUCCESS = 1, OUT_OF_MEM_DATA = 2, OUT_OF_MEM_CHUNKS = 4,
					MULTIROOT = 8, NOROOT = 16, CHUNK_ORDER = 32, CHUNK_MISMATCH_LINKS = 64,
					HEAP_OVERLAP = 128, HEAP_CORRUPTED = 256
				};

				/*
				on add node:
				- check chunk capacity / freelist
				- [optional] check data-heap capacity
				- determine tag-id (or add to tags structure)
				- determine depth (based on parent link)
				- determine data location (inplace or in data-heap)
				- [optional] add data to data-heap
				- add chunk
				- update links

				on remove node:
				- update links
				- add chunk-id to freelist
				*/

				inline uint32_t gen_or_get_tag_id_(const char * tag)
				{
					uint32_t tagId = maxTagId_;
					auto tagIt = tags_.find(tag);
					if (tagIt != tags_.end())
						return *tagIt;
					else
					{
						tags_.insert(std::pair<const char *, uint32_t>(tag, tagId));
						maxTagId_++;
						tagNames_.clear();
						return tagId;
					}
				}
				inline ZChunk16B_ZDSEntry * get_free_entry_()
				{
					if (!freeList_.empty())
					{
						ZChunk16B_ZDSEntry * res = &chunks_[freeList_.back()];
						freeList_.pop_back();
						return res;
					}
					if (numChunks_ < capacityChunks_)
					{
						ZChunk16B_ZDSEntry * res = &chunks_[numChunks_];
						numChunks_++;
						return res;
					}
					return nullptr;
				}
				void resize_chunks_(uint32_t newNumChunks);
				void resize_heap_(uint64_t newHeapSize);

				TaskResult_ add_chunk_(uint32_t lnkParentIDX, const char * tag, uint64_t data_size, bool isAttribute, ZDSDataBaseType type, ZChunk16B_ZDSEntry **chunkPtr, uint32_t * chunkLnk);
				TaskResult_ remove_chunk_(uint32_t lnkIDX);
				TaskResult_ reset_chunk_size_impl_(uint32_t chunkIDX, uint64_t new_size);

				inline uint32_t append_(uint32_t lnkParentIDX, const char * tag, uint64_t data_size, bool isAttribute = false, ZDSDataBaseType type = ZDSDataBaseType::ANY)
				{
					uint32_t lnkIDX;
					TaskResult_ res = add_chunk_(lnkParentIDX, tag, data_size, isAttribute, type, nullptr, &lnkIDX);
					if (res == TaskResult_::SUCCESS)
						return lnkIDX;

					if (res & TaskResult_::OUT_OF_MEM_CHUNKS)
						resize_chunks_(capacityChunks_ + (capacityChunks_ >> 3) + 2);
					if (res & TaskResult_::OUT_OF_MEM_DATA)
						resize_heap_(dataHeapCapacity_ + (dataHeapCapacity_ >> 3) + data_size);

					res = add_chunk_(lnkParentIDX, tag, data_size, isAttribute, type, nullptr, &lnkIDX);
					if (res == TaskResult_::SUCCESS)
						return lnkIDX;
					throw ZDataStructureException("zDataStructure::append_: failed.");
				}
				inline void remove_(uint32_t lnkIDX)
				{
					TaskResult_ res = remove_chunk_(lnkIDX);
					if (res == TaskResult_::SUCCESS)
						return;

					throw ZDataStructureException("zDataStructure::remove_: failed.");
				}
				inline void reset_chunk_size_(uint32_t chunkIDX, uint64_t new_size)
				{
					TaskResult_ res = reset_chunk_size_impl_(chunkIDX, new_size);
					
					if (res == TaskResult_::SUCCESS)
						return;
					if (res & TaskResult_::OUT_OF_MEM_DATA)
						resize_heap_(dataHeapCapacity_ + (dataHeapCapacity_ >> 3) + new_size);

					res = reset_chunk_size_impl_(chunkIDX, new_size);
					if (res == TaskResult_::SUCCESS)
						return;
					throw ZDataStructureException("zDataStructure::reset_chunk_size_: failed.");
				}
				
				void traverse_links_to_find_used_(uint32_t lnkIDX, std::vector<bool>&uchunks, std::vector<bool>&ulinks);				
				TaskResult_ traverse_links_to_move_chunks_(uint32_t lnkIDX, uint32_t depth, uint32_t &pos);
				void move_link_(uint32_t curIDX, uint32_t newIDX);
				
				TaskResult_ optimize_chunks_();
				TaskResult_ optimize_data_();				
				TaskResult_ optimize_tags_();				

				TaskResult_ optimize_();

				void clear_();
				void update_tag_names_() const;
				void update_tag_heap_();

				template<class O, bool FixedTag>
				class iterator_t_
				{
					friend class zDataStorage;
					O * obj_;
					uint32_t linkId_ = NoIDX;
					uint32_t tagId_ = NoIDX;

					inline bool valid_() const
					{
						return (obj_ != nullptr) && (linkId_ < obj_->chunksLinks_.size()) && (obj_->chunksLinks_[linkId_].chunkIdx_ != NoIDX);
					}
					iterator_t_(O * obj, uint32_t linkId) : obj_(obj), linkId_(linkId)
					{
						if (valid_())
							tagId_ = obj_->chunks_[obj_->chunksLinks_.at(linkId_).chunkIdx_].tagId();
					}
					inline void assert_() const
					{
						if (!valid_())
							throw ZDataStructureIteratorException("zDataStructure::iterator: failed assert.");
					}
					inline void assert_size_(uint64_t sz) const
					{
						assert_();
						if (obj_->chunks_[obj_->chunksLinks_[linkId_].chunkIdx_].size() < sz)
							throw ZDataStructureIteratorException("zDataStructure::iterator: failed size assert.");
					}
					inline uint32_t next_() const
					{
						if(!FixedTag)
							return obj_->chunksLinks_[linkId_].nextIdx_;
						uint32_t nid = obj_->chunksLinks_[linkId_].nextIdx_;
						while (nid != NoIDX && obj_->chunks_[obj_->chunksLinks_[nid].chunkIdx_].tagId() != tagId_)
							nid = obj_->chunksLinks_[nid].nextIdx_;
						return nid;
					}
					inline uint32_t prev_() const
					{
						if (!FixedTag)
							return obj_->chunksLinks_[linkId_].prevIdx_;
						uint32_t nid = obj_->chunksLinks_[linkId_].prevIdx_;
						while (nid != NoIDX && obj_->chunks_[obj_->chunksLinks_[nid].chunkIdx_].tagId_ != tagId_)
							nid = obj_->chunksLinks_[nid].prevIdx_;
						return nid;
					}
					inline uint32_t parent_() const
					{
						return obj_->chunksLinks_[linkId_].parentIdx_;
					}
					inline uint32_t child_first_() const
					{
						return obj_->chunksLinks_[linkId_].firstChildIdx_;
					}
					inline uint32_t child_last_() const
					{
						return obj_->chunksLinks_[linkId_].lastChildIdx_;
					}
					inline const ZChunk16B_ZDSEntry & chunk_ref_() const
					{
						assert_();
						return obj_->chunks_[obj_->chunksLinks_[linkId_].chunkIdx_];
					}
					inline ZChunk16B_ZDSEntry & chunk_ref_()
					{
						assert_();
						return obj_->chunks_[obj_->chunksLinks_[linkId_].chunkIdx_];
					}
					inline const ChunkLink_ & link_ref_() const
					{
						assert_();
						return obj_->chunksLinks_[linkId_];
					}
					inline ChunkLink_ & link_ref_()
					{
						assert_();
						return obj_->chunksLinks_[linkId_];
					}
				public:
					inline iterator_t_() : obj_(nullptr), linkId_(NoIDX), tagId_(NoIDX) {}
					inline iterator_t_(const iterator_t_ &o) : obj_(o.obj_), linkId_(o.linkId_), tagId_(o.tagId_) {}
					inline iterator_t_(iterator_t_ &&o) : obj_(o.obj_), linkId_(o.linkId_), tagId_(o.tagId_) {}
					inline iterator_t_ &operator =(const iterator_t_ &o) { obj_ = o.obj_; linkId_ = o.linkId_; tagId_ = o.tagId_; return *this; }
					inline iterator_t_ &operator =(iterator_t_ &&o) { obj_ = o.obj_; linkId_ = o.linkId_; tagId_ = o.tagId_; return *this; }

					inline uint32_t chunk_id() const
					{
						assert_();
						return obj_->chunksLinks_[linkId_].chunkIdx_;
					}
					inline uint8_t * data()
					{
						return chunk_ref_().data(obj_->dataHeap_);
					}
					inline const uint8_t * data() const
					{
						return chunk_ref_().data(obj_->dataHeap_);
					}
					inline uint64_t size() const
					{
						return chunk_ref_().size();
					}

					inline operator iterator_t_<const O, FixedTag>() const
					{
						return iterator_t_<const O, FixedTag>(obj_, linkId_);
					}

					inline bool is_attribute() const { return chunk_ref_().is_attribute(); }
					inline void reset_attribute(bool isAttribute) { chunk_ref_().reset_attribute(isAttribute); }
					inline ZDSDataBaseType type() const { return chunk_ref_().type(); }
					inline void reset_type(ZDSDataBaseType type) { return chunk_ref_().reset_type(type); }

					template<class T>
					void write(const T &val)
					{
						assert_size_(sizeof(T));
						*reinterpret_cast<T *>(chunk_ref_().data(obj_->dataHeap_)) = val;
					}
					template<class T>
					void read(T &val) const
					{
						assert_size_(sizeof(T));
						val = *reinterpret_cast<const T *>(chunk_ref_().data(obj_->dataHeap_));
					}
					template<class T>
					const T &value() const
					{
						assert_size_(sizeof(T));
						return *reinterpret_cast<const T *>(chunk_ref_().data(obj_->dataHeap_));
					}
					template<class T>
					T &value()
					{
						assert_size_(sizeof(T));
						return *reinterpret_cast<T *>(chunk_ref_().data(obj_->dataHeap_));
					}

					inline void copy_data_to_chunk(const uint8_t * src, uint64_t src_size)
					{
						assert_size_(src_size);
						memcpy_s(data(), size(), src, src_size);
					}
					inline void copy_data_from_chunk(uint8_t * dst, uint64_t dst_size) const
					{
						assert_();
						uint64_t sz = size();
						if (sz > dst_size)
							throw ZDataStructureIteratorException("zDataStructure::iterator: copy from chunk failed.");
						memcpy_s(dst, dst_size, data(), sz);
					}

					inline void reset_size(uint64_t new_size)
					{
						assert_();
						obj_->reset_chunk_size_(chunk_id(), new_size);
					}

					inline uint32_t tag_id() const
					{
						if(FixedTag)
							return tagId_;
						else
							return chunk_ref_().tagId();
					}
					inline const char * tag_name() const
					{						
						return obj_->tag(tag_id());
					}

					inline iterator_t_<O, false> append(const char * tag, uint32_t dataSize, bool isAttribute=false, ZDSDataBaseType type=ZDSDataBaseType::ANY)
					{
						assert_();
						return iterator_t_<O, false>(obj_, obj_->append_(linkId_, tag, dataSize, isAttribute, type));
					}

					inline bool valid() const { return valid_(); }

					inline iterator_t_<O, false> unfixed() const { return iterator_t_<O, false>(obj_, linkId_); }
					inline iterator_t_<O, true> fixed() const { return iterator_t_<O, true>(obj_, linkId_); }

					inline iterator_t_<O, FixedTag> next() const { assert_(); return iterator_t_<O, FixedTag>(obj_, next_()); }
					inline iterator_t_<O, FixedTag> prev() const { assert_(); return iterator_t_<O, FixedTag>(obj_, prev_()); }
					inline iterator_t_<O, false> parent() const { assert_(); return iterator_t_<O, false>(obj_, parent_()); }

					inline iterator_t_<O, false> child_begin() const { assert_(); return iterator_t_<O, false>(obj_, child_first_()); }
					inline iterator_t_<O, false> child_end() const { assert_(); return iterator_t_<O, false>(obj_, NoIDX); }

					inline iterator_t_<O, true> child_begin(const char * tag) const
					{
						assert_();
						uint32_t tid = obj_->tag_id(tag);
						if(tid == NoIDX)
							return iterator_t_<O, true>(obj_, NoIDX);
						uint32_t idx = child_first_();
						while (idx != NoIDX && obj_->chunks_[obj_->chunksLinks_[idx].chunkIdx_].tagId() != tid)
							idx = obj_->chunksLinks_[idx].nextIdx_;
						return iterator_t_<O, true>(obj_, idx);
					}
					inline iterator_t_<O, true> child_end(const char * tag) const { assert_(); return iterator_t_<O, true>(obj_, NoIDX); }

					inline bool has_children() const
					{
						return (link_ref_().firstChildIdx_ != NoIDX);
					}
					inline bool has_children(const char * tag) const
					{
						assert_();
						uint32_t tid = obj_->tag_id(tag);
						if (tid == NoIDX)
							return false;
						uint32_t idx = child_first_();
						while (idx != NoIDX && obj_->chunks_[obj_->chunksLinks_[idx].chunkIdx_].tagId() != tid)
							idx = obj_->chunksLinks_[idx].nextIdx_;
						return (idx != NoIDX);
					}
					inline bool has_parrent() const
					{
						return (link_ref_().parentIdx_ != NoIDX);
					}

					inline iterator_t_<O, FixedTag> &operator ++() { assert_(); linkId_ = next_(); return *this; }
					inline iterator_t_<O, FixedTag> &operator --() { assert_(); linkId_ = prev_(); return *this; }
					inline iterator_t_<O, FixedTag> operator ++(int) { assert_(); auto tmp = *this; linkId_ = next_(); return tmp; }
					inline iterator_t_<O, FixedTag> operator --(int) { assert_(); auto tmp = *this; linkId_ = prev_(); return tmp; }

					template<class O2, bool F2> inline bool operator ==(const iterator_t_<O2, F2> &o) const
					{
						bool v1 = valid_();
						bool v2 = o.valid_();
						if (v1 && v2)
							return (obj_ == o.obj_) && (linkId_ == o.linkId_);
						return (v1 == v2);
					}
					template<class O2, bool F2> inline bool operator !=(const iterator_t_<O2, F2> &o) const
					{
						bool v1 = valid_();
						bool v2 = o.valid_();
						if (v1 && v2)
							return (obj_ != o.obj_) || (linkId_ != o.linkId_);
						return (v1 != v2);
					}
				};

			public:
				typedef iterator_t_<zDataStorage, false> iterator;
				typedef iterator_t_<const zDataStorage, false> const_iterator;

				typedef iterator_t_<zDataStorage, false> unfixed_iterator;
				typedef iterator_t_<const zDataStorage, false> const_unfixed_iterator;

				typedef iterator_t_<zDataStorage, true> fixed_iterator;
				typedef iterator_t_<const zDataStorage, true> const_fixed_iterator;

				enum class DataOwnership {UNDEF = 0, COPY, OBTAIN_OWNERSHIP, NO_OWNERSHIP};
				~zDataStorage();
				explicit zDataStorage(const char * root_tag, uint32_t root_size = sizeof(void*), uint32_t chunksCapacity = 20, uint64_t heapCapacity = 1024);
				zDataStorage(const zDataStorageDescription &descr, DataOwnership ownership = DataOwnership::COPY);
				zDataStorage(zDataStorage &&zds);
				zDataStorage(const zDataStorage &zds) = delete;
				zDataStorage &operator =(zDataStorage &&zds);
				zDataStorage &operator =(const zDataStorage &zds) = delete;
				zDataStorage clone() const;

				inline iterator root() { return iterator(this, 0); }
				inline const_iterator root() const { return const_iterator(this, 0); }

				inline const char * tag(uint32_t id) const
				{
					if (tagNames_.empty())
						update_tag_names_();
					return tagNames_.at(id);
				}
				inline uint32_t tag_id(const char * tag) const
				{
					auto i = tags_.find(tag);
					if (i == tags_.end())
						return NoIDX;
					return *i;
				}
				inline void clear() {clear_();}
				void optimize();
				zDataStorageDescription descr();
			};
		}
	}
}
