#include "ff_zds_utils.h"
#include <algorithm>
#include <unordered_map>

void zenith::util::zfile_format::zDataStorage::resize_chunks_(uint32_t newNumChunks)
{
	ZChunk16B_ZDSEntry * newChunks = new ZChunk16B_ZDSEntry[newNumChunks];
	if (chunks_)
	{
		uint32_t copySize = newNumChunks;
		if (numChunks_ < newNumChunks)
			copySize = numChunks_;
		memcpy_s(newChunks, copySize * sizeof(ZChunk16B_ZDSEntry), chunks_, copySize * sizeof(ZChunk16B_ZDSEntry));
		if (isOwner_)
			delete[] chunks_;
	}
	chunks_ = newChunks;
	capacityChunks_ = newNumChunks;
	if (numChunks_ > capacityChunks_)
		numChunks_ = capacityChunks_;
	isOwner_ = true;
}

void zenith::util::zfile_format::zDataStorage::resize_heap_(uint64_t newHeapSize)
{
	newHeapSize = ((newHeapSize >> 4) + 1) << 4; //align on 16 bytes
	uint8_t * tmp = new uint8_t[newHeapSize];
	if (dataHeap_)
	{
		uint64_t copySize = newHeapSize;
		if (dataHeapSize_ < newHeapSize)
			copySize = dataHeapSize_;
		memcpy_s(tmp, copySize, dataHeap_, copySize);
		if (isOwner_)
			delete[]dataHeap_;
	}
	dataHeap_ = tmp;
	dataHeapCapacity_ = newHeapSize;
	if (dataHeapSize_ > dataHeapCapacity_)
		dataHeapSize_ = dataHeapCapacity_;
	isOwner_ = true;
}

zenith::util::zfile_format::zDataStorage::TaskResult_ zenith::util::zfile_format::zDataStorage::add_chunk_(uint32_t parentIDX, const char * tag, uint64_t data_size, bool isAttribute, ZDSDataBaseType type, ZChunk16B_ZDSEntry ** chunkPtr, uint32_t * chunkLnk)
{
	bool isInplace = false;
	TaskResult_ res = TaskResult_::UNDEF;
	if (data_size <= 10)
		isInplace = true;
	else
	{
		if (dataHeapSize_ + data_size > dataHeapCapacity_)
			res = static_cast<TaskResult_>(res | TaskResult_::OUT_OF_MEM_DATA);
	}

	if (freeList_.empty() && numChunks_ + 1 > capacityChunks_)
		res = static_cast<TaskResult_>(res | TaskResult_::OUT_OF_MEM_CHUNKS);
	if (res != TaskResult_::UNDEF)
		return res;

	uint32_t tagId = gen_or_get_tag_id_(tag);
	uint32_t depth = 0;
	if (parentIDX != NoIDX)
		depth = chunks_[chunksLinks_[parentIDX].chunkIdx_].depth() + 1;
	ZChunk16B_ZDSEntry * ptr = get_free_entry_();
	if (data_size > 10)
	{
		*ptr = ZChunk16B_ZDSEntry::onheapEntry(dataHeapSize_, data_size, tagId, depth, isAttribute, type);
		dataHeapSize_ += data_size;
	}
	else
		*ptr = ZChunk16B_ZDSEntry::inplaceEntry(data_size, tagId, depth, isAttribute, type);
	if (chunkPtr)
		*chunkPtr = ptr;

	ChunkLink_ lnk0;
	lnk0.chunkIdx_ = static_cast<uint32_t>(ptr - chunks_);
	lnk0.parentIdx_ = NoIDX;
	lnk0.firstChildIdx_ = NoIDX;
	lnk0.lastChildIdx_ = NoIDX;
	lnk0.prevIdx_ = NoIDX;
	lnk0.nextIdx_ = NoIDX;
	uint32_t curIDX = static_cast<uint32_t>(chunksLinks_.size());
	if (chunkLnk)
		*chunkLnk = curIDX;
	chunksLinks_.push_back(lnk0);
	ChunkLink_ &lnk = chunksLinks_.back();


	if (parentIDX != NoIDX)
	{
		auto parentLink = &chunksLinks_[parentIDX];
		lnk.parentIdx_ = parentIDX;
		if (parentLink->lastChildIdx_ != NoIDX)
		{
			lnk.prevIdx_ = parentLink->lastChildIdx_;
			chunksLinks_[parentLink->lastChildIdx_].nextIdx_ = curIDX;
			parentLink->lastChildIdx_ = curIDX;
		}
		else
		{
			parentLink->lastChildIdx_ = curIDX;
			parentLink->firstChildIdx_ = curIDX;
		}
	}

	res = TaskResult_::SUCCESS;

	return res;
}

