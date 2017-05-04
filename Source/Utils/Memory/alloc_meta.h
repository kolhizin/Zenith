#pragma once

#include "alloc_base.h"

namespace zenith
{
	namespace util
	{
		namespace memory
		{
			template<class Main, class Backup, MemAllocParam p>
			class MemAlloc_Fallback_;

			template<class Main, class Backup>
			class MemAlloc_Fallback_<Main, Backup, MemAllocParam::PARAMETERLESS> : public Main
			{
				Backup backup_;
			public:
				inline bool owns(const MemAllocBlock &blk) const { return Main::owns(blk) || backup_.owns(blk); }
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					try
					{
						MemAllocBlock blk = Main::allocate(size, hint);
						if (blk.size <= 0)
							throw MemAlloc_exception();
						return blk;
					}
					catch (MemAlloc_exception &)
					{
						MemAllocBlock blk = backup_.allocate(size, hint);
						return blk;
					}					
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (Main::owns(blk))
						Main::deallocate(blk);
					else
						backup_.deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					try
					{
						return Main::alloc(size, hint);
					}
					catch (MemAlloc_exception &)
					{
						return backup_.alloc(size, hint);
					}
				}
				inline void free(void * ptr)
				{
					if (Main::owns(ptr))
						Main::free(ptr);
					else
						backup_.free(ptr);
				}
			};
			template<class Main, class Backup>
			class MemAlloc_Fallback_<Main, Backup, MemAllocParam::PARAMETRIC> : public Main
			{
				Backup backup_;
			public:
				MemAlloc_Fallback_(const MemAllocInfo * mai) : Main(mai), backup_(mai) {}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					try
					{
						MemAllocBlock blk = Main::allocate(size, hint);
						if (blk.size <= 0)
							throw MemAlloc_exception();
						return blk;
					}
					catch (MemAlloc_exception &)
					{
						MemAllocBlock blk = backup_.allocate(size, hint);
						return blk;
					}
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (Main::owns(blk))
						Main::deallocate(blk);
					else
						backup_.deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					try
					{
						return Main::alloc(size, hint);
					}
					catch (MemAlloc_exception &)
					{
						return backup_.alloc(size, hint);
					}
				}
				inline void free(void * ptr)
				{
					if (Main::owns(ptr))
						Main::free(ptr);
					else
						backup_.free(ptr);
				}
			};

			template <class Main, class Backup>
			using MemAlloc_Fallback = MemAlloc_Fallback_<Main, Backup, MemAlloc_Combine_Param_<Main::params, Backup::params>::result>;

			template<size_t MainMaxSize, class Main, class Backup, MemAllocParam p>
			class MemAlloc_FallbackOrSize_;

