#pragma once
#include "log_config.h"
#include <string>

namespace zenith
{
	namespace util
	{
		class DynamicPDAGException : public std::exception
		{
		public:
			DynamicPDAGException() : std::exception("DynamicPDAGException: unknown cause") {}
			DynamicPDAGException(const char * p) : std::exception(p) {}
			virtual ~DynamicPDAGException() {}
		};
		class DynamicPDAGInvalidUseException : DynamicPDAGException
		{
		public:
			DynamicPDAGInvalidUseException() : DynamicPDAGException("DynamicPDAGInvalidUseException: invalid use!") {}
			DynamicPDAGInvalidUseException(const char * p) : DynamicPDAGException(p) {}
			virtual ~DynamicPDAGInvalidUseException() {}
		};
		class DynamicPDAGDuplicateKeyException : DynamicPDAGException
		{
		public:
			DynamicPDAGDuplicateKeyException() : DynamicPDAGException("DynamicPDAGDuplicateKeyException: key already exists!") {}
			DynamicPDAGDuplicateKeyException(const char * p) : DynamicPDAGException(p) {}
			virtual ~DynamicPDAGDuplicateKeyException() {}
		};
		class DynamicPDAGMissingKeyException : DynamicPDAGException
		{
		public:
			DynamicPDAGMissingKeyException() : DynamicPDAGException("DynamicPDAGMissingKeyException: key does not exist!") {}
			DynamicPDAGMissingKeyException(const char * p) : DynamicPDAGException(p) {}
			virtual ~DynamicPDAGMissingKeyException() {}
		};
		class DynamicPDAGOutOfMemException : DynamicPDAGException
		{
		public:
			DynamicPDAGOutOfMemException() : DynamicPDAGException("PrefixDagOutOfMemException: not enough memory to perform operation!") {}
			DynamicPDAGOutOfMemException(const char * p) : DynamicPDAGException(p) {}
			virtual ~DynamicPDAGOutOfMemException() {}
		};
		class DynamicPDAGInvalidIteratorException : DynamicPDAGException
		{
		public:
			DynamicPDAGInvalidIteratorException() : DynamicPDAGException("PrefixDagInvalidIteratorException: invalid iterator!") {}
			DynamicPDAGInvalidIteratorException(const char * p) : DynamicPDAGException(p) {}
			virtual ~DynamicPDAGInvalidIteratorException() {}
		};

		template<class KeyType>
		class dynamic_pdag
		{
		public:
			static const uint32_t NoValueId = 0xFFFFFFFF;

			typedef dynamic_pdag<KeyType> object_type;
			typedef uint32_t value_type;
			typedef KeyType key_element_type;
			typedef const KeyType * key_type;

			template<class K>
			class iterator_t_;
		private:
			struct dynamic_pdag_node_
			{
				template<class K>
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
				uint32_t keyBegin_ = NoValueId;
				uint32_t dagBegin_ = NoValueId;
				uint32_t dagParent_ = NoValueId;
				uint32_t keyNum_ = 0;
				uint32_t dagNum_ = 0;
				uint32_t valueId_ = NoValueId;

				inline static dynamic_pdag_node_ empty() { return dynamic_pdag_node_(); }
			};
			static const size_t KeySize = sizeof(KeyType);
			static const size_t NodeSize = sizeof(dynamic_pdag_node_);

			/*
			Allocator is just two-way stack. On left side are keys, on right side are nodes.
			In case of resize everything is memcpy-able.
			In case of children resize -- new on-stack value and move everything past.
			*/

			uint8_t * bufferPtr_ = nullptr;
			uint32_t bufferSize_ = 0;
			uint8_t *stackLeft_, *stackRight_;

			//LOW-LEVEL MEMORY OPERATIONS:
			//1. new and delete of buffer
			enum class _Task {UNDEF = 0, NEW_KEY, NEW_NODE, MOVE_NODES};

