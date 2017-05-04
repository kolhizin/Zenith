#pragma once
#include "alloc_base.h"
namespace zenith
{
	namespace util
	{
		namespace memory
		{
			//GC
			//LOG
			//TIMER
			//STAT
			//LOCK

			template<class Base, size_t PrefixSize, size_t SuffixSize, MemAllocParam p>
			class MemAllocWrapper_AFIX_;

			template<class Base, size_t Prefix, size_t Suffix>
			class MemAllocWrapper_AFIX_<Base, Prefix, Suffix, MemAllocParam::PARAMETERLESS> : public Base
			{
			public:
				static const size_t prefixSize = Prefix;
				static const size_t suffixSize = Suffix;

				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock res = Base::allocate(size + Base::alignSize(Prefix) + Base::alignSize(Suffix), hint);
					if (res.size <= 0)
						return res;
					res.ptr = static_cast<uint8_t *>(res.ptr) + Base::alignSize(Prefix);
					return res;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					MemAllocBlock b = blk;
					b.ptr = static_cast<uint8_t *>(b.ptr) - Base::alignSize(Prefix);
					Base::deallocate(b);
				}
				inline bool owns(const MemAllocBlock &blk)
				{
					MemAllocBlock b = blk;
					b.ptr = static_cast<uint8_t *>(b.ptr) - Base::alignSize(Prefix);
					return Base::owns(b);
				}
				inline MemAllocBlock getPrefix(const MemAllocBlock &blk) const { return MemAllocBlock(0, static_cast<uint8_t *>(blk.ptr) - Base::alignSize(Prefix), Prefix); }
				inline MemAllocBlock getSuffix(const MemAllocBlock &blk) const { return MemAllocBlock(0, static_cast<uint8_t *>(blk.ptr) + blk.size - Base::alignSize(Suffix), Suffix); }
			};
			template<class Base, size_t Prefix, size_t Suffix>
			class MemAllocWrapper_AFIX_<Base, Prefix, Suffix, MemAllocParam::PARAMETRIC> : public Base
			{
				MemAllocWrapper_AFIX_();
			public:
				MemAllocWrapper_AFIX_(const MemAllocInfo * mai) : Base(mai) {}

				static const size_t prefixSize = Prefix;
				static const size_t suffixSize = Suffix;

				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock res = Base::allocate(size + Base::alignSize(Prefix) + Base::alignSize(Suffix), hint);
					if (res.size <= 0)
						return res;
					res.ptr = static_cast<uint8_t *>(res.ptr) + Base::alignSize(Prefix);
					return res;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					MemAllocBlock b = blk;
					b.ptr = static_cast<uint8_t *>(b.ptr) - Base::alignSize(Prefix);
					Base::deallocate(b);
				}
				inline bool owns(const MemAllocBlock &blk)
				{
					MemAllocBlock b = blk;
					b.ptr = static_cast<uint8_t *>(b.ptr) - Base::alignSize(Prefix);
					return Base::owns(b);
				}
				inline MemAllocBlock getPrefix(const MemAllocBlock &blk) const { return MemAllocBlock(0, static_cast<uint8_t*>(blk.ptr) - Base::alignSize(Prefix), Prefix); }
				inline MemAllocBlock getSuffix(const MemAllocBlock &blk) const { return MemAllocBlock(0, static_cast<uint8_t*>(blk.ptr) + blk.size - Base::alignSize(Suffix), Suffix); }
			};

			template<class Base, size_t PrefixSize, size_t SuffixSize>
			using MemAllocWrapper_AFIX = MemAllocWrapper_AFIX_<Base, PrefixSize, SuffixSize, Base::params>;

			template<class Base, MemAllocParam p>
			class MemAllocWrapper_STD_;

