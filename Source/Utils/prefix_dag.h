#pragma once
#include "Memory\alloc_virtual.h"
#include "log_config.h"
#include <string>

namespace zenith
{
	namespace util
	{
		class PrefixDagException : public std::exception
		{
		public:
			PrefixDagException() : std::exception("PrefixDagException: unknown cause") {}
			PrefixDagException(const char * p) : std::exception(p) {}
			virtual ~PrefixDagException() {}
		};
		class PrefixDagInvalidUseException : PrefixDagException
		{
		public:
			PrefixDagInvalidUseException() : PrefixDagException("PrefixDagInvalidUseException: invalid use!") {}
			PrefixDagInvalidUseException(const char * p) : PrefixDagException(p) {}
			virtual ~PrefixDagInvalidUseException() {}
		};
		class PrefixDagDuplicateKeyException : PrefixDagException
		{
		public:
			PrefixDagDuplicateKeyException() : PrefixDagException("PrefixDagDuplicateKeyException: key already exists!") {}
			PrefixDagDuplicateKeyException(const char * p) : PrefixDagException(p) {}
			virtual ~PrefixDagDuplicateKeyException() {}
		};
		class PrefixDagMissingKeyException : PrefixDagException
		{
		public:
			PrefixDagMissingKeyException() : PrefixDagException("PrefixDagMissingKeyException: key does not exist!") {}
			PrefixDagMissingKeyException(const char * p) : PrefixDagException(p) {}
			virtual ~PrefixDagMissingKeyException() {}
		};
		class PrefixDagOutOfMemException : PrefixDagException
		{
		public:
			PrefixDagOutOfMemException() : PrefixDagException("PrefixDagOutOfMemException: not enough memory to perform operation!") {}
			PrefixDagOutOfMemException(const char * p) : PrefixDagException(p) {}
			virtual ~PrefixDagOutOfMemException() {}
		};
		class PrefixDagInvalidIteratorException : PrefixDagException
		{
		public:
			PrefixDagInvalidIteratorException() : PrefixDagException("PrefixDagInvalidIteratorException: invalid iterator!") {}
			PrefixDagInvalidIteratorException(const char * p) : PrefixDagException(p) {}
			virtual ~PrefixDagInvalidIteratorException() {}
		};

		template<class KeyType, class ValueType, size_t MaxValues = 10, size_t MaxKeys = 100>
		class prefix_dag
		{
		public:
			static const uint32_t NoValueId = 0xFFFFFFFF;

			typedef prefix_dag<KeyType, ValueType, MaxValues, MaxKeys> object_type;
			typedef ValueType value_type;
			typedef KeyType key_element_type;
			typedef const KeyType * key_type;

			template<class V, class T, class U>
			class iterator_t_;
		private:
			struct prefix_dag_node_
			{
				template<class V, class T, class U>
				friend class iterator_t_;
				//NOTE:
				/*
				1. Consider use of value-ptr instead of value-id
				2. Full structure can not be "byte-movable" (it would mean imposing such restrictions on <ValueType>)
				3. Nodes are not "byte-movable" also (it disrupts pointer in parent)
				4. Consider adding parent-ptr to allow use of iterators
				5. keyBegin_ is start of key (which is not 0-terminated) and keyNum_ is its length
				6. dagBegin_ is start of continuous array of children, dagNum_ is its length
				*/
				KeyType * keyBegin_ = nullptr;
				prefix_dag_node_ * dagBegin_ = nullptr;
				prefix_dag_node_ * dagParent_ = nullptr;

				uint32_t keyNum_ = 0;
				uint32_t dagNum_ = 0;
				uint32_t valueId_ = NoValueId;

				inline static prefix_dag_node_ empty() { return prefix_dag_node_(); }
			};
			static const size_t KeySize = sizeof(KeyType);
			static const size_t ValueSize = sizeof(ValueType);
			static const size_t NodeSize = sizeof(prefix_dag_node_);

