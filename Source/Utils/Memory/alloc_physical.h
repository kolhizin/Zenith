#pragma once
#include "alloc_base.h"

namespace zenith
{
	namespace util
	{
		namespace memory
		{
			template<class Base>
			class MemAlloc_NULL : public Base
			{
			public:
				static const MemAllocMemory memory = MemAllocMemory::PHYSICAL;
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					return return_blk(nullptr, 0);
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (owns(blk) || blk.ptr == nullptr)
						return;
					Base::error_own();
				}
			};
			template<class Base>
			class MemAlloc_MAlloc : public Base
			{
			public:
				static const MemAllocMemory memory = MemAllocMemory::PHYSICAL;
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock blk = return_blk(malloc(size), size);
					if (!blk.ptr)
						return error_cannot_alloc(blk);
					return blk;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (owns(blk))
						free(blk.ptr);
					else
						Base::error_own();
				}
			};

			template<class VirtualAlloc, class MemPool, MemAllocParam p>
			class MemAlloc_Combine_;

			

			template<class VirtualAlloc, class MemPool>
			class MemAlloc_Combine_<VirtualAlloc, MemPool, MemAllocParam::PARAMETRIC> : public VirtualAlloc, public MemPool
			{
				MemAlloc_Combine_();
			public:
				MemAlloc_Combine_(const MemAllocInfo * mai) : VirtualAlloc(mai), MemPool(mai){}
				static const MemAllocMemory memory = MemAllocMemory::PHYSICAL;
				static const MemAllocParam params = MemAllocParam::PARAMETRIC;
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock blk = VirtualAlloc::allocate(size, hint);
					if (blk.size <= 0)
						return error_cannot_alloc(blk);
					blk.ptr = reinterpret_cast<uint64_t>(blk.ptr) + static_cast<uint8_t *>(MemPool::begin());
					return blk;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					MemAllocBlock b = blk;
					b.ptr = reinterpret_cast<void *>(static_cast<uint8_t *>(b.ptr) - static_cast<uint8_t *>(MemPool::begin()));
					if (owns(b))
						VirtualAlloc::deallocate(b);
					else
						error_own();
				}
				inline bool owns(void * ptr) const { return (ptr >= MemPool::begin()) && (ptr < MemPool:end()); }
				inline bool owns(const MemAllocBlock &blk) const { return VirtualAlloc::owns(blk); }
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					void * ptr = VirtualAlloc::alloc(size, hint);
					ptr = reinterpret_cast<uint64_t>(ptr) + static_cast<uint8_t *>(MemPool::begin());
					return ptr;
				}
				inline void free(void * ptr)
				{
					ptr = reinterpret_cast<void *>(static_cast<uint8_t *>(ptr) - static_cast<uint8_t *>(MemPool::begin()));
					VirtualAlloc::free(ptr);
				}
			};
			template<class VirtualAlloc, class MemPool>
			class MemAlloc_Combine_<VirtualAlloc, MemPool, MemAllocParam::PARAMETERLESS> : public VirtualAlloc, public MemPool
			{				
			public:
				static const MemAllocMemory memory = MemAllocMemory::PHYSICAL;
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock blk = VirtualAlloc::allocate(size, hint);
					if (blk.size <= 0)
						return error_cannot_alloc(blk);
					blk.ptr = reinterpret_cast<uint64_t>(blk.ptr) + static_cast<uint8_t *>(MemPool::begin());
					return blk;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					MemAllocBlock b = blk;
					b.ptr = reinterpret_cast<void *>(static_cast<uint8_t *>(b.ptr) - static_cast<uint8_t *>(MemPool::begin()));
					if (owns(b))
						VirtualAlloc::deallocate(b);
					else
						error_own();
				}
				inline bool owns(void * ptr) const { return (ptr >= begin()) && (ptr < end()); }
				inline bool owns(const MemAllocBlock &blk) const { return VirtualAlloc::owns(blk); }
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					void * ptr = VirtualAlloc::alloc(size, hint);
					ptr = reinterpret_cast<uint64_t>(ptr) + static_cast<uint8_t *>(MemPool::begin());
					return ptr;
				}
				inline void free(void * ptr)
				{
					ptr = reinterpret_cast<void *>(static_cast<uint8_t *>(ptr) - static_cast<uint8_t *>(MemPool::begin()));
					VirtualAlloc::free(ptr);
				}
			};

			template <class VirtualAlloc, class MemPool>
			using MemAlloc_Combine = MemAlloc_Combine_<VirtualAlloc, MemPool, MemAlloc_Combine_Param_<VirtualAlloc::params, MemPool::params>::result>;

		}
	}
}