zenith::util::zfile_format::zDataStorage::TaskResult_ zenith::util::zfile_format::zDataStorage::remove_chunk_(uint32_t lnkIDX)
{
	auto lnk = &chunksLinks_[lnkIDX];

	lnk->chunkIdx_ = NoIDX; //invalidate chunk

							//invalidate all children
	if (lnk->firstChildIdx_ != NoIDX)
	{
		auto fst = lnk->firstChildIdx_;
		auto lst = lnk->lastChildIdx_;
		while (fst != lst)
		{
			remove_chunk_(fst);
			fst = chunksLinks_[fst].nextIdx_;
		}
		remove_chunk_(lst);
		lnk->firstChildIdx_ = NoIDX;
		lnk->lastChildIdx_ = NoIDX;
	}

	//rejoin neighbours:
	if (lnk->nextIdx_ != NoIDX)
	{
		ChunkLink_ * ptr = &chunksLinks_[lnk->nextIdx_];
		ptr->prevIdx_ = lnk->prevIdx_;
	}
	if (lnk->prevIdx_ != NoIDX)
	{
		ChunkLink_ * ptr = &chunksLinks_[lnk->prevIdx_];
		ptr->nextIdx_ = lnk->nextIdx_;
	}

	//update parent
	if (lnk->parentIdx_ != NoIDX)
	{
		ChunkLink_ * p = &chunksLinks_[lnk->parentIdx_];
		if (p->firstChildIdx_ == lnkIDX)
			p->firstChildIdx_ = lnk->nextIdx_;
		if (p->lastChildIdx_ == lnkIDX)
			p->lastChildIdx_ = lnk->prevIdx_;
	}

	*lnk = ChunkLink_(); //empty chunk link
	return TaskResult_::SUCCESS;
}

zenith::util::zfile_format::zDataStorage::TaskResult_ zenith::util::zfile_format::zDataStorage::reset_chunk_size_impl_(uint32_t chunkIDX, uint64_t new_size)
{
	ZChunk16B_ZDSEntry * p = &chunks_[chunkIDX];
	if (new_size > 10)
	{
		if (new_size + dataHeapSize_ > dataHeapCapacity_)
			return TaskResult_::OUT_OF_MEM_DATA;
		uint64_t offset = dataHeapSize_;
		dataHeapSize_ += new_size;
		p->reset_onheap(offset, new_size);
	}
	else
		p->reset_inplace(new_size);
	return TaskResult_::SUCCESS;
}

void zenith::util::zfile_format::zDataStorage::traverse_links_to_find_used_(uint32_t lnkIDX, std::vector<bool>& uchunks, std::vector<bool>& ulinks)
{
	if (lnkIDX == NoIDX)
		return;
	if (chunksLinks_[lnkIDX].chunkIdx_ == NoIDX)
		return;
	ulinks[lnkIDX] = true;
	uchunks[chunksLinks_[lnkIDX].chunkIdx_] = true;
	uint32_t curIDX = chunksLinks_[lnkIDX].firstChildIdx_;
	uint32_t lstIDX = chunksLinks_[lnkIDX].lastChildIdx_;
	while (curIDX != lstIDX)
	{
		traverse_links_to_find_used_(curIDX, uchunks, ulinks);
		curIDX = chunksLinks_[curIDX].nextIdx_;
	}
	traverse_links_to_find_used_(lstIDX, uchunks, ulinks);
}