			memory::MemAllocVirtual_BitmapStatic<memory::MemAllocVirtual_BASE, KeySize, MaxKeys> allocKey_;
			memory::MemAllocVirtual_BitmapStatic<memory::MemAllocVirtual_BASE, NodeSize, MaxValues> allocNode_;
			uint8_t keyBuff_[KeySize * MaxKeys], nodeBuff_[NodeSize * (1 + MaxValues)], valueBuff_[ValueSize * MaxValues];
			uint32_t valueBuffTop_;

			//CHUNK: memory operation with nodes
			prefix_dag_node_ * alloc_node_(uint32_t num = 1)
			{
				auto req = NodeSize * num;
				auto blk = allocNode_.allocate(req);
				if(blk.size < req)
					throw PrefixDagOutOfMemException("prefix_dag::alloc_node_(): out of memory for nodes");
				return reinterpret_cast<prefix_dag_node_ *>(&nodeBuff_[0] + reinterpret_cast<size_t>(blk.ptr));
			}
			void dealloc_node_(prefix_dag_node_ * p, uint32_t num = 1)
			{
				memory::MemAllocBlock blk(memory::MemAllocBlock::NoOwner,
					reinterpret_cast<void *>(reinterpret_cast<uint8_t *>(p) - &nodeBuff_[0]), NodeSize * num);
				allocNode_.deallocate(blk);
			}
			prefix_dag_node_ * new_node_()
			{
				prefix_dag_node_ * n = alloc_node_();
				new (n) prefix_dag_node_;
				return n;
			}
			inline prefix_dag_node_ * new_nodes_(uint32_t num)
			{
				prefix_dag_node_ * res = alloc_node_(num);
				auto p = res;
				for (uint32_t i = 0; i < num; i++)
				{
					new (p) prefix_dag_node_;
					p++;
				}
				return res;
			}
			void delete_node_(prefix_dag_node_ * p)
			{
				if (p->keyNum_ != 0)
					dealloc_key_(p->keyBegin_, p->keyNum_);
				p->~prefix_dag_node_();
				dealloc_node_(p);
			}
			void delete_nodes_(prefix_dag_node_ * p, uint32_t num)
			{
				for (uint32_t i = 0; i < num; i++)
				{
					if (p->keyNum_ != 0)
						dealloc_key_(p->keyBegin_, p->keyNum_);
					p->~prefix_dag_node_();
					dealloc_node_(p);
				}
			}
			//simple byte-copy of node
			inline void copy_node_(prefix_dag_node_ * pd, const prefix_dag_node_ * ps)
			{
				pd->keyBegin_ = ps->keyBegin_;
				pd->keyNum_ = ps->keyNum_;
				pd->dagBegin_ = ps->dagBegin_;
				pd->dagNum_ = ps->dagNum_;
				pd->valueId_ = ps->valueId_;
				pd->dagParent_ = ps->dagParent_;
			}

			inline void reassign_children_parent_(prefix_dag_node_ * node, prefix_dag_node_ * new_parent)
			{
				for (uint32_t i = 0; i < node->dagNum_; i++)
					node->dagBegin_[i].dagParent_ = new_parent;
			}

			//CHUNK: memory operation with keys
			KeyType * alloc_key_(uint32_t length)
			{
				auto blk = allocKey_.allocate(length * KeySize);
				if (blk.size < length * KeySize)
					throw PrefixDagOutOfMemException("prefix_dag::alloc_key_(): out of memory for keys");
				return reinterpret_cast<KeyType *>(&keyBuff_[0] + reinterpret_cast<size_t>(blk.ptr));
			}
			void dealloc_key_(KeyType * p, uint32_t length)
			{
				memory::MemAllocBlock blk(memory::MemAllocBlock::NoOwner,
					reinterpret_cast<void *>(reinterpret_cast<uint8_t *>(p) - &keyBuff_[0]), length * KeySize);
				allocKey_.deallocate(blk);
			}
			void copy_key_(KeyType * pd, const KeyType * ps, uint32_t len)
			{
				for (uint32_t i = 0; i < len; i++)
				{
					*pd = *ps;
					pd++;
					ps++;
				}
			}

