#pragma once
#include "Memory\alloc.h"
#include <iostream>

namespace zenith
{
	namespace util
	{
		template<class Base>
		using zstdAllocatorBase = memory::MemAlloc_FnExt_ErrThrow<memory::MemAlloc_FnExt_LogCOUT<Base>>;
		template<class Base>
		using zstdAllocatorAuxBase = memory::MemAlloc_FnExt_ErrThrow<Base>;

		using zstdAllocatorAuxImpl = memory::MemAlloc_MAlloc<zstdAllocatorAuxBase<memory::MemAlloc_BASE>>;
		using zstdAllocatorStaticAux = memory::MemAllocWrapper_STATELESS<zstdAllocatorAuxImpl>;

		class zstdAllocStatInfo : public memory::MemAlloc_StatInfo_Base
		{			
		public:
			typedef memory::MemAlloc_StatInfo_Base base;

			zstdAllocStatInfo(size_t size, memory::MemAllocHint hint) : memory::MemAlloc_StatInfo_Base(size, hint)
			{
				std::cout << "Started allocation of block (" << size << ")\n";
			}
			~zstdAllocStatInfo()
			{
				std::cout << "Completed deallocation\n";
			}
			inline void deallocate(const memory::MemAllocBlock &b)
			{
				std::cout << "Started deallocation of block (ptr=" << b.ptr << ", size=" << b.size << ", owner=" << b.owner_id << ")\n";
				base::deallocate(b);
			}
			inline void failed()
			{
				std::cout << "AllocStatInfo.failed()\n";
			}
			inline void failed(const std::exception * e)
			{
				std::cout << "AllocStatInfo.failed(e)\n";
			}
			inline void allocated(const memory::MemAllocBlock &b)
			{
				std::cout << "Completed allocation of block (ptr=" << b.ptr << ", size=" << b.size << ", owner=" << b.owner_id << ")\n";
				base::allocated(b);
			}
		};

		template<class T>
		using zstdAllocatorWrapper = memory::MemAllocWrapper_LOG<memory::MemAllocWrapper_STD<
											memory::MemAllocWrapper_Stats<T, zstdAllocStatInfo, zstdAllocatorStaticAux>
										>>;
		//template<class T>
		//using zstdAllocatorWrapper = memory::MemAllocWrapper_STD<
		//	memory::MemAllocWrapper_Stats<T, zstdAllocStatInfo, zstdAllocatorStaticAux>
		//	>;


		using zstdAllocatorGlobalImpl =
			memory::MemAlloc_Fallback<
				memory::MemAlloc_StaticSegregator<16384,
					memory::MemAlloc_StaticSegregator<256,
						memory::MemAlloc_Combine<
							memory::MemAllocVirtual_BitmapStatic<zstdAllocatorBase<memory::MemAlloc_BASE>, 8, 16384>,
							memory::MemAllocPool_Stack<8 * 16384, 8>
						>,
						memory::MemAlloc_Combine<
							memory::MemAllocVirtual_BitmapStatic<zstdAllocatorBase<memory::MemAlloc_BASE>, 256, 16384>,
							memory::MemAllocPool_StaticHeap<256 * 16384, 16>
						>,
						zstdAllocatorBase<memory::MemAllocFn_BASE>
					>,
					memory::MemAlloc_MAlloc<zstdAllocatorBase<memory::MemAlloc_BASE>>,
					zstdAllocatorBase<memory::MemAllocFn_BASE>
				>,
				memory::MemAlloc_MAlloc<zstdAllocatorBase<memory::MemAlloc_BASE>>
			>;

		using zstdAllocatorLocalImpl =
			memory::MemAlloc_Fallback<
				memory::MemAlloc_StaticSegregator<512,
					memory::MemAlloc_Combine<
						memory::MemAllocVirtual_BitmapStatic<zstdAllocatorBase<memory::MemAlloc_BASE>, 8, 4096>,
						memory::MemAllocPool_Stack<8 * 4096, 8>
					>,
					memory::MemAlloc_MAlloc<zstdAllocatorBase<memory::MemAlloc_BASE>>,
					zstdAllocatorBase<memory::MemAllocFn_BASE>
				>,
				memory::MemAlloc_MAlloc<zstdAllocatorBase<memory::MemAlloc_BASE>>
			>;
		
		template<class Alloc>
		using zstdAllocatorStatic = memory::MemAllocWrapper_STATELESS<Alloc>;

		template<class T, class Alloc>
		using zstdAllocatorSTL_GLOBAL = memory::MemAllocWrapper_STL<T, Alloc>;

		template<class T, class Alloc>
		using zstdAllocatorSTL_LOCAL = memory::MemAllocWrapper_STL_REF<T, Alloc>;


		using zstdAllocatorMain = zstdAllocatorWrapper<zstdAllocatorGlobalImpl>;
		using zstdAllocatorMainStatic = zstdAllocatorStatic<zstdAllocatorMain>;
		using zstdAllocatorLocal = zstdAllocatorWrapper<zstdAllocatorLocalImpl>;