			inline void delete_buffer_()
			{
				if (bufferPtr_)
					delete[] bufferPtr_;
				bufferPtr_ = nullptr;
				bufferSize_ = 0;
				stackLeft_ = nullptr; stackRight_ = nullptr;
			}
			inline void create_buffer_(uint32_t size)
			{
				delete_buffer_();
				bufferPtr_ = new uint8_t[size];
				bufferSize_ = size;
				stackLeft_ = bufferPtr_;
				stackRight_ = bufferPtr_ + size;
			}
			inline void resize_buffer_(uint32_t new_size)
			{
				if (!bufferPtr_)
				{
					create_buffer_(new_size);
					return;
				}
				uint32_t freeSize = stackRight_ - stackLeft_;
				if (new_size < bufferSize_ - freeSize)
					throw DynamicPDAGOutOfMemException("resize_buffer(): new_size is not enough to hold current data!");

				uint8_t newBuffer = new uint8_t[new_size];
				uint32_t szLeft = stackLeft_ - bufferPtr_;
				uint32_t szRight = bufferPtr_ + bufferSize_ - stackRight_;
				uint32_t offRight = new_size - szRight;
				memcpy_s(newBuffer, szLeft, bufferPtr_, szLeft);
				memcpy_s(newBuffer + offRight, szRight, stackRight_, szRight);
				delete[] bufferPtr_;
				bufferPtr_ = newBuffer;
				bufferSize_ = new_size;
				stackLeft_ = bufferPtr_ + szLeft;
				stackRight_ = bufferPtr_ + bufferSize_ - szRight;
			}
			inline void manage_buffer_resize_(uint32_t minReqSize, _Task task)
			{
				resize_buffer_(minReqSize);
			}
			//2. alloc and dealloc from buffer
			inline uint8_t * alloc_buffer_left_(uint32_t sz, _Task task = _Task::UNDEF)
			{
				if (stackLeft_ + sz > stackRight_)
					resize_buffer_(bufferSize_ + sz);
				uint8_t * res = stackLeft_;
				stackLeft_ += sz;
				return res;
			}
			inline uint8_t * alloc_buffer_right_(uint32_t sz, _Task task = _Task::UNDEF)
			{
				if (stackLeft_ + sz > stackRight_)
					resize_buffer_(bufferSize_ + sz);
				stackRight_ -= sz;				
				return stackRight_;
			}
			inline void dealloc_buffer_left_(uint8_t * ptr, uint32_t sz)
			{
				if(ptr < bufferPtr_)
					throw DynamicPDAGInvalidUseException("dealloc_buffer_left(): pointer does not match buffer range.");
				if (ptr + sz > bufferPtr_ + bufferSize_)
					throw DynamicPDAGInvalidUseException("dealloc_buffer_left(): pointer does not match buffer range.");
				if (ptr + sz > stackLeft_)
					throw DynamicPDAGInvalidUseException("dealloc_buffer_left(): pointer and size are inconsistent with current buffer state.");
				if (ptr + sz == stackLeft_)
				{
					stackLeft_ -= sz;
					return;
				}
				uint8_t * p2 = ptr + sz;
				while (p2 < stackLeft_)
				{
					uint32_t actSize = stackLeft_ - p2;
					if (actSize > sz)
						actSize = sz;
					memcpy_s(ptr, actSize, p2, actSize);
					ptr += actSize;
					p2 += actSize;
				}
				stackLeft_ -= sz;
			}
			inline void dealloc_buffer_right_(uint8_t * ptr, uint32_t sz)
			{
				if (ptr < bufferPtr_)
					throw DynamicPDAGInvalidUseException("dealloc_buffer_right(): pointer does not match buffer range.");
				if (ptr + sz > bufferPtr_ + bufferSize_)
					throw DynamicPDAGInvalidUseException("dealloc_buffer_right(): pointer does not match buffer range.");
				if (stackRight_ > ptr)
					throw DynamicPDAGInvalidUseException("dealloc_buffer_right(): pointer and size are inconsistent with current buffer state.");
				if (ptr == stackRight_)
				{
					stackRight_ += sz;
					return;
				}
				uint8_t * p1 = ptr + sz;
				uint8_t * p2 = ptr;
				while (p2 > stackRight_)
				{
					uint32_t actSize = p2 - stackRight_;
					if (actSize > sz)
						actSize = sz;
					memcpy_s(p1 - actSize, actSize, p2 - actSize, actSize);
					p1 -= actSize;
					p2 -= actSize;
				}
				stackRight_ += sz;
			}
			//CHUNK: memory operation with nodes
			//negative index
			inline uint32_t alloc_node_(uint32_t num = 1)
			{
				auto req = NodeSize * num;
				uint8_t * p = alloc_buffer_right_(req, _Task::NEW_NODE);
				uint32_t rOff = bufferPtr_ + bufferSize_ - p;
				uint32_t res = rOff / NodeSize;
				if(res * NodeSize != rOff)
					throw DynamicPDAGInvalidUseException("alloc_node(): incorrect offset of node.");
				return res - 1;
			}
			inline void dealloc_node_(uint32_t ind, uint32_t num = 1)
			{
				uint8_t * p = bufferPtr_ + bufferSize_ - NodeSize * (ind + 1);
				dynamic_pdag_node_ * pt = stackRight_;
				while (pt < bufferPtr_ + bufferSize_)
				{
					//if(pt->)
					pt++;
				}
				dealloc_buffer_right_(p, NodeSize * num);
			}
			inline dynamic_pdag_node_ * node_ptr_(uint32_t ind) { return reinterpret_cast<dynamic_pdag_node_ *>(bufferPtr_ + bufferSize_ - NodeSize * (ind + 1)); }
			inline const dynamic_pdag_node_ * node_ptr_(uint32_t ind) const { return reinterpret_cast<const dynamic_pdag_node_ *>(bufferPtr_ + bufferSize_ - NodeSize * (ind + 1)); }
			inline uint32_t node_idx_(const dynamic_pdag_node_ *p) const { return (bufferPtr_ + bufferSize_ - reinterpret_cast<const uint8_t *>(p)) / NodeSize - 1; }

