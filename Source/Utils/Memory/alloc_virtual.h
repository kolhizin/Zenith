#pragma once
#include "alloc_base.h"

namespace zenith
{
	namespace util
	{
		namespace memory
		{
			template<class Base, size_t MinSize, size_t MaxSize, size_t NumBlocks>
			class MemAllocVirtual_StaticSize : public Base
			{
				static const NumServiceBytes = (NumBlocks >> 3) + (NumBlocks & 7 > 0 ? 1 : 0);
				uint8_t used_[NumServiceBytes];
				inline uint8_t getbit_(size_t index) const
				{
					uint8_t tmp = used_[index >> 3];
					return (tmp >> (index & 7)) & 1;
				}
				inline void setbit_(size_t index, uint8_t val)
				{
					uint8_t &tmp = used_[index >> 3];
					tmp = (tmp & (~(1 << (index & 7)))) | (val << (index & 7));
				}
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;
				static const MemAllocMemory memory = MemAllocMemory::VIRTUAL;
				MemAllocVirtual_StaticSize()
				{
					for (size_t i = 0; i < (NumBlocks >> 3); i++)
						used_[i] = 0;
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size > MaxSize)
						throw MemAlloc_alloc_exception("Too large size for MemAllocVirtual_StaticSize!");
					if (size < MinSize)
						throw MemAlloc_alloc_exception("Too small size for MemAllocVirtual_StaticSize!");
					for (size_t i = 0; i < NumBlocks; i++)
						if (getbit_(i) == 0)
						{
							setbit_(i, 1);
							MemAllocBlock blk = return_blk(reinterpret_cast<void *>(MaxSize * i), MaxSize);
							return blk;
						}
					return error_cannot_alloc(return_blk(nullptr, 0));
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (!owns(blk))
						Base::error_own();
					size_t start = reinterpret_cast<size_t>(blk.ptr) / MaxSize;
					setbit_(start, 0);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size > MaxSize)
						throw MemAlloc_alloc_exception("Too large size for MemAllocVirtual_StaticSize!");
					if (size < MinSize)
						throw MemAlloc_alloc_exception("Too small size for MemAllocVirtual_StaticSize!");
					for (size_t i = 0; i < NumBlocks; i++)
						if (getbit_(i) == 0)
						{
							setbit_(i, 1);
							MemAllocBlock blk = return_blk(reinterpret_cast<void *>(MaxSize * i), MaxSize);
							return blk.ptr;
						}
					return error_cannot_alloc(return_blk(nullptr, 0)).ptr;
				}
				inline void free(void * ptr)
				{
					size_t start = reinterpret_cast<size_t>(ptr) / MaxSize;
					setbit_(start, 0);
				}
			};

			template<class Base, size_t BlockSize, size_t NumBlocks>
			class MemAllocVirtual_BitmapStatic : public Base
			{
				static const NumServiceBytes = (NumBlocks >> 3) + (NumBlocks & 7 > 0 ? 1 : 0);
				uint8_t used_[NumServiceBytes];

				inline uint8_t getbit_(size_t index) const
				{
					uint8_t tmp = used_[index >> 3];
					return (tmp >> (index & 7)) & 1;
				}
				inline void setbit_(size_t index, uint8_t val)
				{
					uint8_t &tmp = used_[index >> 3];
					tmp = (tmp & (~(1 << (index & 7)))) | (val << (index & 7));
				}
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;
				static const MemAllocMemory memory = MemAllocMemory::VIRTUAL;
				MemAllocVirtual_BitmapStatic()
				{
					for (size_t i = 0; i < (NumBlocks >> 3); i++)
						used_[i] = 0;
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					size_t numBlock = size / BlockSize;
					if (numBlock * BlockSize < size)
						numBlock++;
					size_t freeBlocks = 0;
					size_t i = 0;
					for (; i < NumBlocks; i++)
						if (getbit_(i))
							freeBlocks = 0;
						else
						{
							freeBlocks++;
							if (freeBlocks >= numBlock)
								break;
						}
					if (freeBlocks < numBlock)
						return error_cannot_alloc(return_blk(nullptr, 0));

					size_t start = i - (numBlock - 1);
					for (size_t j = 0; j < numBlock; j++)
						setbit_(start+j, 1);

					MemAllocBlock blk = return_blk(reinterpret_cast<void *>(BlockSize * start), BlockSize * numBlock);
					return blk;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (!owns(blk))
						Base::error_own();
					size_t start = reinterpret_cast<size_t>(blk.ptr) / BlockSize;
					size_t numBlk = blk.size / BlockSize;
					for (size_t i = 0; i < numBlk; i++)
						setbit_(start+i, 0);
				}
			};
			template<class Base, class StatelessAlloc>
			class MemAllocVirtual_BitmapDynamic : public Base
			{
				size_t blockSize_;
				size_t numBlocks_;
				MemAllocBlock serviceBlk_;
				uint8_t * used_;