zenith::util::zfile_format::zDataStorage::TaskResult_ zenith::util::zfile_format::zDataStorage::traverse_links_to_move_chunks_(uint32_t lnkIDX, uint32_t depth, uint32_t & pos)
{
	//1 move chunk
	if (pos != chunksLinks_[lnkIDX].chunkIdx_)
	{
		bool succeed = false;
		for (uint32_t i = pos + 1; i < chunksLinks_.size(); i++)
			if (chunksLinks_[i].chunkIdx_ == pos)
			{
				std::swap(chunks_[pos], chunks_[chunksLinks_[lnkIDX].chunkIdx_]);
				chunksLinks_[i].chunkIdx_ = chunksLinks_[lnkIDX].chunkIdx_;
				chunksLinks_[lnkIDX].chunkIdx_ = pos;
				succeed = true;
				break;
			}
		if (!succeed)
			return TaskResult_::CHUNK_ORDER;
	}
	chunks_[pos].reset_depth(depth);
	pos++;
	//2 move in children
	uint32_t curIDX = chunksLinks_[lnkIDX].firstChildIdx_;
	uint32_t lstIDX = chunksLinks_[lnkIDX].lastChildIdx_;
	while (curIDX != lstIDX)
	{
		auto res = traverse_links_to_move_chunks_(curIDX, depth + 1, pos);
		if (res != TaskResult_::SUCCESS)
			return res;
		curIDX = chunksLinks_[curIDX].nextIdx_;
	}
	if (lstIDX != NoIDX)
	{
		auto res = traverse_links_to_move_chunks_(lstIDX, depth + 1, pos);
		if (res != TaskResult_::SUCCESS)
			return res;
	}
	return TaskResult_::SUCCESS;
}

void zenith::util::zfile_format::zDataStorage::move_link_(uint32_t curIDX, uint32_t newIDX)
{
	chunksLinks_[newIDX] = chunksLinks_[curIDX];
	//update in children
	uint32_t fstIDX = chunksLinks_[curIDX].firstChildIdx_;
	uint32_t lstIDX = chunksLinks_[curIDX].lastChildIdx_;
	while (fstIDX != lstIDX)
	{
		chunksLinks_[fstIDX].parentIdx_ = newIDX;
		fstIDX = chunksLinks_[fstIDX].nextIdx_;
	}
	if (lstIDX != NoIDX)
		chunksLinks_[lstIDX].parentIdx_ = newIDX;
	//update in neighbours
	if (chunksLinks_[curIDX].prevIdx_ != NoIDX)
		chunksLinks_[chunksLinks_[curIDX].prevIdx_].nextIdx_ = newIDX;
	if (chunksLinks_[curIDX].nextIdx_ != NoIDX)
		chunksLinks_[chunksLinks_[curIDX].nextIdx_].prevIdx_ = newIDX;
	//update in parent
	if (chunksLinks_[curIDX].parentIdx_ != NoIDX)
	{
		auto pt = &chunksLinks_[chunksLinks_[curIDX].parentIdx_];
		if (pt->firstChildIdx_ == curIDX)
			pt->firstChildIdx_ = newIDX;
		if (pt->lastChildIdx_ == curIDX)
			pt->lastChildIdx_ = newIDX;
	}
	chunksLinks_[curIDX] = ChunkLink_();
}

zenith::util::zfile_format::zDataStorage::TaskResult_ zenith::util::zfile_format::zDataStorage::optimize_chunks_()
{
	if (chunksLinks_.size() == 0)
		return TaskResult_::SUCCESS;
	//0 preparation
	std::vector<bool> usedChunks(numChunks_, false);
	std::vector<bool> usedChunkLinks(chunksLinks_.size(), false);
	if (chunksLinks_[0].chunkIdx_ == NoIDX || chunksLinks_[0].parentIdx_ != NoIDX)
		return TaskResult_::NOROOT;

	traverse_links_to_find_used_(0, usedChunks, usedChunkLinks); //0 is always the root node, if not -- everything is broken

																 //1 move links
	uint32_t numLinks = 0, rootId = NoIDX;
	for (uint32_t i = 0, j = static_cast<uint32_t>(usedChunkLinks.size() - 1); i <= j; i++, numLinks++)
	{
		if (usedChunkLinks[i])
		{
			if (chunksLinks_[i].parentIdx_ == NoIDX)
			{
				if (rootId == NoIDX)
					rootId = i;
				else
					return TaskResult_::MULTIROOT;
			}
			continue;
		}
		while (i < j && (!usedChunkLinks[j]))
			j--;
		if (i >= j)
			break;
		move_link_(j, i);
		usedChunkLinks[i] = true;
		usedChunkLinks[j] = false;
	}
	if (rootId == NoIDX)
		return TaskResult_::NOROOT;
	if (rootId != 0)
	{
		if (numLinks + 1 > chunksLinks_.size())
			chunksLinks_.push_back(ChunkLink_());
		move_link_(0, static_cast<uint32_t>(chunksLinks_.size() - 1));
		move_link_(rootId, 0);
		move_link_(static_cast<uint32_t>(chunksLinks_.size() - 1), rootId);
	}
	chunksLinks_.resize(numLinks);

	//2 sort and move chunks and update depth
	uint32_t pos = 0;
	auto res = traverse_links_to_move_chunks_(0 /*root*/, 0/*depth*/, pos/*starting pos*/);
	if (res != TaskResult_::SUCCESS)
		return res;
	if (pos != numLinks)
		return TaskResult_::CHUNK_MISMATCH_LINKS;
	numChunks_ = numLinks;
	return TaskResult_::SUCCESS;
}