			//CHUNK: memory operation with values
			uint32_t alloc_value_()
			{
				auto tmp = valueBuffTop_++;
				if (valueBuffTop_ > MaxValues)
					throw PrefixDagOutOfMemException("prefix_dag::alloc_value_(): out of memory for values");
				return tmp;
			}
			inline ValueType * ptr_value_(uint32_t id) { return reinterpret_cast<ValueType *>(&valueBuff_[id * ValueSize]); }
			inline const ValueType * ptr_value_(uint32_t id) const { return reinterpret_cast<const ValueType *>(&valueBuff_[id * ValueSize]); }
			//only used in destruction of full structure
			inline void delete_values_recursive_(prefix_dag_node_ * n)
			{
				if (n->valueId_ != NoValueId)
				{
					ptr_value_(n->valueId_)->~ValueType();
					n->valueId_ = NoValueId;
				}
				for (uint32_t i = 0; i < n->dagNum_; i++)
					delete_values_recursive_(n->dagBegin_ + i);
			}
			inline uint32_t new_value_(ValueType &&v)
			{
				uint32_t id = alloc_value_();
				new (ptr_value_(id)) ValueType(std::move(v));
				return id;
			}

			inline void create_value_(prefix_dag_node_ * p, ValueType &&v)
			{
				if (p->valueId_ != NoValueId)
					throw PrefixDagDuplicateKeyException("prefix_dag::create_value(): key already exists!");
				p->valueId_ = new_value_(std::move(v));
			}
			inline void delete_value_(prefix_dag_node_ * p)
			{
				if (p->valueId_ == NoValueId)
					throw PrefixDagMissingKeyException("prefix_dag::delete_value(): key does not exist!");
				ptr_value_(p->valueId_)->~ValueType();
				p->valueId_ = NoValueId;
			}
			inline ValueType * get_value_(prefix_dag_node_ * p)
			{
				if (p->valueId_ == NoValueId)
					return nullptr;
				return ptr_value_(p->valueId_);
			}
			inline const ValueType * get_value_(const prefix_dag_node_ * p) const
			{
				if (p->valueId_ == NoValueId)
					return nullptr;
				return ptr_value_(p->valueId_);
			}

			//CHUNK: logical node operations
			inline prefix_dag_node_ * root_() { return reinterpret_cast<prefix_dag_node_ *>(&nodeBuff_[0]); }
			inline const prefix_dag_node_ * root_() const { return reinterpret_cast<const prefix_dag_node_ *>(&nodeBuff_[0]); }
			void create_root_node_()
			{
				auto p = alloc_node_();
				if (p != root_())
				{
					dealloc_node_(p);
					throw PrefixDagException("prefix_dag::create_root_node_(): failed during root-node creation");
				}
				new (p) prefix_dag_node_();
			}
			//technically realloc nodes
			inline prefix_dag_node_ * add_node_tech_(prefix_dag_node_ * p, uint32_t numCur, uint32_t idxNew)
			{
				uint32_t numNew = numCur + 1;
				prefix_dag_node_ * n = new_nodes_(numNew);
				uint32_t j = 0;
				for (uint32_t i = 0; i < numNew; i++)
				{
					if (i == idxNew)
						continue;

					copy_node_(n + i, p + j);
					reassign_children_parent_(n + i, n + i);
					p[j].keyNum_ = 0;
					p[j].keyBegin_ = nullptr;
					p[j].valueId_ = NoValueId;
					j++;
				}
				
				delete_nodes_(p, numCur); //children already rerouted
				return n;
			}
			inline prefix_dag_node_ * add_node_(prefix_dag_node_ * n, const KeyType * k, uint32_t klen)
			{
				uint32_t oldNum = n->dagNum_;
				uint32_t newNum = oldNum + 1;
				uint32_t idxNew = 0;
				if (oldNum != 0)
				{
					auto pf = n->dagBegin_;
					auto pl = pf + (oldNum - 1);
					search_children0_(pf, pl, k);
					if (!pf)
						idxNew = 0;
					else if (!pl)
						idxNew = oldNum;
					else
						idxNew = pl - n->dagBegin_;
				}

				n->dagBegin_ = add_node_tech_(n->dagBegin_, oldNum, idxNew);
				n->dagNum_ = newNum;

				prefix_dag_node_ * p = n->dagBegin_ + idxNew; //new node
				p->keyBegin_ = alloc_key_(klen);
				p->keyNum_ = klen;
				copy_key_(p->keyBegin_, k, klen);

				p->dagBegin_ = nullptr;
				p->dagNum_ = 0;
				p->valueId_ = NoValueId;
				p->dagParent_ = n;
				return p;
			}
			