				static_assert(StatelessAlloc::params == MemAllocParam::STATELESS, "Awaiting stateless allocator!");

				inline uint8_t getbit_(size_t index) const
				{
					uint8_t tmp = used_[index >> 3];
					return (tmp >> (index & 7)) & 1;
				}
				inline void setbit_(size_t index, uint8_t val)
				{
					uint8_t &tmp = used_[index >> 3];
					tmp = (tmp & (~(1 << (index & 7)))) | (val << (index & 7));
				}
				MemAllocVirtual_BitmapDynamic();
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETRIC;
				static const MemAllocMemory memory = MemAllocMemory::VIRTUAL;

				MemAllocVirtual_BitmapDynamic(const MemAllocInfo * mai)
				{
					blockSize_ = mai->allocMinSize;
					numBlocks_ = mai->allocNumBlock;
					serviceBlk_ = StatelessAlloc::allocate((numBlocks_ >> 3) + (numBlocks_ & 7 > 0 ? 1 : 0));
					used_ = reinterpret_cast<uint8_t *>(serviceBlk_.ptr);

					for (size_t i = 0; i < (numBlocks_ >> 3); i++)
						used_[i] = 0;
				}
				~MemAllocVirtual_BitmapDynamic()
				{
					StatelessAlloc::deallocate(serviceBlk_);
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					size_t numBlock = size / blockSize_;
					if (numBlock * blockSize_ < size)
						numBlock++;
					size_t freeBlocks = 0;
					size_t i = 0;
					for (; i < numBlocks_; i++)
						if (getbit_(i))
							freeBlocks = 0;
						else
						{
							freeBlocks++;
							if (freeBlocks >= numBlock)
								break;
						}
					if (freeBlocks < numBlock)
						return error_cannot_alloc(return_blk(nullptr, 0));

					size_t start = i - (numBlock - 1);
					for (size_t j = 0; j < numBlock; j++)
						setbit_(start + j, 1);

					MemAllocBlock blk = return_blk(reinterpret_cast<void *>(blockSize_ * start), blockSize_ * numBlock);
					return blk;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (!owns(blk))
						Base::error_own();
					size_t start = reinterpret_cast<size_t>(blk.ptr) / blockSize_;
					size_t numBlk = blk.size / blockSize_;
					for (size_t i = 0; i < numBlk; i++)
						setbit_(start + i, 0);
				}
			};

			template<class Base>
			class MemAllocVirtual_BitmapDynamicExt : public Base
			{
				size_t blockSize_;
				size_t numBlocks_;
				MemAllocBlock serviceBlk_;
				uint8_t * used_;

				void * allocPtr_;
				MemAllocInfo::PFN_Allocate pfnAllocate_;
				MemAllocInfo::PFN_Deallocate pfnDeallocate_;
				
				inline uint8_t getbit_(size_t index) const
				{
					uint8_t tmp = used_[index >> 3];
					return (tmp >> (index & 7)) & 1;
				}
				inline void setbit_(size_t index, uint8_t val)
				{
					uint8_t &tmp = used_[index >> 3];
					tmp = (tmp & (~(1 << (index & 7)))) | (val << (index & 7));
				}
				MemAllocVirtual_BitmapDynamicExt();
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETRIC;
				static const MemAllocMemory memory = MemAllocMemory::VIRTUAL;