zenith::util::zfile_format::zDataStorage::TaskResult_ zenith::util::zfile_format::zDataStorage::optimize_data_()
{
	struct DataMap
	{
		uint64_t offset = 0;
		uint64_t size = 0;
		uint32_t chunkId = NoIDX;

		inline DataMap(uint64_t offset = 0, uint64_t size = 0, uint32_t id = NoIDX) : offset(offset), size(size), chunkId(id) {}

		inline bool operator <(const DataMap & oth) const { return offset < oth.offset; }
	};
	if (numChunks_ == 0)
		return TaskResult_::SUCCESS;

	std::vector<DataMap> dataMap; dataMap.reserve(numChunks_ * 2);
	//gather data map
	uint64_t fullSize = 0;
	for (uint32_t i = 0; i < numChunks_; i++)
	{
		if (chunks_[i].is_internal())
			continue;
		dataMap.emplace_back(chunks_[i].heap_offset(), chunks_[i].size(), i);
		fullSize += chunks_[i].size();
	}
	if (dataMap.empty())
		return TaskResult_::SUCCESS;
	//sort data map
	std::sort(dataMap.begin(), dataMap.end());
	//check data map
	for (uint32_t i = 0; i < dataMap.size() - 1; i++)
		if (dataMap[i].offset + dataMap[i].size > dataMap[i + 1].offset)
			return TaskResult_::HEAP_OVERLAP;

	//move/copy data -- byte by byte
	uint64_t j_off = 0, j_ind = 0;
	bool corrupted = false;
	chunks_[dataMap.front().chunkId].reset_offset(0);
	for (uint64_t i = 0; i < fullSize; i++)
	{
		uint64_t j = dataMap[j_ind].offset + j_off;
		if (i > j)
			corrupted = true;
		else if (i < j)
			dataHeap_[i] = dataHeap_[j];

		j_off++;
		if (j_off >= dataMap[j_ind].size)
		{
			j_off = 0;
			j_ind++;
			if (j_ind >= dataMap.size())
				break;
			chunks_[dataMap[j_ind].chunkId].reset_offset(i + 1);
		}
	}
	if (corrupted)
		return TaskResult_::HEAP_CORRUPTED;
	return TaskResult_::SUCCESS;
}

zenith::util::zfile_format::zDataStorage::TaskResult_ zenith::util::zfile_format::zDataStorage::optimize_tags_()
{
	std::unordered_map<uint16_t, uint16_t> usedTags; usedTags.reserve(tags_.size());
	uint16_t numUsedTags = 0;
	for (uint32_t i = 0; i < numChunks_; i++)
		if (usedTags.find(chunks_[i].tagId()) == usedTags.end())
			usedTags.insert(std::pair<uint16_t, uint16_t>(chunks_[i].tagId(), numUsedTags++));

	for (auto it = tags_.begin(); it != tags_.end(); ++it)
	{
		auto iter = usedTags.find(it.value());
		if (iter != usedTags.end())
			it.value() = iter->second;
		else
			it.value() = NoTAG;
	}
	maxTagId_ = numUsedTags;
	tagNames_.clear();
	return TaskResult_::SUCCESS;
}