			inline std::pair<prefix_dag_node_ *, prefix_dag_node_ *> split_node_(prefix_dag_node_ * n, uint32_t klen)
			{
				prefix_dag_node_ * nn = new_node_();
				uint32_t leftLen = n->keyNum_ - klen;

				n->keyNum_ = klen;
				nn->keyNum_ = leftLen;
				nn->keyBegin_ = n->keyBegin_ + klen;

				nn->dagBegin_ = n->dagBegin_;
				nn->dagNum_ = n->dagNum_;

				n->dagBegin_ = nn;
				n->dagNum_ = 1;

				nn->valueId_ = n->valueId_;
				n->valueId_ = NoValueId;

				reassign_children_parent_(nn, nn);

				return std::pair<prefix_dag_node_ *, prefix_dag_node_ *>(n, nn);
			}
			//searches node
			template<class T>
			inline static uint32_t full_key_length_(T * pn)
			{
				uint32_t res = 0;
				while (pn)
				{
					res += pn->keyNum_;
					pn = pn->dagParent_;
				}
				return res;
			}
			template<class T>
			inline static uint32_t full_key_(T * pn, KeyType * buff = nullptr, uint32_t buffSize = 0)
			{
				uint32_t res = 0;
				auto ptmp = pn;
				while (ptmp)
				{
					res += ptmp->keyNum_;
					ptmp = ptmp->dagParent_;
				}
				if (res >= buffSize || !buff)
					return res;

				uint32_t idx = res;
				while (pn)
				{
					idx -= pn->keyNum_;
					for (uint32_t i = 0; i < pn->keyNum_; i++)
						buff[idx + i] = pn->keyBegin_[i];
					pn = pn->dagParent_;
				}
				return res;
			}

			//match = -1 -- pk mismatched node key (either shorter, or different)
			//match = +1 -- pk longer than node key
			//match = 0  -- pk equal to node key
			template<class T>
			inline static uint32_t match_(T * pn, const KeyType * pk, int &match)
			{
				match = 0;
				if (!pn || !pk)
					return 0;
				for (uint32_t i = 0; i < pn->keyNum_; i++)
				{
					if (*pk != pn->keyBegin_[i])
					{
						match = -1;
						return i;
					}
					pk++;
				}
				if (*pk)
					match = 1;
				return pn->keyNum_;
			}
			//searches children by first char
			template<class T>
			inline static void search_children0_(T * &first, T * &last, const KeyType * pk)
			{
				if (!first || !last)
					return;
				if (first > last)
					throw PrefixDagInvalidUseException("prefix_dag::search_children(): expecting last >= first");
				if (*first->keyBegin_ > *pk)
				{
					last = first;
					first = nullptr;
					return;
				}
				else if (*first->keyBegin_ == *pk)
				{
					last = first;
					return;
				}

				if (*last->keyBegin_ < *pk)
				{
					first = last;
					last = nullptr;
					return;
				}else if (*last->keyBegin_ == *pk)
				{
					first = last;
					return;
				}

				while (first < last)
				{
					uint32_t num = last - first;
					if (num == 1)
						return;					
					auto mid = first + (num >> 1);
					if (*mid->keyBegin_ == *pk)
					{
						first = mid;
						last = mid;
						return;
					}
					if (*mid->keyBegin_ < *pk)
						first = mid;
					else
						last = mid;
				}
			}