		template<class T>
		using zstdAllocatorLocalSTL = zstdAllocatorSTL_LOCAL<T, zstdAllocatorLocal>;

		template<class T>
		using zstdAllocatorMainSTL = zstdAllocatorSTL_GLOBAL<T, zstdAllocatorMainStatic>;

		template<class FallBack, size_t sz1, size_t n1, size_t sz2, size_t n2>
		using zstdLocalPoolAllocator2 =
			memory::MemAlloc_Fallback<
				memory::MemAlloc_StaticSegregator<sz1,
					memory::MemAlloc_Combine<memory::MemAllocVirtual_StaticSize<zstdAllocatorBase<memory::MemAlloc_BASE>, 1, sz1, n1>, memory::MemAllocPool_Stack<sz1 * n1, sz1>>,
					memory::MemAlloc_Combine<memory::MemAllocVirtual_StaticSize<zstdAllocatorBase<memory::MemAlloc_BASE>, sz1 + 1, sz2, n2>, memory::MemAllocPool_Stack<sz2 * n2, sz2>>,
					zstdAllocatorBase<memory::MemAlloc_BASE>>,
			FallBack>;

		template<class FallBack, size_t sz1, size_t n1, size_t sz2, size_t n2, size_t sz3, size_t n3>
		using zstdLocalPoolAllocator3 =
			memory::MemAlloc_FallbackOrSize<sz3,			
				memory::MemAlloc_StaticSegregator<sz2,
					memory::MemAlloc_StaticSegregator<sz1,
						memory::MemAlloc_Combine<memory::MemAllocVirtual_StaticSize<zstdAllocatorBase<memory::MemAlloc_BASE>, 1, sz1, n1>, memory::MemAllocPool_Stack<sz1 * n1, sz1>>,
						memory::MemAlloc_Combine<memory::MemAllocVirtual_StaticSize<zstdAllocatorBase<memory::MemAlloc_BASE>, sz1 + 1, sz2, n2>, memory::MemAllocPool_Stack<sz2 * n2, sz2>>,
						zstdAllocatorBase<memory::MemAlloc_BASE>>,
					memory::MemAlloc_Combine<memory::MemAllocVirtual_StaticSize<zstdAllocatorBase<memory::MemAlloc_BASE>, sz2 + 1, sz3, n3>, memory::MemAllocPool_Stack<sz3 * n3, sz3>>,
					zstdAllocatorBase<memory::MemAlloc_BASE>>,
				FallBack>;

		template<class FallBack, size_t sz1, size_t n1, size_t sz2, size_t n2>
		class zstdLocalPool2
		{
		public:
			using AllocatorType = zstdLocalPoolAllocator2<FallBack, sz1, n1, sz2, n2>;

			template<class T>
			using STLAllocatorType = memory::MemAllocWrapper_STL_REF<T, AllocatorType>;
			
		private:
			AllocatorType alloc_;
			STLAllocatorType<void> stlAlloc_;
			zstdLocalPool2(const zstdLocalPool2 &) = delete;
			zstdLocalPool2(zstdLocalPool2 &&) = delete;
			zstdLocalPool2 &operator =(const zstdLocalPool2 &) = delete;
		public:
			zstdLocalPool2() : alloc_(), stlAlloc_(alloc_) {}

			inline AllocatorType &allocator() { return alloc_; }
			inline const AllocatorType &allocator() const { return alloc_; }

			inline STLAllocatorType<void> &stl_allocator() { return stlAlloc_; }
			inline const STLAllocatorType<void> &stl_allocator() const { return stlAlloc_; }
		};
		template<class FallBack, size_t sz1, size_t n1, size_t sz2, size_t n2, size_t sz3, size_t n3>
		class zstdLocalPool3
		{
		public:
			using AllocatorType = zstdLocalPoolAllocator3<FallBack, sz1, n1, sz2, n2, sz3, n3>;

			template<class T>
			using STLAllocatorType = memory::MemAllocWrapper_STL_REF<T, AllocatorType>;

		private:
			AllocatorType alloc_;
			STLAllocatorType<void> stlAlloc_;
			zstdLocalPool3(const zstdLocalPool3 &) = delete;
			zstdLocalPool3(zstdLocalPool3 &&) = delete;
			zstdLocalPool3 &operator =(const zstdLocalPool3 &) = delete;
		public:
			zstdLocalPool3() : alloc_(), stlAlloc_(alloc_) {}

			inline AllocatorType &allocator() { return alloc_; }
			inline const AllocatorType &allocator() const { return alloc_; }

			inline STLAllocatorType<void> &stl_allocator() { return stlAlloc_; }
			inline const STLAllocatorType<void> &stl_allocator() const { return stlAlloc_; }
		};
	}
}
/*
void * operator new(std::size_t n)// throw(std::bad_alloc)
{
	return zenith::util::zstdAllocatorMainStatic::alloc(n);
}
void operator delete(void * p)// throw()
{
	zenith::util::zstdAllocatorMainStatic::free(p);
}
*/