zenith::util::zfile_format::zDataStorage::TaskResult_ zenith::util::zfile_format::zDataStorage::optimize_()
{
	TaskResult_ res = TaskResult_::UNDEF;
	res = optimize_chunks_();
	if (res != TaskResult_::SUCCESS)
		return res;

	res = optimize_data_();
	if (res != TaskResult_::SUCCESS)
		return res;

	res = optimize_tags_();
	if (res != TaskResult_::SUCCESS)
		return res;

	return TaskResult_::SUCCESS;
}

void zenith::util::zfile_format::zDataStorage::clear_()
{
	freeList_.clear();
	chunksLinks_.clear();
	if (chunks_ && isOwner_)
		delete[] chunks_;
	chunks_ = nullptr;
	numChunks_ = capacityChunks_ = 0;
	if (dataHeap_ && isOwner_)
		delete[] dataHeap_;
	dataHeap_ = nullptr;
	dataHeapSize_ = dataHeapCapacity_ = 0;
	
	zenith::util::hdict<uint32_t> tmp;
	tags_ = std::move(tmp);
	maxTagId_ = 0;

	if (tmpTagHeap_)
		delete[] tmpTagHeap_;
	tmpTagHeap_ = nullptr;
}

void zenith::util::zfile_format::zDataStorage::update_tag_names_() const
{
	tagNames_.clear(); tagNames_.resize(tags_.size());
	for (auto it = tags_.begin(); it != tags_.end(); ++it)
	{
		//auto tmp_v = it.value(); //tag-id
		//auto tmp_k = it.key(); //tag-name
		tagNames_[it.value()] = it.key();
	}
}

void zenith::util::zfile_format::zDataStorage::update_tag_heap_()
{
	if (tmpTagHeap_)
		delete[] tmpTagHeap_;
	tmpTagHeap_ = nullptr;

	uint32_t totalLength = 0;
	std::vector<const char *> names_(tags_.size(), nullptr);
	std::vector<uint32_t> sizes_(tags_.size(), 0);
	std::vector<uint32_t> offsets_(tags_.size(), 0);
	for (auto it = tags_.begin(); it != tags_.end(); ++it)
	{
		sizes_[it.value()] = static_cast<uint32_t>(std::strlen(it.key()) + 1);
		names_[it.value()] = it.key();
	}
	for (uint32_t i = 0; i < sizes_.size(); i++)
	{
		offsets_[i] = totalLength;
		totalLength += sizes_[i];
	}
	uint32_t tblSize = 4 * static_cast<uint32_t>(sizes_.size());

	tmpTagHeapSize_ = tblSize + totalLength;
	tmpTagHeap_ = new uint8_t[tmpTagHeapSize_];

	uint32_t * tmpTagTable = reinterpret_cast<uint32_t*>(tmpTagHeap_);
	for (uint32_t i = 0; i < sizes_.size(); i++)
	{
		uint32_t off = tblSize + offsets_[i];
		tmpTagTable[i] = off;
		memcpy_s(tmpTagHeap_ + off, sizes_[i], names_[i], sizes_[i]);
	}
}

zenith::util::zfile_format::zDataStorage::~zDataStorage()
{
	clear_();
}

zenith::util::zfile_format::zDataStorage::zDataStorage(const char * root_tag, uint32_t root_size, uint32_t chunksCapacity, uint64_t heapCapacity)
{
	if (root_size > heapCapacity)
		heapCapacity = root_size;
	if (chunksCapacity == 0)
		chunksCapacity = 1;

	tmpTagHeap_ = nullptr;
	capacityChunks_ = chunksCapacity;
	numChunks_ = 0;
	chunks_ = new ZChunk16B_ZDSEntry[chunksCapacity];
	chunksLinks_.reserve(chunksCapacity);

	dataHeap_ = new uint8_t[heapCapacity];
	dataHeapCapacity_ = heapCapacity;
	dataHeapSize_ = 0;

	isOwner_ = true;
	maxTagId_ = 0;

	append_(NoIDX, root_tag, root_size);
}