			template<size_t MainMaxSize, class Main, class Backup>
			class MemAlloc_FallbackOrSize_<MainMaxSize, Main, Backup, MemAllocParam::PARAMETERLESS> : public Main
			{
				Backup backup_;
			public:
				inline bool owns(const MemAllocBlock &blk) const { return Main::owns(blk) || backup_.owns(blk); }
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size > MainMaxSize)
					{
						MemAllocBlock blk = backup_.allocate(size, hint);
						return blk;
					}
					try
					{
						MemAllocBlock blk = Main::allocate(size, hint);
						if (blk.size <= 0)
							throw MemAlloc_exception();
						return blk;
					}
					catch (MemAlloc_exception &)
					{
						MemAllocBlock blk = backup_.allocate(size, hint);
						return blk;
					}
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (Main::owns(blk))
						Main::deallocate(blk);
					else
						backup_.deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size > MainMaxSize)
					{
						return backup_.alloc(size, hint);
					}
					try
					{
						return Main::alloc(size, hint);
					}
					catch (MemAlloc_exception &)
					{
						return backup_.alloc(size, hint);
					}
				}
				inline void free(void * ptr)
				{
					if (Main::owns(ptr))
						Main::free(ptr);
					else
						backup_.free(ptr);
				}
			};
			template<size_t MainMaxSize, class Main, class Backup>
			class MemAlloc_FallbackOrSize_<MainMaxSize, Main, Backup, MemAllocParam::PARAMETRIC> : public Main
			{
				Backup backup_;
			public:
				MemAlloc_FallbackOrSize_(const MemAllocInfo * mai) : Main(mai), backup_(mai) {}
				inline bool owns(const MemAllocBlock &blk) const { return Main::owns(blk) || backup_.owns(blk); }
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size > MainMaxSize)
					{
						MemAllocBlock blk = backup_.allocate(size, hint);
						return blk;
					}
					try
					{
						MemAllocBlock blk = Main::allocate(size, hint);
						if (blk.size <= 0)
							throw MemAlloc_exception();
						return blk;
					}
					catch (MemAlloc_exception &)
					{
						MemAllocBlock blk = backup_.allocate(size, hint);
						return blk;
					}
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (Main::owns(blk))
						Main::deallocate(blk);
					else
						backup_.deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size > MainMaxSize)
					{
						return backup_.alloc(size, hint);
					}
					try
					{
						return Main::alloc(size, hint);
					}
					catch (MemAlloc_exception &)
					{
						return backup_.alloc(size, hint);
					}
				}
				inline void free(void * ptr)
				{
					if (Main::owns(ptr))
						Main::free(ptr);
					else
						backup_.free(ptr);
				}
			};

			template <size_t MainMaxSize, class Main, class Backup>
			using MemAlloc_FallbackOrSize = MemAlloc_FallbackOrSize_<MainMaxSize, Main, Backup, MemAlloc_Combine_Param_<Main::params, Backup::params>::result>;

			

			template<size_t threshold, class SmallA, class LargeA, class BaseFn, MemAllocParam p>
			class MemAlloc_StaticSegregator_;

			template<size_t threshold, class SmallA, class LargeA, class BaseFn>
			class MemAlloc_StaticSegregator_<threshold, SmallA, LargeA, BaseFn, MemAllocParam::PARAMETERLESS> : public BaseFn
			{
				SmallA small_;
				LargeA large_;
			public:
				static_assert(MemAlloc_Combine_Memory_<SmallA::memory, LargeA::memory>::result != MemAllocMemory::UNDEF, "Parameter-options should be the same!");

				static const MemAllocMemory memory = MemAlloc_Combine_Memory_<SmallA::memory, LargeA::memory>::result;
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;

				inline bool owns(const MemAllocBlock &blk) const
				{
					if (blk.size <= threshold)
						return small_.owns(blk);
					else return large_.owns(blk);
				}
				inline bool owns(void * ptr) const
				{
					if (small_.owns(ptr))
						return true;
					if (large_.owns(ptr))
						return true;
					return false;
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size <= threshold)
						return small_.allocate(size, hint);
					else return large_.allocate(size, hint);
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (blk.size <= threshold)
						small_.deallocate(blk);
					else large_.deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size <= threshold)
						return small_.alloc(size, hint);
					else return large_.alloc(size, hint);
				}
				inline void free(void * ptr)
				{
					if (small_.owns(ptr))
						small_.free(ptr);
					else if(large_.owns(ptr))
						large_.free(ptr);
					else throw MemAlloc_owner_exception("Error in MemAlloc_StaticSegregator_::free()!");
				}
			
				inline size_t alignSize(size_t s) const 
				{
					if (s <= threshold)
						return small_.alignSize(s);
					return large_.alignSize(s);
				}
			};

			template<size_t threshold, class SmallA, class LargeA, class BaseFn>
			class MemAlloc_StaticSegregator_<threshold, SmallA, LargeA, BaseFn, MemAllocParam::PARAMETRIC> : public BaseFn
			{
				SmallA small_;
				LargeA large_;
			public:
				MemAlloc_StaticSegregator_(const MemAllocInfo * mai) : small_(mai->left), large_(mai->right) {}
				static_assert(MemAlloc_Combine_Memory_<SmallA::memory, LargeA::memory>::result != MemAllocMemory::UNDEF, "Parameter-options should be the same!");

				static const MemAllocMemory memory = MemAlloc_Combine_Memory_<SmallA::memory, LargeA::memory>::result;
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;

				inline bool owns(const MemAllocBlock &blk) const
				{
					if (blk.size <= threshold)
						return small_.owns(blk);
					else return large_.owns(blk);
				}
				inline bool owns(void * ptr) const
				{
					if (small_.owns(ptr))
						return true;
					if (large_.owns(ptr))
						return true;
					return false;
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size <= threshold)
						return small_.allocate(size, hint);
					else return large_.allocate(size, hint);
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (blk.size <= threshold)
						small_.deallocate(blk);
					else large_.deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size <= threshold)
						return small_.alloc(size, hint);
					else return large_.alloc(size, hint);
				}
				inline void free(void * ptr)
				{
					if (small_.owns(ptr))
						small_.free(ptr);
					else if (large_.owns(ptr))
						large_.free(ptr);
					else throw MemAlloc_owner_exception("Error in MemAlloc_StaticSegregator_::free()!");
				}

				inline size_t alignSize(size_t s) const
				{
					if (s <= threshold)
						return small_.alignSize(s);
					return large_.alignSize(s);
				}

			};

			template<size_t threshold, class SmallA, class LargeA, class BaseFn>
			using MemAlloc_StaticSegregator = MemAlloc_StaticSegregator_<threshold, SmallA, LargeA, BaseFn, MemAlloc_Combine_Param_<SmallA::params, LargeA::params>::result>;






			template<class SmallA, class LargeA, class BaseFn, MemAllocParam p>
			class MemAlloc_DynamicSegregator_;

			template<class SmallA, class LargeA, class BaseFn>
			class MemAlloc_DynamicSegregator_<SmallA, LargeA, BaseFn, MemAllocParam::PARAMETRIC> : public BaseFn
			{
				SmallA small_;
				LargeA large_;
				size_t threshold_;
			public:
				MemAlloc_DynamicSegregator_(const MemAllocInfo * mai) : small_(mai->left), large_(mai->right), threshold_(mai->allocMinSize) {}
				static_assert(MemAlloc_Combine_Memory_<SmallA::memory, LargeA::memory>::result != MemAllocMemory::UNDEF, "Parameter-options should be the same!");

				static const MemAllocMemory memory = MemAlloc_Combine_Memory_<SmallA::memory, LargeA::memory>::result;
				static const MemAllocParam params = MemAllocParam::PARAMETERLESS;

				inline bool owns(const MemAllocBlock &blk) const
				{
					if (blk.size <= threshold_)
						return small_.owns(blk);
					else return large_.owns(blk);
				}
				inline bool owns(void * ptr) const
				{
					if (small_.owns(ptr))
						return true;
					if (large_.owns(ptr))
						return true;
					return false;
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size <= threshold_)
						return small_.allocate(size, hint);
					else return large_.allocate(size, hint);
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					if (blk.size <= threshold_)
						small_.deallocate(blk);
					else large_.deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					if (size <= threshold_)
						return small_.alloc(size, hint);
					else return large_.alloc(size, hint);
				}
				inline void free(void * ptr)
				{
					if (small_.owns(ptr))
						small_.free(ptr);
					else if (large_.owns(ptr))
						large_.free(ptr);
					else throw MemAlloc_owner_exception("Error in MemAlloc_DynamicSegregator_::free()!");
				}

				inline size_t alignSize(size_t s) const
				{
					if (s <= threshold_)
						return small_.alignSize(s);
					return large_.alignSize(s);
				}

			};

			template<class SmallA, class LargeA, class BaseFn>
			using MemAlloc_DynamicSegregator = MemAlloc_DynamicSegregator_<SmallA, LargeA, BaseFn, MemAlloc_Combine_Param_<SmallA::params, LargeA::params>::result>;

		}
	}
}