#pragma once
#include <exception>
#include <queue>
#include <mutex>

#include <iostream>

#undef max

namespace zenith
{
	namespace util
	{
		namespace memory
		{

			enum class MemAllocHint { UNDEF = 0, SHORT_SPAN = 1, LONG_SPAN = 2, UNDEF_SPAN = 3 };
			enum class MemAllocMemory { UNDEF = 0, VIRTUAL, PHYSICAL };
			enum class MemAllocParam { UNDEF = 0, STATELESS, PARAMETERLESS, PARAMETRIC };

			struct MemAllocInfo_PoolExtension_BASE
			{
				size_t extension_type = 0;
			};
			struct MemAllocInfo_AllocExtension_BASE
			{
				size_t extension_type = 0;
			};

			struct MemAllocBlock;

			struct MemAllocInfo
			{
				typedef MemAllocBlock(*PFN_Allocate)(void * allocPtr, size_t size, MemAllocHint hint);
				typedef void(*PFN_Deallocate)(void * allocPtr, const MemAllocBlock &blk);

				size_t poolSize; //in bytes
				size_t poolAlign; //in bytes
				void * poolOffset; //in bytes
				MemAllocInfo_PoolExtension_BASE * poolExtension = nullptr;

				size_t allocMinSize;
				size_t allocMaxSize;
				size_t allocNumBlock;
				MemAllocInfo_AllocExtension_BASE * allocExtension = nullptr;

				void * allocPtr = nullptr;
				PFN_Allocate pfnAllocate = nullptr;
				PFN_Deallocate pfnDeallocate = nullptr;

				MemAllocInfo * left = nullptr;
				MemAllocInfo * right = nullptr;
			};

			struct MemAllocBlock
			{
				typedef size_t AllocatorID;
				static const AllocatorID NoOwner = std::numeric_limits<AllocatorID>::max();

				void * ptr = nullptr;
				size_t size = 0;
				AllocatorID owner_id = NoOwner;
				MemAllocBlock() : owner_id(0), size(0), ptr(nullptr) {}
				MemAllocBlock(size_t owner_id, void * ptr, size_t size) : owner_id(owner_id), size(size), ptr(ptr) {}
			};

			class MemAlloc_exception : public std::exception
			{
			public:
				MemAlloc_exception() : std::exception("MemAlloc_exception: unknown cause") {}
				MemAlloc_exception(const char * p) : std::exception(p) {}
				virtual ~MemAlloc_exception() {}
			};
			class MemAlloc_alloc_exception : public MemAlloc_exception
			{
			public:
				MemAlloc_alloc_exception() : MemAlloc_exception("MemAlloc_alloc_exception: unknown cause") {}
				MemAlloc_alloc_exception(const char * p) : MemAlloc_exception(p) {}
				virtual ~MemAlloc_alloc_exception() {}
			};
			class MemAlloc_owner_exception : public MemAlloc_exception
			{
			public:
				MemAlloc_owner_exception() : MemAlloc_exception("MemAlloc_owner_exception: unknown cause") {}
				MemAlloc_owner_exception(const char * p) : MemAlloc_exception(p) {}
				virtual ~MemAlloc_owner_exception() {}
			};

			
			class MemAllocFn_BASE
			{
			protected:
				inline void log_notify(const char * str) {}
				inline void log_error(const char * str) {}
				inline void error_own() {}
				inline void error_corrupt() {}
				inline void error_internal() {}
				inline MemAllocBlock error_cannot_alloc(const MemAllocBlock &blk) { return blk; }
			};

			//the most base class
			class MemAlloc_BASE : public MemAllocFn_BASE
			{
				static inline std::mutex &mutex()
				{
					static std::mutex mtx;
					return mtx;
				}
				static inline size_t &counter()
				{
					static size_t idCounter = 1;
					return idCounter;
				}				
				static inline std::priority_queue<size_t> &freeCounters()
				{
					/*MUST MAKE STATIC PRIORITY QUEUE*/
					static std::priority_queue<size_t> freeStack;
					return freeStack;
				}
				static void updateCounters()
				{
					std::cout << freeCounters().size() << "\n";
					while(!freeCounters().empty() && (freeCounters().top() + 1 == counter()))
					{
						freeCounters().pop();
						counter()--;
					}
				}
				static inline size_t newID()
				{
					std::lock_guard<std::mutex> lg(mutex());
					if(freeCounters().empty())
						return counter()++;
					else
					{
						size_t res = freeCounters().top();
						freeCounters().pop();
						return res;
					}
				}
				static inline void destroyID(size_t id)
				{
					std::lock_guard<std::mutex> lg(mutex());
					if (counter() == id + 1)
						counter()--;
					else
						freeCounters().push(id);

					updateCounters();
				}
				MemAlloc_BASE(const MemAlloc_BASE &);
				MemAlloc_BASE(MemAlloc_BASE &&);
				MemAlloc_BASE &operator=(const MemAlloc_BASE &) = delete;