zenith::util::zfile_format::zDataStorage::zDataStorage(const zDataStorageDescription & descr, DataOwnership ownership)
{
	tmpTagHeap_ = nullptr;

	numChunks_ = capacityChunks_ = descr.numChunks;
	dataHeapSize_ = dataHeapCapacity_ = descr.dataHeapSize;

	switch (ownership)
	{
	case DataOwnership::UNDEF: throw ZDataStructureException("zDataStorage ownership option is not specified.");
	case DataOwnership::COPY:
		{
			chunks_ = new ZChunk16B_ZDSEntry[numChunks_];
			memcpy_s(chunks_, numChunks_ * sizeof(ZChunk16B_ZDSEntry), descr.chunks, numChunks_ * sizeof(ZChunk16B_ZDSEntry));
			dataHeap_ = new uint8_t[dataHeapSize_];
			memcpy_s(dataHeap_, dataHeapSize_, descr.dataHeap, dataHeapSize_);
			isOwner_ = true;
			break;
		}
	case DataOwnership::OBTAIN_OWNERSHIP:
		{
			chunks_ = descr.chunks;
			dataHeap_ = descr.dataHeap;
			isOwner_ = true;
			break;
		}
	case DataOwnership::NO_OWNERSHIP:
		{
			chunks_ = descr.chunks;
			dataHeap_ = descr.dataHeap;
			isOwner_ = false;
			break;
		}
	default: throw ZDataStructureException("zDataStorage ownership option is not specified.");
	}

	uint32_t * tagTable = reinterpret_cast<uint32_t *>(descr.tagHeap);
	const char * tagHeap = reinterpret_cast<char *>(descr.tagHeap);// +4 * uint32_t(descr.numTags));
	for (uint32_t i = 0; i < descr.numTags; i++)
	{
		uint32_t off = tagTable[i];
		const char * name = &tagHeap[off];
		tags_.insert(std::pair<const char *, uint32_t>(name, i));
	}
	maxTagId_ = descr.numTags;

	chunksLinks_.reserve(descr.numChunks);
	if (descr.numChunks == 0)
		return;

	std::vector<uint32_t> parentStack_(1, 0);
	uint32_t curDepth = chunks_[0].depth();
	ChunkLink_ r;
	r.chunkIdx_ = 0;
	r.parentIdx_ = NoIDX;
	chunksLinks_.push_back(r);

	for (uint32_t i = 1; i < descr.numChunks; i++)
	{
		chunksLinks_.push_back(ChunkLink_());
		ChunkLink_ &n = chunksLinks_.back();
		n.chunkIdx_ = i;
		if (chunks_[i].depth() > curDepth)
		{
			if(chunks_[i].depth() != curDepth + 1)
				throw ZDataStructureException("zDataStorage: chunks depths are incorrect.");
			parentStack_.push_back(i - 1);
			curDepth = chunks_[i].depth();
			chunksLinks_[i - 1].firstChildIdx_ = i;
			chunksLinks_[i - 1].lastChildIdx_ = i;
			chunksLinks_[i].parentIdx_ = i - 1;
			continue;
		}
		if (chunks_[i].depth() == curDepth)
		{
			chunksLinks_[i].prevIdx_ = i - 1;
			chunksLinks_[i].parentIdx_ = chunksLinks_[i - 1].parentIdx_;
			chunksLinks_[i - 1].nextIdx_ = i;
			chunksLinks_[chunksLinks_[i - 1].parentIdx_].lastChildIdx_ = i;
			continue;
		}
		if (chunks_[i].depth() < curDepth)
		{
			uint32_t delta = curDepth - chunks_[i].depth();
			curDepth = chunks_[i].depth();
			for (uint32_t j = 0; j < delta; j++)
				parentStack_.pop_back();
			if (parentStack_.empty())
				throw ZDataStructureException("zDataStorage: failed to create class, due to multiple root elements.");
			uint32_t p = parentStack_.back();
			chunksLinks_[i].parentIdx_ = p;
			if (chunksLinks_[p].lastChildIdx_ == NoIDX)
			{
				chunksLinks_[p].firstChildIdx_ = i;
				chunksLinks_[p].lastChildIdx_ = i;
			}
			else
			{
				uint32_t pp = chunksLinks_[p].lastChildIdx_;
				chunksLinks_[p].lastChildIdx_ = i;
				chunksLinks_[i].prevIdx_ = pp;
				chunksLinks_[pp].nextIdx_ = i;
			}
		}
	}
}

