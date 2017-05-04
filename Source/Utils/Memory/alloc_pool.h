#pragma once
#include <memory>
#include "alloc_base.h"

namespace zenith
{
	namespace util
	{
		namespace memory
		{

			template<size_t Size, size_t Align = 1>
			class MemAllocPool_Stack : public MemAllocPool_BASE
			{
				mutable alignas(Align)uint8_t stack_[Size];
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;

				static const size_t Size = Size;
				static const size_t Align = Align;
				static const bool StaticSize = true;
				static const bool StaticAlign = true;

				inline void * begin() const
				{
					return &stack_[0];
				}
				inline void * end() const
				{
					return (&stack_[0] + Size);
				}
				inline size_t size() const
				{
					return Size;
				}
				inline size_t align() const
				{
					return Align;
				}
			};

			class MemAllocPool_DynamicVirtual : public MemAllocPool_BASE
			{
				void * offset_;
				size_t size_, align_;
			public:
				MemAllocPool_DynamicVirtual(const MemAllocInfo * mai) : offset_(mai->poolOffset), size_(mai->poolSize), align_(mai->poolAlign) {}
				static const MemAllocParam params = MemAllocParam::PARAMETRIC;

				static const bool StaticSize = false;
				static const bool StaticAlign = false;

				inline void * begin() const
				{
					return offset_;
				}
				inline void * end() const
				{
					return reinterpret_cast<uint8_t *>(offset_) + size_;
				}
				inline size_t size() const
				{
					return size_;
				}
				inline size_t align() const
				{
					return align_;
				}
			};

			template<size_t Size, size_t Align = 1>
			class MemAllocPool_StaticHeap : public MemAllocPool_BASE
			{
				mutable uint8_t * stack_, * ptr0_;
			public:
				MemAllocPool_StaticHeap()
				{
					ptr0_ = static_cast<uint8_t *>(malloc(Size + Align - 1));
					
					void * ptr = ptr0_;
					auto space = Size + Align - 1;
					stack_ = static_cast<uint8_t *>(std::align(Align, Size, ptr, space));
				}
				~MemAllocPool_StaticHeap()
				{
					free(ptr0_);
				}
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;

				static const size_t Size = Size;
				static const size_t Align = Align;
				static const bool StaticSize = true;
				static const bool StaticAlign = true;



				inline void * begin() const
				{
					return stack_;
				}
				inline void * end() const
				{
					return (stack_ + Size);
				}
				inline size_t size() const
				{
					return Size;
				}
				inline size_t align() const
				{
					return Align;
				}
			};

			template<size_t Align = 1>
			class MemAllocPool_MAlloc : public MemAllocPool_BASE
			{
				mutable uint8_t * start_, *ptr_;
				size_t size_;

				MemAllocPool_MAlloc(const MemAllocPool_MAlloc<Align> &);
				MemAllocPool_MAlloc(MemAllocPool_MAlloc<Align> &&);
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETRIC;

				static const size_t Size = 0;
				static const size_t Align = Align;
				static const bool StaticSize = false;
				static const bool StaticAlign = true;

				MemAllocPool_MAlloc(const MemAllocInfo * mai) : size_(mai->poolSize)
				{
					ptr_ = static_cast<uint8_t *>(malloc(size_ + Align - 1));
					//if(!ptr_)
					//	throw std::
					void * ptr = ptr_;
					auto space = size_ + Align - 1;
					start_ = static_cast<uint8_t *>(std::align(Align, size_, ptr, space));
				}
				~MemAllocPool_MAlloc()
				{
					free(ptr_);
				}
				inline void * begin() const
				{
					return start_;
				}
				inline void * end() const
				{
					return (start_ + size_);
				}
				inline size_t size() const
				{
					return size_;
				}
				inline size_t align() const
				{
					return Align;
				}
			};
			template<>
			class MemAllocPool_MAlloc<1> : public MemAllocPool_BASE
			{
				mutable uint8_t * ptr_;
				size_t size_;

				MemAllocPool_MAlloc(const MemAllocPool_MAlloc<1> &);
				MemAllocPool_MAlloc(MemAllocPool_MAlloc<1> &&);
			public:
				static const MemAllocParam params = MemAllocParam::PARAMETRIC;

				static const size_t Size = 0;
				static const size_t Align = 1;
				static const bool StaticSize = false;
				static const bool StaticAlign = true;

				MemAllocPool_MAlloc(const MemAllocInfo * mai) : size_(mai->poolSize)
				{
					ptr_ = static_cast<uint8_t *>(malloc(size_));
					//if(!ptr_)
					//	throw std::
				}
				~MemAllocPool_MAlloc()
				{
					free(ptr_);
				}
				inline void * begin() const
				{
					return ptr_;
				}
				inline void * end() const
				{
					return (ptr_ + size_);
				}
				inline size_t size() const
				{
					return size_;
				}
				inline size_t align() const
				{
					return 1;
				}
			};
			template<>
			class MemAllocPool_MAlloc<0>;
		}
	}
}