				size_t allocator_id_;
			public:

				static const MemAllocMemory memory = MemAllocMemory::UNDEF;
				static const MemAllocParam params = MemAllocParam::UNDEF;
				static const size_t prefixSize = 0;
				static const size_t suffixSize = 0;

				MemAlloc_BASE() : allocator_id_(newID()) {}
				~MemAlloc_BASE()
				{
					destroyID(allocator_id_);
				}
				inline size_t id() const { return allocator_id_; }
				inline bool owns(const MemAllocBlock &blk) const { return blk.owner_id == allocator_id_; }
				inline MemAllocBlock getPrefix(const MemAllocBlock &blk) const { return MemAllocBlock(MemAllocBlock::NoOwner, nullptr, 0); }
				inline MemAllocBlock getSuffix(const MemAllocBlock &blk) const { return MemAllocBlock(MemAllocBlock::NoOwner, nullptr, 0); }


				inline size_t alignSize(size_t s) const { return s; }

			protected:

				inline MemAllocBlock return_blk(void * ptr, size_t size)
				{
					return MemAllocBlock(allocator_id_, ptr, size);
				}
			};


			class MemAllocVirtual_BASE : public MemAllocFn_BASE
			{
				MemAllocVirtual_BASE(const MemAllocVirtual_BASE &);
				MemAllocVirtual_BASE(MemAllocVirtual_BASE &&);
				MemAllocVirtual_BASE &operator=(const MemAllocVirtual_BASE &) = delete;
				MemAllocVirtual_BASE &operator=(MemAllocVirtual_BASE &&) = delete;
			public:

				static const MemAllocMemory memory = MemAllocMemory::UNDEF;
				static const MemAllocParam params = MemAllocParam::UNDEF;
				static const size_t prefixSize = 0;
				static const size_t suffixSize = 0;

				MemAllocVirtual_BASE(){}
				~MemAllocVirtual_BASE(){}
				inline bool owns(const MemAllocBlock &blk) const { return true; }
				inline MemAllocBlock getPrefix(const MemAllocBlock &blk) const { return MemAllocBlock(MemAllocBlock::NoOwner, nullptr, 0); }
				inline MemAllocBlock getSuffix(const MemAllocBlock &blk) const { return MemAllocBlock(MemAllocBlock::NoOwner, nullptr, 0); }


				inline size_t alignSize(size_t s) const { return s; }

			protected:

				inline MemAllocBlock return_blk(void * ptr, size_t size)
				{
					return MemAllocBlock(MemAllocBlock::NoOwner, ptr, size);
				}
			};

			class MemAllocPool_BASE
			{
				MemAllocPool_BASE(const MemAllocPool_BASE &);
				MemAllocPool_BASE(MemAllocPool_BASE &&);
				MemAllocPool_BASE &operator=(const MemAllocPool_BASE &) = delete;
			protected:
				MemAllocPool_BASE() {}
			public:
				static const MemAllocParam params = MemAllocParam::UNDEF;

				inline void * begin() const { return nullptr; }
				inline void * end() const { return nullptr; }
				inline size_t size() const { return 0; }
			};

			template<class E, E p1, E p2, E def>
			class MemAlloc_Combine_ENUM_
			{
			public:
				static const E result = (p1 == p2 ? p1 : def);
			};

			template<MemAllocParam def>
			class MemAlloc_Combine_ENUM_<MemAllocParam, MemAllocParam::PARAMETERLESS, MemAllocParam::STATELESS, def>
			{
			public:
				static const MemAllocParam result = MemAllocParam::PARAMETERLESS;
			};
			template<MemAllocParam def>
			class MemAlloc_Combine_ENUM_<MemAllocParam, MemAllocParam::STATELESS, MemAllocParam::PARAMETERLESS, def>
			{
			public:
				static const MemAllocParam result = MemAllocParam::PARAMETERLESS;
			};

			template<MemAllocParam p1, MemAllocParam p2>
			class MemAlloc_Combine_Param_ : public MemAlloc_Combine_ENUM_<MemAllocParam, p1, p2, MemAllocParam::UNDEF> {};

			template<MemAllocMemory p1, MemAllocMemory p2>
			class MemAlloc_Combine_Memory_ : public MemAlloc_Combine_ENUM_<MemAllocMemory, p1, p2, MemAllocMemory::UNDEF> {};
		}
	}
}