zenith::util::zfile_format::zDataStorage::zDataStorage(zDataStorage && zds)
	: chunksLinks_(std::move(zds.chunksLinks_)), tags_(std::move(zds.tags_)), freeList_(std::move(zds.freeList_))
{
	tmpTagHeap_ = nullptr;
	capacityChunks_ = zds.capacityChunks_; zds.capacityChunks_ = 0;
	numChunks_ = zds.numChunks_; zds.numChunks_ = 0;
	chunks_ = zds.chunks_; zds.chunks_ = nullptr;

	dataHeap_ = zds.dataHeap_; zds.dataHeap_ = nullptr;
	dataHeapCapacity_ = zds.dataHeapCapacity_; zds.dataHeapCapacity_ = 0;
	dataHeapSize_ = zds.dataHeapSize_; zds.dataHeapSize_ = 0;

	maxTagId_ = zds.maxTagId_; zds.maxTagId_ = 0;
	isOwner_ = zds.isOwner_;
}

zenith::util::zfile_format::zDataStorage & zenith::util::zfile_format::zDataStorage::operator=(zDataStorage && zds)
{
	clear_();

	chunksLinks_ = std::move(zds.chunksLinks_);
	tags_ = std::move(zds.tags_);
	freeList_ = std::move(zds.freeList_);

	capacityChunks_ = zds.capacityChunks_; zds.capacityChunks_ = 0;
	numChunks_ = zds.numChunks_; zds.numChunks_ = 0;
	chunks_ = zds.chunks_; zds.chunks_ = nullptr;

	dataHeap_ = zds.dataHeap_; zds.dataHeap_ = nullptr;
	dataHeapCapacity_ = zds.dataHeapCapacity_; zds.dataHeapCapacity_ = 0;
	dataHeapSize_ = zds.dataHeapSize_; zds.dataHeapSize_ = 0;

	maxTagId_ = zds.maxTagId_; zds.maxTagId_ = 0;
	isOwner_ = zds.isOwner_;
	return *this;
}

zenith::util::zfile_format::zDataStorage zenith::util::zfile_format::zDataStorage::clone() const
{
	zDataStorage res(root().tag_name(), static_cast<uint32_t>(root().size()), capacityChunks_, dataHeapCapacity_);

	res.chunksLinks_ = chunksLinks_;
	memcpy_s(res.chunks_, res.capacityChunks_ * sizeof(ZChunk16B_ZDSEntry), chunks_, capacityChunks_ * sizeof(ZChunk16B_ZDSEntry));
	res.maxTagId_ = maxTagId_;
	res.tags_ = tags_;

	memcpy_s(res.dataHeap_, res.dataHeapCapacity_, dataHeap_, dataHeapCapacity_);

	res.dataHeapSize_ = dataHeapSize_;
	res.numChunks_ = numChunks_;
	return res;
}

void zenith::util::zfile_format::zDataStorage::optimize()
{
	TaskResult_ res = optimize_();
	if (res == TaskResult_::SUCCESS)
		return;
	switch (res)
	{
	case OUT_OF_MEM_DATA: throw ZDataStructureException("zDataStorage::optimize: out of memory for data heap");
	case OUT_OF_MEM_CHUNKS: throw ZDataStructureException("zDataStorage::optimize: out of memory for chunks");
	case MULTIROOT: throw ZDataStructureException("zDataStorage::optimize: multiple root nodes");
	case NOROOT: throw ZDataStructureException("zDataStorage::optimize: no root node");
	case CHUNK_ORDER: throw ZDataStructureException("zDataStorage::optimize: order of chunks is incorrect");
	case CHUNK_MISMATCH_LINKS: throw ZDataStructureException("zDataStorage::optimize: chunk data does not match link data");
	case HEAP_OVERLAP: throw ZDataStructureException("zDataStorage::optimize: data heap overlapping");
	case HEAP_CORRUPTED: throw ZDataStructureException("zDataStorage::optimize: data heap corrupted");
	default: throw ZDataStructureException("zDataStorage::optimize: unknown fault");
	}
}

zenith::util::zfile_format::zDataStorageDescription zenith::util::zfile_format::zDataStorage::descr()
{
	zDataStorageDescription res;

	optimize();
	update_tag_heap_();

	res.chunks = chunks_;
	res.numChunks = numChunks_;
	res.tagHeap = tmpTagHeap_;
	res.tagHeapSize = tmpTagHeapSize_;
	res.numTags = tags_.size();
	res.dataHeap = dataHeap_;
	res.dataHeapSize = dataHeapSize_;

	return res;
}