			inline void reassign_children_parent_(dynamic_pdag_node_ * node, dynamic_pdag_node_ * new_parent)
			{
				uint32_t idx = node_idx_(new_parent);
				for (uint32_t i = 0; i < node->dagNum_; i++)
					node_ptr_(node->dagBegin_)[i].dagParent_ = idx;
			}

			//CHUNK: memory operation with keys
			inline uint32_t alloc_key_(uint32_t length)
			{
				auto res = alloc_buffer_left_(length * KeySize, _Task::NEW_KEY);
				return (res - bufferPtr_) / KeySize;
			}
			inline void dealloc_key_(uint32_t ind, uint32_t length)
			{
				throw DynamicPDAGInvalidUseException("dealloc_key(): not supported.");
				dealloc_buffer_left_(bufferPtr_ + ind * KeySize, length * KeySize);
			}
			inline void copy_key_(KeyType * pd, const KeyType * ps, uint32_t len)
			{
				memcpy_s(pd, len * KeySize, ps, len * KeySize);
			}

			inline KeyType * key_ptr_(uint32_t ind) { return reinterpret_cast<KeyType *>(bufferPtr_ + KeySize * ind); }
			inline const KeyType * key_ptr_(uint32_t ind) const { return reinterpret_cast<const KeyType *>(bufferPtr_ + KeySize * ind); }
			inline uint32_t key_idx_(const KeyType *p) const { return (reinterpret_cast<const uint8_t *>(p) - bufferPtr_) / KeySize; }


			//CHUNK: logical node operations
			inline dynamic_pdag_node_ * root_() { return reinterpret_cast<dynamic_pdag_node_ *>(bufferPtr_ + bufferSize_ - NodeSize); }
			inline const dynamic_pdag_node_ * root_() const { return reinterpret_cast<const dynamic_pdag_node_ *>(bufferPtr_ + bufferSize_ - NodeSize); }
			void create_root_node_()
			{
				uint32_t p = alloc_node_();
				if (p != 0)
					throw DynamicPDAGException("create_root_node_(): failed during root-node creation");
				*(node_ptr_(0)) = dynamic_pdag_node_::empty();
			}
			//technically realloc nodes
			inline dynamic_pdag_node_ * add_node_tech_(dynamic_pdag_node_ * p, uint32_t numCur, uint32_t idxNew)
			{
				uint32_t numNew = numCur + 1;
				uint32_t startNew = alloc_node_(numNew);
				dynamic_pdag_node_ * n = node_ptr_(startNew);
				uint32_t j = 0;
				for (uint32_t i = 0; i < numNew; i++)
				{
					if (i == idxNew)
						continue;

					memcpy_s(n + i, NodeSize, p + j, NodeSize);

					reassign_children_parent_(n + i, n + i);
					p[j].keyNum_ = 0;
					p[j].keyBegin_ = nullptr;
					p[j].valueId_ = NoValueId;
					j++;
				}

				dealloc_node_(node_idx_(p), numCur);
				return n;
			}
			inline dynamic_pdag_node_ * add_node_(dynamic_pdag_node_ * n, const KeyType * k, uint32_t klen)
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

				dynamic_pdag_node_ * p = n->dagBegin_ + idxNew; //new node
				p->keyBegin_ = alloc_key_(klen);
				p->keyNum_ = klen;
				copy_key_(p->keyBegin_, k, klen);

				p->dagBegin_ = nullptr;
				p->dagNum_ = 0;
				p->valueId_ = NoValueId;
				p->dagParent_ = n;
				return p;
			}

			inline std::pair<dynamic_pdag_node_ *, dynamic_pdag_node_ *> split_node_(dynamic_pdag_node_ * n, uint32_t klen)
			{
				dynamic_pdag_node_ * nn = new_node_();
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

				return std::pair<dynamic_pdag_node_ *, dynamic_pdag_node_ *>(n, nn);
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
				}
				else if (*last->keyBegin_ == *pk)
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
				iterator_t_(iterator_t_ &&t) : pn_(t.pn_), po_(t.po_) {}
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
					full_key_(pn_, &res[0], len + 1);
					return res;
				}


				inline ValueType &value() { assert_(); return *po_->get_value_(pn_); }
				inline const ValueType &value() const { assert_(); return *po_->get_value_(pn_); }

				inline iterator_t_<V, T, U> next() { return iterator_t_<V, T, U>(next_vnode_(pn_), po_); }
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

			typedef iterator_t_<ValueType, dynamic_pdag_node_, object_type> iterator;
			typedef iterator_t_<const ValueType, const dynamic_pdag_node_, const object_type> const_iterator;

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
				if (pp.first.valueId_ == NoValueId)
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