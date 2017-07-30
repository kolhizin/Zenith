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
					uint32_t tagId = maxTagId_ + 1;
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
				TaskResult_ add_chunk_(uint32_t parentIDX, const char * tag, uint64_t data_size, ZChunk16B_ZDSEntry **chunkPtr, uint32_t * chunkLnk);
				TaskResult_ remove_chunk_(uint32_t lnkIDX);

				inline uint32_t append_(uint32_t parentIDX, const char * tag, uint64_t data_size)
				{
					uint32_t lnkIDX;
					TaskResult_ res = add_chunk_(parentIDX, tag, data_size, nullptr, &lnkIDX);
					if (res == TaskResult_::SUCCESS)
						return lnkIDX;

					if (res & TaskResult_::OUT_OF_MEM_CHUNKS)
						throw ZDataStructureException("zDataStructure::append_: out of chunk memory.");
					if (res & TaskResult_::OUT_OF_MEM_DATA)
						throw ZDataStructureException("zDataStructure::append_: out of heap memory.");
					res = add_chunk_(parentIDX, tag, data_size, nullptr, &lnkIDX);
					if (res == TaskResult_::SUCCESS)
						return lnkIDX;
					throw ZDataStructureException("zDataStructure::append_: failed.");
				}
				inline void remove_(uint32_t lnkIDX)
				{
					TaskResult_ res = remove_chunk_(lnkIDX);
					if (res == TaskResult_::SUCCESS)
						return;

					throw ZDataStructureException("zDataStructure::append_: failed.");
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

				template<class O>
				class iterator_t_
				{
					O * obj_;
					uint32_t linkId_;

					iterator_t_(O * obj, uint32_t linkId) : obj_(obj), linkId_(linkId) {}
					inline bool valid_() const
					{
						return (obj_ != nullptr) && (linkId_ < obj_->chunksLists_.size()) && (obj_->chunksLists_[linkId_].chunkId_ != NoIDX);
					}
					inline void assert_() const
					{
						if (!valid_())
							throw ZDataStructureIteratorException("zDataStructure::iterator: failed assert.");
					}
					inline uint32_t next_() const
					{
						return obj_->chunksLists_[linkId_]->nextIdx_;
					}
					inline uint32_t prev_() const
					{
						return obj_->chunksLists_[linkId_]->prevIdx_;
					}
					inline uint32_t parent_() const
					{
						return obj_->chunksLists_[linkId_]->parentIdx_;
					}
					inline uint32_t child_first_() const
					{
						return obj_->chunksLists_[linkId_]->firstChildIdx_;
					}
					inline uint32_t child_last_() const
					{
						return obj_->chunksLists_[linkId_]->firstChildIdx_;
					}
				public:
					inline iterator_t_() : obj_(nullptr), linkId_(NoIDX) {}
					inline iterator_t_(const iterator_t_ &o) : obj_(o.obj_), linkId_(o.linkId_) {}
					inline iterator_t_(iterator_t_ &&o) : obj_(o.obj_), linkId_(o.linkId_) {}
					inline iterator_t_ &operator =(const iterator_t_ &o){obj_ = o.obj_; linkId_ = o.linkId_; return *this;}
					inline iterator_t_ &operator =(iterator_t_ &&o) { obj_ = o.obj_; linkId_ = o.linkId_; return *this; }

					inline uint32_t chunk_id() const
					{
						assert_();
						return obj_->chunksLists_[linkId_].chunkId_;
					}
					inline void * data() const
					{
						return obj_->chunks_[chunk_id()].data(obj_->dataHeap_);
					}
					inline uint64_t size() const
					{
						return obj_->chunks_[chunk_id()].size();
					}

					inline uint32_t tag_id() const
					{
						return obj_->chunks_[chunk_id()].tagId();
					}
					inline const char * tag_name() const
					{						
						return obj_->tag(tag_id());
					}

					inline iterator_t_<O> append(const char * tag, uint32_t dataSize)
					{
						assert_();
						return iterator_t_<O>(obj_, obj_->append(linkId_, tag, dataSize));
					}

					inline bool valid() const { return valid_(); }
					inline iterator_t_<O> next() const { assert_(); return iterator_t_<O>(obj_, next_()); }
					inline iterator_t_<O> prev() const { assert_(); return iterator_t_<O>(obj_, prev_()); }
					inline iterator_t_<O> parent() const { assert_(); return iterator_t_<O>(obj_, parent_()); }

					inline iterator_t_<O> child_begin() const { assert_(); return iterator_t_<O>(obj_, child_first_()); }
					inline iterator_t_<O> child_end() const { assert_(); return iterator_t_<O>(obj_, NoIDX); }


					inline iterator_t_<O> &operator ++() { assert_(); linkId_ = next_(); return *this; }
					inline iterator_t_<O> &operator --() { assert_(); linkId_ = prev_(); return *this; }
					inline iterator_t_<O> operator ++(int) { assert_(); auto tmp = *this; linkId_ = next_(); return tmp; }
					inline iterator_t_<O> operator --(int) { assert_(); auto tmp = *this; linkId_ = prev_(); return tmp; }

					template<class O2> inline bool operator ==(const iterator_t_<O2> &o) const
					{
						bool v1 = valid_();
						bool v2 = o.valid_();
						if (v1 && v2)
							return (obj_ == o.obj_) && (linkId_ == o.linkId_);
						return (v1 == v2);
					}
					template<class O2> inline bool operator !=(const iterator_t_<O2> &o) const
					{
						bool v1 = valid_();
						bool v2 = o.valid_();
						if (v1 && v2)
							return (obj_ != o.obj_) || (linkId_ != o.linkId_);
						return (v1 != v2);
					}
				};

			public:
				enum class DataOwnership {UNDEF = 0, COPY, OBTAIN_OWNERSHIP, NO_OWNERSHIP};
				~zDataStorage();
				zDataStorage(uint32_t chunksCapacity = 20, uint32_t heapCapacity = 1024);
				zDataStorage(const zDataStorageDescription &descr, DataOwnership ownership = DataOwnership::COPY);
				zDataStorage(zDataStorage &&zds);
				zDataStorage(const zDataStorage &zds) = delete;
				zDataStorage &operator =(zDataStorage &&zds);
				zDataStorage &operator =(const zDataStorage &zds) = delete;
				zDataStorage clone() const;

				inline const char * tag(uint32_t id) const
				{
					if (tagNames_.empty())
						update_tag_names_();
					return tagNames_.at(id);
				}
				inline void clear() {clear_();}
				void optimize();
				zDataStorageDescription descr();
			};
		}
	}
}