			//searches node
			template<class T>
			inline static std::pair<T *, const KeyType *> search_(T * pn, const KeyType * pk, bool &exact)
			{
				while (1)
				{
					if (!pn || !pk)
					{
						exact = false;
						return std::pair<T *, const KeyType *>(nullptr, nullptr);
					}
					if (!*pk)
					{
						exact = true;
						return std::pair<T *, const KeyType *>(pn, pk);
					}
					bool needContinue = false;
					
					if (pn->dagNum_ > 0)
					{
						int match = 0;
						uint32_t iadv = 0;
						auto pf = pn->dagBegin_;
						auto pl = pf + (pn->dagNum_ - 1);
						search_children0_(pf, pl, pk);
						if (pf == pl)
						{
							iadv = match_(pf, pk, match);
							if (match <= 0)
							{
								exact = (match == 0);
								return std::pair<T *, const KeyType *>(pf, pk);
							}
							pk += iadv;
							pn = pf;
							needContinue = true;
						}
					}
					if (needContinue)
						continue;
					exact = false;
					return std::pair<T *, const KeyType *>(pn, pk);
				}
			}

			template<class T>
			inline static T * next_node_(T * pn)
			{
				if (!pn)
					return nullptr;
				if (pn->dagNum_ > 0)
					return pn->dagBegin_;
				while (pn)
				{
					KeyType * pk = pn->keyBegin_;
					T * pp = pn->dagParent_;
					if (!pp)
						return nullptr;
					T * pf = pp->dagBegin_;
					T * pl = pf + (pp->dagNum_ - 1);
					T * pl1 = pl;
					search_children0_(pf, pl, pk);
					if (pl < pl1)
					{
						pl++;
						return pl;
					}
					pn = pp;
				}
				return pn;
			}

			template<class T>
			inline static T * next_vnode_(T * pn)
			{
				if (!pn)
					return nullptr;
				while (pn)
				{
					auto p = next_node_(pn);
					if (!p)
						return nullptr;
					if (p->valueId_ != NoValueId)
						return p;
					pn = p; //continue
				}
			}
			
			inline const ValueType * search_value_(const KeyType *pk) const
			{
				bool exact = false;
				auto pp = search_(root_(), pk, exact);
				if (!exact)
					return nullptr;
				return get_value_(pp.first);
			}
			inline ValueType * search_value_(const KeyType *pk)
			{
				bool exact = false;
				auto pp = search_(root_(), pk, exact);
				if (!exact)
					return nullptr;
				return get_value_(pp.first);
			}

		
			template<class V, class T, class U>
			class iterator_t_
			{
				friend class prefix_dag;
				T * pn_ = nullptr;
				U * po_ = nullptr;

				iterator_t_(T * p, U * o) : pn_(p), po_(o) {}
				inline bool check_() const
				{
					return pn_ && po_ && (pn_->valueId_ != NoValueId);
				}
				inline void assert_() const
				{
					if (!check_())
						throw PrefixDagInvalidIteratorException();
				}
			public:
				typedef ValueType value_type;
				typedef KeyType key_type;

				iterator_t_() : pn_(nullptr), po_(nullptr) {}
				iterator_t_(const iterator_t_ &t) : pn_(t.pn_), po_(t.po_) {}
				iterator_t_(iterator_t_ &&t) : pn_(t.pn_), po_(t.po_){}
				iterator_t_ &operator =(const iterator_t_ &t) { pn_ = t.pn_; po_ = t.po_; return *this; }
				iterator_t_ &operator =(iterator_t_ &&t) { pn_ = t.pn_; po_ = t.po_; return *this; }

				inline bool valid() const { return check_(); }
				
				inline uint32_t key_length() const { assert_(); return full_key_(pn_, nullptr, 0); }
				inline uint32_t key_fill(KeyType * buff, uint32_t buffSize) const
				{
					assert_();
					return full_key_(pn_, buff, buffSize);
				}
				inline std::basic_string<KeyType> key_string() const
				{
					assert_();
					auto len = full_key_(pn_, nullptr, 0);
					std::basic_string<KeyType> res(len, KeyType(0));
					full_key_(pn_, &res[0], len+1);
					return res;
				}


				inline ValueType &value() { assert_(); return *po_->get_value_(pn_); }
				inline const ValueType &value() const { assert_(); return *po_->get_value_(pn_); }
				
				inline iterator_t_<V,T,U> next(){ return iterator_t_<V, T, U>(next_vnode_(pn_), po_);	}
				inline iterator_t_<const V, const T, const U> next() const { return iterator_t_<const V, const T, const U>(next_vnode_(pn_), po_); }