				MemAllocVirtual_BitmapDynamicExt(const MemAllocInfo * mai)
				{
					allocPtr_ = mai->allocPtr;
					pfnAllocate_ = mai->pfnAllocate;
					pfnDeallocate_ = mai->pfnDeallocate;

					blockSize_ = mai->allocMinSize;
					numBlocks_ = mai->allocNumBlock;
					serviceBlk_ = pfnAllocate_(allocPtr_, (numBlocks_ >> 3) + (numBlocks_ & 7 > 0 ? 1 : 0), MemAllocHint::UNDEF);
					used_ = reinterpret_cast<uint8_t *>(serviceBlk_.ptr);

					for (size_t i = 0; i < (numBlocks_ >> 3); i++)
						used_[i] = 0;
				}
				~MemAllocVirtual_BitmapDynamicExt()
				{
					pfnDeallocate_(allocPtr_, serviceBlk_);
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					size_t numBlock = size / blockSize_;
					if (numBlock * blockSize_ < size)
						numBlock++;
					size_t freeBlocks = 0;
					size_t i = 0;
					for (; i < numBlocks_; i++)
						if (getbit_(i))
							freeBlocks = 0;
						else
						{
							freeBlocks++;
							if (freeBlocks >= numBlock)
								break;
						}
					if (freeBlocks < numBlock)
						return error_cannot_alloc(return_blk(nullptr, 0));

					size_t start = i - (numBlock - 1);
					for (size_t j = 0; j < numBlock; j++)
						setbit_(start + j, 1);

					MemAllocBlock blk = return_blk(reinterpret_cast<void *>(blockSize_ * start), blockSize_ * numBlock);
					return blk;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (!owns(blk))
						Base::error_own();
					size_t start = reinterpret_cast<size_t>(blk.ptr) / blockSize_;
					size_t numBlk = blk.size / blockSize_;
					for (size_t i = 0; i < numBlk; i++)
						setbit_(start + i, 0);
				}
			};



			template<class Base>
			class MemAllocVirtual_ListAlloc : public Base
			{
				size_t chunkSize_;
				struct BlockInfo_
				{
					BlockInfo_ *pNext_ = nullptr, *pPrev_ = nullptr;
					size_t blockSize_ = 0;
					MemAllocBlock nextBlkAlloc_;
					bool used = false;
				};

				MemAllocBlock rootBlkAlloc_;
				BlockInfo_ * rootBlock_, * lastBlock_;

				void * allocPtr_;
				MemAllocInfo::PFN_Allocate pfnAllocate_;
				MemAllocInfo::PFN_Deallocate pfnDeallocate_;

				
				std::pair<BlockInfo_ *, size_t> findBestBlock_(size_t sz)
				{
					BlockInfo_ * ptr = rootBlock_, *pBest = nullptr;
					size_t curOffset = 0, bestOffset = 0;
					while (ptr)
					{
						if (!ptr->used && ptr->blockSize_ >= sz)
						{
							if (pBest)
							{
								if (ptr->blockSize_ < pBest->blockSize_)
								{
									pBest = ptr;
									bestOffset = curOffset;
								}
							}
							else
							{
								pBest = ptr;
								bestOffset = curOffset;
							}
						}
						curOffset += ptr->blockSize_;
						ptr = ptr->pNext_;						
					}
					return std::make_pair(pBest, bestOffset);
				}
				BlockInfo_ * findOffBlock_(size_t off)
				{
					BlockInfo_ * ptr = rootBlock_;
					size_t curOffset = 0;
					while (ptr)
					{
						if (off == curOffset)
							return ptr;
						curOffset += ptr->blockSize_;
						ptr = ptr->pNext_;
					}
					return nullptr;
				}
				MemAllocVirtual_ListAlloc();
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETRIC;
				static const MemAllocMemory memory = MemAllocMemory::VIRTUAL;