			template<class Base>
			class MemAllocWrapper_STD_<Base, MemAllocParam::PARAMETERLESS> : public MemAllocWrapper_AFIX<Base, sizeof(MemAllocBlock::owner_id) + sizeof(MemAllocBlock::size), 0>
			{
				typedef MemAllocWrapper_AFIX<Base, sizeof(MemAllocBlock::owner_id) + sizeof(MemAllocBlock::size), 0> uBase;
			public:
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{					
					MemAllocBlock res = uBase::allocate(size, hint);
					if (res.size <= 0)
						return res;
					MemAllocBlock prf = uBase::getPrefix(res);
					size_t * addInfo = static_cast<size_t *>(prf.ptr);
					addInfo[0] = res.size;
					addInfo[1] = res.owner_id;
					return res;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					uBase::deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock b = allocate(size, hint);
					return b.ptr;
				}
				inline void free(void * ptr)
				{
					MemAllocBlock b(0, ptr, 0);
					MemAllocBlock a(getPrefix(b));
					MemAllocBlock r;

					size_t * addInfo = static_cast<size_t *>(a.ptr);
					r.size = addInfo[0];
					r.owner_id = addInfo[1];
					r.ptr = ptr;

					uBase::deallocate(r);
				}
			};

			template<class Base>
			class MemAllocWrapper_STD_<Base, MemAllocParam::PARAMETRIC> : public MemAllocWrapper_AFIX<Base, sizeof(MemAllocBlock::owner_id) + sizeof(MemAllocBlock::size), 0>
			{
				MemAllocWrapper_STD_();
				typedef MemAllocWrapper_AFIX<Base, sizeof(MemAllocBlock::owner_id) + sizeof(MemAllocBlock::size), 0> uBase;
			public:
				MemAllocWrapper_STD_(const MemAllocInfo * mai) : MemAllocWrapper_AFIX<Base, sizeof(MemAllocBlock::owner_id) + sizeof(MemAllocBlock::size), 0>(mai) {}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock res = uBase::allocate(size, hint);
					if (res.size <= 0)
						return res;
					MemAllocBlock prf = uBase::getPrefix(res);
					size_t * addInfo = static_cast<size_t *>(prf.ptr);
					addInfo[0] = res.size;
					addInfo[1] = res.owner_id;
					return res;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					uBase::deallocate(blk);
				}
				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock b = allocate(size, hint);
					return b.ptr;
				}
				inline void free(void * ptr)
				{
					MemAllocBlock b(0, ptr, 0);
					MemAllocBlock a(getPrefix(b));
					MemAllocBlock r;

					size_t * addInfo = static_cast<size_t *>(a.ptr);
					r.size = addInfo[0];
					r.owner_id = addInfo[1];
					r.ptr = ptr;

					uBase::deallocate(r);
				}
			};

			template<class Base>
			using MemAllocWrapper_STD = MemAllocWrapper_STD_<Base,Base::params>;

			template<class Base, MemAllocParam p>
			class MemAllocWrapper_LOG_;

			template<class Base>
			class MemAllocWrapper_LOG_<Base, MemAllocParam::PARAMETERLESS> : public Base
			{
			public:
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					Base::log_notify("Called allocate()");
					return Base::allocate(size, hint);
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					Base::log_notify("Called deallocate()");
					Base::deallocate(blk);
				}
				inline bool owns(const MemAllocBlock &blk)
				{
					Base::log_notify("Called owns()");
					return Base::owns(blk);
				}

				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					Base::log_notify("Called alloc()");
					return Base::alloc(size, hint);
				}
				inline void free(void * p)
				{
					Base::log_notify("Called free()");
					Base::free(p);
				}
			};

			template<class Base>
			class MemAllocWrapper_LOG_<Base, MemAllocParam::PARAMETRIC> : public Base
			{
				MemAllocWrapper_LOG_();
			public:
				MemAllocWrapper_LOG_(const MemAllocInfo * mai) : Base(mai) {}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					Base::log_notify("Called allocate()");
					return Base::allocate(size, hint);
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					Base::log_notify("Called deallocate()");
					Base::deallocate(blk);
				}
				inline bool owns(const MemAllocBlock &blk)
				{
					Base::log_notify("Called owns()");
					return Base::owns(blk);
				}

				inline void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					Base::log_notify("Called alloc()");
					return Base::alloc(size, hint);
				}
				inline void free(void * p)
				{
					Base::log_notify("Called free()");
					Base::free(p);
				}
			};

			template<class Base>
			using MemAllocWrapper_LOG = MemAllocWrapper_LOG_<Base, Base::params>;


			template<class Base, MemAllocParam p>
			class MemAllocWrapper_STATELESS_IMPL_;

			template<class Base>
			class MemAllocWrapper_STATELESS_IMPL_<Base, MemAllocParam::PARAMETERLESS>
			{
				static inline Base &impl_instance_()
				{
					static Base global_instance;
					return global_instance;
				}
			protected:
				static inline Base &instance_()
				{
					return impl_instance_();
				}
				static inline void init_(const MemAllocInfo * mai)
				{
					static size_t counter = 0;
					if (counter > 0)
						throw MemAlloc_exception();
					impl_instance_();
					counter++;
				}
			};

			template<class Base>
			class MemAllocWrapper_STATELESS_IMPL_<Base, MemAllocParam::PARAMETRIC>
			{			
				static inline const MemAllocInfo * checkInit_(const MemAllocInfo * mai)
				{
					if (!mai)
						throw MemAlloc_exception();
					return mai;
				}
				static inline Base &impl_instance_(const MemAllocInfo * mai = nullptr)
				{
					static Base global_instance(checkInit_(mai));
					return global_instance;
				}

			protected:
				static inline Base &instance_()
				{
					return impl_instance_();
				}
				static inline void init_(const MemAllocInfo * mai)
				{
					static size_t counter = 0;
					if (counter > 0)
						throw MemAlloc_exception();
					impl_instance_(mai);
					counter++;
				}
			};

			template<class Base>
			class MemAllocWrapper_STATELESS : public MemAllocWrapper_STATELESS_IMPL_<Base, Base::params>
			{
			public:
				typedef Base base_alloc;

				static const MemAllocMemory memory = Base::memory;
				static const MemAllocParam params = MemAllocParam::STATELESS;
				static const size_t prefixSize = Base::prefixSize;
				static const size_t suffixSize = Base::suffixSize;

				inline static void init(const MemAllocInfo * mai) { init_(mai); }
				inline static Base &instance() { return instance_(); }

				inline static bool owns(const MemAllocBlock &b) { return instance_().owns(b); }
				inline static MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF) { return instance_().allocate(size, hint); }
				inline static void deallocate(const MemAllocBlock &blk) { instance_().deallocate(blk); }

				inline static void * alloc(size_t size, MemAllocHint hint = MemAllocHint::UNDEF) { return instance_().alloc(size, hint); }
				inline static void free(void * ptr) { return instance_().free(ptr); }

			};

			template<class T, class Base>
			class MemAllocWrapper_STL
			{
			public:
				typedef T value_type;
				typedef typename Base::base_alloc base_alloc;

				MemAllocWrapper_STL() {};

				template<class U>
				MemAllocWrapper_STL(const MemAllocWrapper_STL<U, Base> &other) {};

				inline static void init(const MemAllocInfo * mai) { Base::init(mai); }
				inline static base_alloc &instance() { return Base::instance(); }

				inline T * allocate(size_t size) { return static_cast<T *>(Base::instance().alloc(size * sizeof(T))); };
				inline void deallocate(T * ptr, size_t size) { Base::instance().free(ptr); };

				template<class U>
				inline bool operator ==(const MemAllocWrapper_STL<U, Base> &oth) const { return true; }
				template<class U>
				inline bool operator !=(const MemAllocWrapper_STL<U, Base> &oth) const { return false; }
			};

			template<class T, class Base>
			class MemAllocWrapper_STL_REF
			{
				template<class T2, class Base2> friend class MemAllocWrapper_STL_REF;

				MemAllocWrapper_STL_REF();
				Base &alloc_;
			public:
				typedef T value_type;

				MemAllocWrapper_STL_REF(Base &alloc) : alloc_(alloc) {};
				MemAllocWrapper_STL_REF(const MemAllocWrapper_STL_REF &) = default;
				MemAllocWrapper_STL_REF &operator =(const MemAllocWrapper_STL_REF &) = delete;

				template<class U>
				MemAllocWrapper_STL_REF(const MemAllocWrapper_STL_REF<U, Base> &other) : alloc_(other.alloc_) {};

				inline static Base &instance() { return alloc_; }

				inline T * allocate(size_t size) { return static_cast<T *>(alloc_.alloc(size * sizeof(T))); };
				inline void deallocate(T * ptr, size_t size) { alloc_.free(ptr); };

				template<class U>
				inline bool operator ==(const MemAllocWrapper_STL_REF<U, Base> &oth) const { return true; }
				template<class U>
				inline bool operator !=(const MemAllocWrapper_STL_REF<U, Base> &oth) const { return false; }
			};


			template<class Base, class StatInfo, class AuxAlloc, MemAllocParam p, MemAllocParam p_aux>
			class MemAllocWrapper_Stats_;

			enum class MemAlloc_StatInfo_State {UNDEF, ALLOCATION, ALIVE, DEALLOCATION};

			class MemAlloc_StatInfo_Base
			{
				MemAlloc_StatInfo_Base();
			private:
				MemAlloc_StatInfo_State state_;
			public:
				MemAlloc_StatInfo_Base(size_t size, MemAllocHint hint) : state_(MemAlloc_StatInfo_State::ALLOCATION) {}
				inline void deallocate(const MemAllocBlock &)
				{
					state_ = MemAlloc_StatInfo_State::DEALLOCATION;
				}
				inline void failed(const std::exception * e) {}
				inline void failed() {}
				inline void allocated(const MemAllocBlock &)
				{
					state_ = MemAlloc_StatInfo_State::ALIVE;
				}
				inline MemAlloc_StatInfo_State state() const { return state_; }
			};

			template<class StatInfo>
			struct MemAllocWrapper_Stats_Node_
			{
				MemAllocBlock blk, aux;
				StatInfo info;
				MemAllocWrapper_Stats_Node_ *prev, *next;
				MemAllocWrapper_Stats_Node_(size_t size, MemAllocHint hint) : info(size, hint), prev(nullptr), next(nullptr) {}
				inline bool operator ==(const MemAllocBlock &b) const
				{
					if (blk.owner_id != b.owner_id)
						return false;
					if (blk.size != b.size)
						return false;
					return (blk.ptr == b.ptr);
				}
			};

			template<class StatInfo>
			inline MemAllocWrapper_Stats_Node_<StatInfo> * insert_node_(void * ptr, MemAllocWrapper_Stats_Node_<StatInfo> * prev, MemAllocWrapper_Stats_Node_<StatInfo> * next, size_t size, MemAllocHint hint)
			{
				MemAllocWrapper_Stats_Node_<StatInfo> * res = new (ptr) MemAllocWrapper_Stats_Node_<StatInfo>(size, hint);
				res->next = next;
				if (next)
					next->prev = res;
				res->prev = prev;
				if (prev)
					prev->next = res;
				return res;
			}
			template<class StatInfo>
			inline void delete_node_(MemAllocWrapper_Stats_Node_<StatInfo> * node)
			{
				if (node->prev)
					node->prev->next = node->next;
				if (node->next)
					node->next->prev = node->prev;

				node->~MemAllocWrapper_Stats_Node_<StatInfo>();
			}
			template<class StatInfo>
			inline MemAllocWrapper_Stats_Node_<StatInfo> * find_node_(MemAllocWrapper_Stats_Node_<StatInfo> * root, const MemAllocBlock &blk)
			{
				while (root)
				{
					if (*root == blk)
						return root;
					root = root->next;
				}
				return nullptr;
			}

			template<class Base, class StatInfo, class AuxAlloc>
			class MemAllocWrapper_Stats_<Base, StatInfo, AuxAlloc, MemAllocParam::PARAMETERLESS, MemAllocParam::STATELESS> : public Base
			{
				MemAllocWrapper_Stats_Node_<StatInfo> * root_;
			public:
				MemAllocWrapper_Stats_() : root_(nullptr)
				{
				}
				~MemAllocWrapper_Stats_()
				{
					//DO NOT DO GARBAGE COLLECTION!!!
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock aux = AuxAlloc::allocate(sizeof(MemAllocWrapper_Stats_Node_<StatInfo>));
					
					MemAllocWrapper_Stats_Node_<StatInfo> * p = insert_node_<StatInfo>(aux.ptr, nullptr, root_, size, hint);
					
					p->aux = aux;
					MemAllocBlock res;
					try
					{
						res = Base::allocate(size, hint);
					}
					catch (const std::exception * e)
					{
						p->info.failed(e);
						delete_node_(p);
						AuxAlloc::deallocate(aux);
						throw;
					}
					catch (...)
					{
						p->info.failed();
						delete_node_(p);
						AuxAlloc::deallocate(aux);
						throw;
					}

					root_ = p;

					p->blk = res;
					p->info.allocated(res);
					return res;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					MemAllocWrapper_Stats_Node_<StatInfo> * p = find_node_(root_, blk);
					if (!p)
						throw MemAlloc_exception("Stats failed at deallocate: could not find node!");
					

					p->info.deallocate(blk);
					MemAllocBlock aux = p->aux;

					try
					{
						Base::deallocate(blk);
					}
					catch (const std::exception * e)
					{
						p->info.failed(e);
						throw;
					}
					catch (...)
					{
						p->info.failed();
						throw;
					}

					if (root_ == p)
						root_ = p->next;

					delete_node_(p);
					AuxAlloc::deallocate(aux);
				}				
			};
			template<class Base, class StatInfo, class AuxAlloc>
			class MemAllocWrapper_Stats_<Base, StatInfo, AuxAlloc, MemAllocParam::PARAMETRIC, MemAllocParam::STATELESS> : public Base
			{
				MemAllocWrapper_Stats_Node_<StatInfo> * root_;
				MemAllocWrapper_Stats_();
			public:
				MemAllocWrapper_Stats_(const MemAllocInfo * mai) : Base(mai), root_(nullptr)
				{
				}
				~MemAllocWrapper_Stats_()
				{
					//DO NOT DO GARBAGE COLLECTION!!!
				}
				inline MemAllocBlock allocate(size_t size, MemAllocHint hint = MemAllocHint::UNDEF)
				{
					MemAllocBlock aux = AuxAlloc::allocate(sizeof(MemAllocWrapper_Stats_Node_<StatInfo>));

					MemAllocWrapper_Stats_Node_<StatInfo> * p = insert_node_<StatInfo>(aux.ptr, nullptr, root_, size, hint);

					p->aux = aux;
					MemAllocBlock res;
					try
					{
						res = Base::allocate(size, hint);
					}
					catch (const std::exception * e)
					{
						p->info.failed(e);
						delete_node_(p);
						AuxAlloc::deallocate(aux);
						throw;
					}
					catch (...)
					{
						p->info.failed();
						delete_node_(p);
						AuxAlloc::deallocate(aux);
						throw;
					}

					root_ = p;

					p->blk = res;
					p->info.allocated(res);
					return res;
				}
				inline void deallocate(const MemAllocBlock &blk)
				{
					MemAllocWrapper_Stats_Node_<StatInfo> * p = find_node_(root_, blk);
					if (!p)
						throw MemAlloc_exception("Stats failed at deallocate: could not find node!");


					p->info.deallocate(blk);
					MemAllocBlock aux = p->aux;

					try
					{
						Base::deallocate(blk);
					}
					catch (const std::exception * e)
					{
						p->info.failed(e);
						throw;
					}
					catch (...)
					{
						p->info.failed();
						throw;
					}

					if (root_ == p)
						root_ = p->next;

					delete_node_(p);
					AuxAlloc::deallocate(aux);
				}
			};

			template<class Base, class StatInfo, class AuxAlloc>
			using MemAllocWrapper_Stats = MemAllocWrapper_Stats_<Base, StatInfo, AuxAlloc, Base::params, AuxAlloc::params>;
		}
	}
}