				//prefix
				inline iterator_t_<V, T, U> &operator++() { pn_ = next_vnode_(pn_); return *this; }
				//postfix
				inline iterator_t_<V, T, U> operator++(int) { auto p = pn_; pn_ = next_vnode_(pn_); return iterator_t_<V, T, U>(p, po_); }

				inline V * operator->() { assert_(); return po_->get_value_(pn_); }
				inline const V * operator->() const { assert_(); return po_->get_value_(pn_); }

				inline V & operator*() { assert_(); return *po_->get_value_(pn_); }
				inline const V & operator*() const { assert_(); return *po_->get_value_(pn_); }


				template<class V2, class T2, class U2>
				inline bool operator ==(const iterator_t_<V2, T2, U2> &o) const
				{
					if (o.po_ != po_ || !po_ || !o.po_)
						return false;
					return pn_ == o.pn_;
				}
				template<class V2, class T2, class U2>
				inline bool operator !=(const iterator_t_<V2, T2, U2> &o) const
				{
					if (!po_ || !o.po_)
						return true;
					if (po_ != o.po_)
						return true;
					return pn_ != o.pn_;
				}
			};


			prefix_dag(const prefix_dag &d) = delete;
			prefix_dag(prefix_dag &&d) = delete;
			prefix_dag &operator =(const prefix_dag &d) = delete;
			prefix_dag &operator =(prefix_dag &&d) = delete;

			public:

			typedef iterator_t_<ValueType, prefix_dag_node_, object_type> iterator;
			typedef iterator_t_<const ValueType, const prefix_dag_node_, const object_type> const_iterator;

			~prefix_dag()
			{
				delete_values_recursive_(root_());
			}
			prefix_dag() : valueBuffTop_(0)
			{
				create_root_node_();
			}

			inline iterator begin() { return iterator(next_vnode_(root_()), this); }
			inline const_iterator begin() const { return const_iterator(next_vnode_(root_()), this); }
			inline iterator end() { return iterator(nullptr, this); }
			inline const_iterator end() const { return const_iterator(nullptr, this); }


			inline void add(const KeyType * key, ValueType &&val)
			{
				bool exact = false;
				auto pp = search_(root_(), key, exact);
				if (exact)
				{
					if (pp.first->valueId_ == NoValueId)
					{
						create_value_(pp.first, std::move(val));
						return;
					}
					throw PrefixDagDuplicateKeyException("prefix_dag::add(): key already exists!");
				}
				auto pn = pp.first;
				auto pk = pp.second;
				int match;
				uint32_t adv = match_(pn, pk, match);
				if (adv == 0)
				{
					auto pkl = pk;
					while (*pkl)pkl++;
					auto newp = add_node_(pn, pk, pkl - pk); 
					create_value_(newp, std::move(val));
					return;
				}
				auto psp = split_node_(pn, adv);
				if (match <= 0)
					create_value_(psp.first, std::move(val));
				else
				{
					auto pkl = pk + adv;
					while (*pkl)pkl++;
					auto newp = add_node_(psp.first, pk + adv, pkl - pk - adv); 
					create_value_(newp, std::move(val));
				}
			}
			inline void remove(const KeyType * key)
			{
				bool exact = false;
				auto pp = search_(root_(), key, exact);
				if (!exact)
					throw PrefixDagMissingKeyException("prefix_dag::remove(): key does not exist!");
				if(pp.first.valueId_ == NoValueId)
					throw PrefixDagMissingKeyException("prefix_dag::remove(): key does not exist!");
				delete_value_(pp.first);
			}
			inline bool exists(const KeyType * key) const
			{
				return search_value_(key);
			}
			inline ValueType &get(const KeyType * key)
			{
				auto p = search_value_(key);
				if (!p)
					throw PrefixDagMissingKeyException("prefix_dag::get(): key does not exist");
				return *p;
			}
			inline const ValueType &get(const KeyType * key) const
			{
				auto p = search_value_(key);
				if (!p)
					throw PrefixDagMissingKeyException("prefix_dag::get(): key does not exist");
				return *p;
			}
		};
	}
}