				MemAllocVirtual_ListAlloc(const MemAllocInfo * mai)
				{
					chunkSize_ = mai->allocMinSize;

					pfnAllocate_ = mai->pfnAllocate;
					pfnDeallocate_ = mai->pfnDeallocate;
					allocPtr_ = mai->allocPtr;

					rootBlkAlloc_ = pfnAllocate_(allocPtr_, sizeof(BlockInfo_), MemAllocHint::UNDEF);
					rootBlock_ = static_cast<BlockInfo_*>(rootBlkAlloc_.ptr);
					rootBlock_->blockSize_ = mai->allocMaxSize;
					rootBlock_->used = false;
					rootBlock_->pNext_ = nullptr;
					rootBlock_->pPrev_ = nullptr;
					lastBlock_ = rootBlock_;
				}
				~MemAllocVirtual_ListAlloc()
				{
					BlockInfo_ * ptr = lastBlock_->pPrev_;
					while (ptr)
					{
						BlockInfo_ * p = ptr->pPrev_;
						pfnDeallocate_(allocPtr_, ptr->nextBlkAlloc_);
						ptr = p;
					}
					pfnDeallocate_(allocPtr_, rootBlkAlloc_);
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					size_t rem = size % chunkSize_;
					size_t actSize = size + (rem > 0 ? chunkSize_ - rem : 0);
					std::pair<BlockInfo_ *, size_t> po = findBestBlock_(actSize);
					BlockInfo_ * p = po.first;
					if (!p)
						return error_cannot_alloc(return_blk(nullptr, 0));

					if (p->blockSize_ == actSize)
					{
						p->used = true;
						return return_blk(reinterpret_cast<void *>(po.second), actSize);
					}

					MemAllocBlock blk = pfnAllocate_(allocPtr_, sizeof(BlockInfo_), MemAllocHint::UNDEF);
					if(blk.size < sizeof(BlockInfo_) || blk.ptr == nullptr)
						return error_cannot_alloc(return_blk(nullptr, 0));

					BlockInfo_ * pNew = static_cast<BlockInfo_ *>(blk.ptr);
					pNew->pPrev_ = p;
					pNew->pNext_ = p->pNext_;
					p->pNext_ = pNew;

					if (pNew->pNext_)
						pNew->pNext_->pPrev_ = pNew;
					else
						lastBlock_ = pNew;

					pNew->nextBlkAlloc_ = p->nextBlkAlloc_;
					p->nextBlkAlloc_ = blk;

					pNew->used = false;
					pNew->blockSize_ = p->blockSize_ - actSize;

					p->used = true;
					p->blockSize_ = actSize;

					return return_blk(reinterpret_cast<void *>(po.second), actSize);
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (!owns(blk))
						Base::error_own();
					BlockInfo_ * p = findOffBlock_(reinterpret_cast<size_t>(blk.ptr));
					if(!p)
						Base::error_own();
					p->used = false;
					
					if (p->pNext_ && !p->pNext_->used)
					{
						MemAllocBlock blk = p->nextBlkAlloc_;

						p->nextBlkAlloc_ = p->pNext_->nextBlkAlloc_;

						if (!p->pNext_->pNext_)
							lastBlock_ = p;
						else
							p->pNext_->pNext_->pPrev_ = p;

						p->blockSize_ += p->pNext_->blockSize_;
						p->pNext_ = p->pNext_->pNext_;

						pfnDeallocate_(allocPtr_, blk);
					}
					if (p->pPrev_ && !p->pPrev_->used)
					{
						p = p->pPrev_;

						MemAllocBlock blk = p->nextBlkAlloc_;

						p->nextBlkAlloc_ = p->pNext_->nextBlkAlloc_;

						if (!p->pNext_->pNext_)
							lastBlock_ = p;
						else
							p->pNext_->pNext_->pPrev_ = p;

						p->blockSize_ += p->pNext_->blockSize_;
						p->pNext_ = p->pNext_->pNext_;

						pfnDeallocate_(allocPtr_, blk);
					}
				}
			};


			/*Linear allocator*/
			template<class Base, size_t PoolSize>
			class MemAllocVirtual_LinearStatic : public Base
			{
				size_t top_;
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;
				static const MemAllocMemory memory = MemAllocMemory::VIRTUAL;
				MemAllocVirtual_LinearStatic() : top_(0)
				{
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (top_ + size > PoolSize)
						return_blk(nullptr, 0);
					auto blk = return_blk(reinterpret_cast<void *>(top_), size);
					top_ += size;
					return blk;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (!owns(blk))
						Base::error_own();
					if (reinterpret_cast<size_t>(blk.ptr) + blk.size == top_)
						top_ -= blk.size;
				}
				inline size_t top() const { return top_; }
				inline void freeAtTop(size_t size = 1) { if (top_ < size)top_ = 0;else top_ -= size; }
				inline void freeUntil(size_t pos = 0) { if (top_ > pos)top_ = pos;}
			};
		}
	}
}
