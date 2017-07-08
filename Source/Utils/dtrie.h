#pragma once
#include "log_config.h"
#include <string>

namespace zenith
{
	namespace util
	{
		class DTrieException : public std::exception
		{
		public:
			DTrieException() : std::exception("DTrieException: unknown cause") {}
			DTrieException(const char * p) : std::exception(p) {}
			virtual ~DTrieException() {}
		};
		class DTrieInvalidUseException : DTrieException
		{
		public:
			DTrieInvalidUseException() : DTrieException("DTrieInvalidUseException: invalid use!") {}
			DTrieInvalidUseException(const char * p) : DTrieException(p) {}
			virtual ~DTrieInvalidUseException() {}
		};
		class DTrieDuplicateKeyException : DTrieException
		{
		public:
			DTrieDuplicateKeyException() : DTrieException("DTrieDuplicateKeyException: key already exists!") {}
			DTrieDuplicateKeyException(const char * p) : DTrieException(p) {}
			virtual ~DTrieDuplicateKeyException() {}
		};
		class DTrieMissingKeyException : DTrieException
		{
		public:
			DTrieMissingKeyException() : DTrieException("DTrieMissingKeyException: key does not exist!") {}
			DTrieMissingKeyException(const char * p) : DTrieException(p) {}
			virtual ~DTrieMissingKeyException() {}
		};
		class DTrieOutOfMemException : DTrieException
		{
		public:
			DTrieOutOfMemException() : DTrieException("PrefixDagOutOfMemException: not enough memory to perform operation!") {}
			DTrieOutOfMemException(const char * p) : DTrieException(p) {}
			virtual ~DTrieOutOfMemException() {}
		};
		class DTrieInvalidIteratorException : DTrieException
		{
		public:
			DTrieInvalidIteratorException() : DTrieException("PrefixDagInvalidIteratorException: invalid iterator!") {}
			DTrieInvalidIteratorException(const char * p) : DTrieException(p) {}
			virtual ~DTrieInvalidIteratorException() {}
		};

		template<class KeyType>
		class dtrie_id
		{
		public:
			static const uint32_t NoValueId = 0xFFFFFFFF;

			typedef dtrie_id<KeyType> object_type;
			typedef uint32_t value_type;
			typedef KeyType key_element_type;
			typedef const KeyType * key_type;

			template<class K>
			class iterator_t_;
		private:
			struct dtrie_id_node_
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

				inline static dtrie_id_node_ empty() { return dtrie_id_node_(); }
			};
			static const uint32_t KeySize = sizeof(KeyType);
			static const uint32_t NodeSize = sizeof(dtrie_id_node_);

			/*
			Allocator is just two-way stack. On left side are keys, on right side are nodes.
			In case of resize everything is memcpy-able.
			In case of children resize -- new on-stack value and move everything past.
			*/

			uint8_t * bufferPtr_ = nullptr;
			uint32_t bufferSize_ = 0;
			uint8_t *stackLeft_, *stackRight_;
			uint32_t valueTop_ = 0;

			enum class _MatchRes { UNDEF = 0, EXACT = 1, MATCH_SHORT, MATCH_LONG, MISMATCH };
			enum class _SearchRes {UNDEF = 0, EXACT = 1, ADD_NODE, SPLIT_NODE, SPLIT_AND_ADD_NODE};

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
				uint32_t freeSize = static_cast<uint32_t>(stackRight_ - stackLeft_);
				if (new_size < bufferSize_ - freeSize)
					throw DTrieOutOfMemException("resize_buffer(): new_size is not enough to hold current data!");

				uint8_t * newBuffer = new uint8_t[new_size];
				uint32_t szLeft = static_cast<uint32_t>(stackLeft_ - bufferPtr_);
				uint32_t szRight = static_cast<uint32_t>(bufferPtr_ + bufferSize_ - stackRight_);
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
					throw DTrieInvalidUseException("dealloc_buffer_left(): pointer does not match buffer range.");
				if (ptr + sz > bufferPtr_ + bufferSize_)
					throw DTrieInvalidUseException("dealloc_buffer_left(): pointer does not match buffer range.");
				if (ptr + sz > stackLeft_)
					throw DTrieInvalidUseException("dealloc_buffer_left(): pointer and size are inconsistent with current buffer state.");
				if (ptr + sz == stackLeft_)
				{
					stackLeft_ -= sz;
					return;
				}
				//move data
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
					throw DTrieInvalidUseException("dealloc_buffer_right(): pointer does not match buffer range.");
				if (ptr + sz > bufferPtr_ + bufferSize_)
					throw DTrieInvalidUseException("dealloc_buffer_right(): pointer does not match buffer range.");
				if (stackRight_ > ptr)
					throw DTrieInvalidUseException("dealloc_buffer_right(): pointer and size are inconsistent with current buffer state.");
				if (ptr == stackRight_)
				{
					stackRight_ += sz;
					return;
				}
				uint8_t * p1 = ptr + sz;
				uint8_t * p2 = ptr;
				while (p2 > stackRight_)
				{
					uint32_t actSize = static_cast<uint32_t>(p2 - stackRight_);
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
				uint32_t req = NodeSize * num;
				uint8_t * p = alloc_buffer_right_(req, _Task::NEW_NODE);
				uint32_t rOff = static_cast<uint32_t>(bufferPtr_ + bufferSize_ - p);
				uint32_t res = rOff / NodeSize;
				if(res * NodeSize != rOff)
					throw DTrieInvalidUseException("alloc_node(): incorrect offset of node.");
				return res - 1;
			}
			inline void dealloc_node_(uint32_t ind, uint32_t num = 1)
			{
				if (num == 0)
					return;
				uint8_t * p = bufferPtr_ + bufferSize_ - NodeSize * (ind + 1);
				dtrie_id_node_ * pt = reinterpret_cast<dtrie_id_node_ *>(stackRight_);
				//update node-indices
				auto bufferEnd_ = reinterpret_cast<dtrie_id_node_ *>(bufferPtr_ + bufferSize_);
				for (auto pn = reinterpret_cast<dtrie_id_node_ *>(stackRight_); pn < bufferEnd_; pn++)
				{
					if (pn->dagBegin_ > ind - num)
						pn->dagBegin_ -= num;
					if (pn->dagParent_ > ind - num)
						pn->dagParent_ -= num;
				}
				dealloc_buffer_right_(p, NodeSize * num);
			}
			inline dtrie_id_node_ * node_ptr_(uint32_t ind) { return reinterpret_cast<dtrie_id_node_ *>(bufferPtr_ + bufferSize_ - NodeSize * (ind + 1)); }
			inline const dtrie_id_node_ * node_ptr_(uint32_t ind) const { return reinterpret_cast<const dtrie_id_node_ *>(bufferPtr_ + bufferSize_ - NodeSize * (ind + 1)); }
			inline uint32_t node_idx_(const dtrie_id_node_ *p) const { return static_cast<uint32_t>(bufferPtr_ + bufferSize_ - reinterpret_cast<const uint8_t *>(p)) / NodeSize - 1; }

			inline void reassign_children_parent_(uint32_t node)
			{
				auto ptr = node_ptr_(node);
				auto dags = node_ptr_(ptr->dagBegin_);
				for (uint32_t i = 0; i < ptr->dagNum_; i++)
					dags[i].dagParent_ = node;
			}

			//CHUNK: memory operation with keys
			inline uint32_t alloc_key_(uint32_t length)
			{
				auto res = alloc_buffer_left_(length * KeySize, _Task::NEW_KEY);
				return static_cast<uint32_t>(res - bufferPtr_) / KeySize;
			}
			inline void dealloc_key_(uint32_t ind, uint32_t length)
			{
				//update key-indices
				auto bufferEnd_ = reinterpret_cast<dtrie_id_node_ *>(bufferPtr_ + bufferSize_);
				for (auto pn = reinterpret_cast<dtrie_id_node_ *>(stackRight_); pn < bufferEnd_; pn++)
					if (pn->keyBegin_ > ind)
						pn->keyBegin_ -= length;

				dealloc_buffer_left_(bufferPtr_ + ind * KeySize, length * KeySize);
			}
			inline void copy_key_(KeyType * pd, const KeyType * ps, uint32_t len)
			{
				memcpy_s(pd, len * KeySize, ps, len * KeySize);
			}

			inline KeyType * key_ptr_(uint32_t ind) { return reinterpret_cast<KeyType *>(bufferPtr_ + KeySize * ind); }
			inline const KeyType * key_ptr_(uint32_t ind) const { return reinterpret_cast<const KeyType *>(bufferPtr_ + KeySize * ind); }
			inline uint32_t key_idx_(const KeyType *p) const { return static_cast<uint32_t>(reinterpret_cast<const uint8_t *>(p) - bufferPtr_) / KeySize; }


			//CHUNK: logical node operations
			inline dtrie_id_node_ * root_() { return reinterpret_cast<dtrie_id_node_ *>(bufferPtr_ + bufferSize_ - NodeSize); }
			inline const dtrie_id_node_ * root_() const { return reinterpret_cast<const dtrie_id_node_ *>(bufferPtr_ + bufferSize_ - NodeSize); }
			void create_root_node_()
			{
				uint32_t p = alloc_node_();
				if (p != 0)
					throw DTrieException("create_root_node_(): failed during root-node creation");
				*(node_ptr_(0)) = dtrie_id_node_::empty();
			}
			//technically alloc nodes / does not dealloc
			inline uint32_t add_node_tech_(uint32_t node_id, uint32_t numCur, uint32_t idxNew)
			{
				uint32_t numNew = numCur + 1;
				uint32_t startNew = alloc_node_(numNew);
				dtrie_id_node_ * pOld = node_ptr_(node_id);
				dtrie_id_node_ * pNew = node_ptr_(startNew);
				uint32_t j = 0;
				for (uint32_t i = 0; i < numNew; i++)
				{
					if (i == idxNew)
						continue;

					memcpy_s(pNew + i, NodeSize, pOld + j, NodeSize);

					reassign_children_parent_(startNew - i);
					pOld[j].keyNum_ = 0;
					pOld[j].keyBegin_ = NoValueId;
					pOld[j].valueId_ = NoValueId;
					j++;
				}

				return startNew;
			}
			inline void add_node_(uint32_t idx, const KeyType * k, uint32_t klen)
			{
				auto pNode = node_ptr_(idx);
				uint32_t oldNum = pNode->dagNum_;
				uint32_t oldDag = pNode->dagBegin_;
				uint32_t newNum = oldNum + 1;
				uint32_t idxNew = 0;
				if (oldNum != 0)
				{
					auto pf = node_ptr_(oldDag);
					auto pl = pf + (oldNum - 1);
					search_children0_(this, pf, pl, k);
					if (!pf)
						idxNew = 0;
					else if (!pl)
						idxNew = oldNum;
					else
						idxNew = uint32_t(pl - node_ptr_(pNode->dagBegin_));
				}
				uint32_t tmpId = add_node_tech_(oldDag, oldNum, idxNew);
				pNode = node_ptr_(idx);
				pNode->dagBegin_ = tmpId;
				pNode->dagNum_ = newNum;

				dtrie_id_node_ * p = node_ptr_(pNode->dagBegin_) + idxNew; //new node
				*p = dtrie_id_node_::empty();
				p->keyBegin_ = alloc_key_(klen);
				p->keyNum_ = klen;
				copy_key_(key_ptr_(p->keyBegin_), k, klen);
				p->dagParent_ = idx;

				//all indices are invalidated after here:
				dealloc_node_(oldDag, oldNum);
			}

			inline uint32_t split_node_(uint32_t nodeIdx, uint32_t klen)
			{
				uint32_t newNodeIdx = alloc_node_();
				dtrie_id_node_ * nn = node_ptr_(newNodeIdx);
				dtrie_id_node_ * n = node_ptr_(nodeIdx);
				*nn = dtrie_id_node_::empty();
				uint32_t leftLen = n->keyNum_ - klen;

				n->keyNum_ = klen;
				nn->keyNum_ = leftLen;
				nn->keyBegin_ = n->keyBegin_ + klen;

				nn->dagBegin_ = n->dagBegin_;
				nn->dagNum_ = n->dagNum_;

				nn->dagParent_ = nodeIdx;

				n->dagBegin_ = newNodeIdx;
				n->dagNum_ = 1;

				nn->valueId_ = n->valueId_;
				n->valueId_ = NoValueId;

				reassign_children_parent_(newNodeIdx);

				return newNodeIdx;
			}
			//searches node
			template<class T>
			inline static uint32_t full_key_length_(T * pn)
			{
				uint32_t res = 0;
				while (pn)
				{
					res += pn->keyNum_;
					pn = node_ptr_(pn->dagParent_);
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
					ptmp = node_ptr_(ptmp->dagParent_);
				}
				if (res >= buffSize || !buff)
					return res;

				uint32_t idx = res;
				while (pn)
				{
					idx -= pn->keyNum_;
					auto lkp = key_ptr_(pn->keyBegin_);
					for (uint32_t i = 0; i < pn->keyNum_; i++)
						buff[idx + i] = lkp[i];
					pn = node_ptr_(pn->dagParent_);
				}
				return res;
			}

			template<class U, class T>
			inline static uint32_t match_(U * po, T * pn, const KeyType * pk, _MatchRes &matchres)
			{
				matchres = _MatchRes::UNDEF;
				if (!pn || !pk)
					return 0;
				const KeyType * pnk = po->key_ptr_(pn->keyBegin_);
				for (uint32_t i = 0; i < pn->keyNum_; i++)
				{
					if (*pk != pnk[i])
					{
						if (!*pk)
							matchres = _MatchRes::MATCH_SHORT;
						else
							matchres = _MatchRes::MISMATCH;
						return i;
					}
					pk++;
				}
				if (*pk)
					matchres = _MatchRes::MATCH_LONG;
				else
					matchres = _MatchRes::EXACT;
				return pn->keyNum_;
			}
			//searches children by first char
			template<class T, class U>
			inline static void search_children0_(U * po, T * &first, T * &last, const KeyType * pk)
			{
				if (!first || !last)
					return;
				if (first > last)
					throw DTrieInvalidUseException("dtrie_id::search_children(): expecting last >= first");
				if (*po->key_ptr_(first->keyBegin_) > *pk)
				{
					last = first;
					first = nullptr;
					return;
				}
				else if (*po->key_ptr_(first->keyBegin_) == *pk)
				{
					last = first;
					return;
				}

				if (*po->key_ptr_(last->keyBegin_) < *pk)
				{
					first = last;
					last = nullptr;
					return;
				}
				else if (*po->key_ptr_(last->keyBegin_) == *pk)
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
					if (*po->key_ptr_(mid->keyBegin_) == *pk)
					{
						first = mid;
						last = mid;
						return;
					}
					if (*po->key_ptr_(mid->keyBegin_) < *pk)
						first = mid;
					else
						last = mid;
				}
			}

			/*
			options:
			1) found exact node
			2) need new node (to continue)
			3) need split node
			4) need split node and new node
			*/

			//searches node
			template<class T, class U>
			inline static std::pair<T *, const KeyType *> search_(U * po, T * pn, const KeyType * pk, _SearchRes &sres)
			{
				sres = _SearchRes::UNDEF;
				while (1)
				{
					if (!pn || !pk)
					{
						sres = _SearchRes::UNDEF;
						return std::pair<T *, const KeyType *>(nullptr, nullptr);
					}
					if (!*pk)
					{
						sres = _SearchRes::EXACT;
						return std::pair<T *, const KeyType *>(pn, pk);
					}
					bool needContinue = false;

					if (pn->dagNum_ > 0)
					{
						_MatchRes match = _MatchRes::UNDEF;
						uint32_t iadv = 0;
						dtrie_id_node_ * pf = po->node_ptr_(pn->dagBegin_);
						auto pl = pf + (pn->dagNum_ - 1);
						search_children0_(po, pf, pl, pk);
						if (pf == pl)
						{
							iadv = match_(po, pf, pk, match);
							if (match == _MatchRes::MATCH_LONG)
							{
								pk += iadv;
								pn = pf;
							}
							else
							{
								if (match == _MatchRes::EXACT)
								{
									sres = _SearchRes::EXACT;
									return std::pair<T *, const KeyType *>(pf, pk);
								}
								else if (match == _MatchRes::MATCH_SHORT)
								{
									sres = _SearchRes::SPLIT_NODE;
									return std::pair<T *, const KeyType *>(pf, pk);
								}
								else if (match == _MatchRes::MISMATCH)
								{
									sres = _SearchRes::SPLIT_AND_ADD_NODE;
									return std::pair<T *, const KeyType *>(pf, pk);
								}
							}
						}
						else
						{
							sres = _SearchRes::ADD_NODE;
							return std::pair<T *, const KeyType *>(pn, pk);
						}
					}
					else
					{
						sres = _SearchRes::ADD_NODE;
						return std::pair<T *, const KeyType *>(pn, pk);
					}
				}
			}

			template<class T>
			inline static T * next_node_(T * pn)
			{
				if (!pn)
					return nullptr;
				if (pn->dagNum_ > 0)
					return node_ptr_(pn->dagBegin_);
				while (pn)
				{
					KeyType * pk = key_ptr_(pn->keyBegin_);
					T * pp = node_ptr_(pn->dagParent_);
					if (!pp)
						return nullptr;
					T * pf = node_ptr_(pp->dagBegin_);
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

			inline uint32_t search_value_(const KeyType *pk) const
			{
				_SearchRes sr;
				auto pp = search_(this, root_(), pk, sr);
				if (sr != _SearchRes::EXACT)
					return NoValueId;
				return get_value_(pp.first);
			}
			inline uint32_t search_value_(const KeyType *pk)
			{
				_SearchRes sr;
				auto pp = search_(this, root_(), pk, sr);
				if (sr != _SearchRes::EXACT)
					return NoValueId;
				return pp.first->valueId_;
			}

			/*
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

			*/
			dtrie_id(const dtrie_id &d) = delete;
			dtrie_id(dtrie_id &&d) = delete;
			dtrie_id &operator =(const dtrie_id &d) = delete;
			dtrie_id &operator =(dtrie_id &&d) = delete;

		public:

			//typedef iterator_t_<ValueType, dtrie_id_node_, object_type> iterator;
			//typedef iterator_t_<const ValueType, const dtrie_id_node_, const object_type> const_iterator;

			~dtrie_id()
			{
				delete[] bufferPtr_;
			}
			dtrie_id(uint32_t capacityNodes = 5, uint32_t capactityKeys = 100)
				: bufferPtr_(nullptr), bufferSize_(0), valueTop_(0)
			{
				resize_buffer_(capacityNodes * NodeSize + capactityKeys * KeySize);
				create_root_node_();
			}
			/*
			inline iterator begin() { return iterator(next_vnode_(root_()), this); }
			inline const_iterator begin() const { return const_iterator(next_vnode_(root_()), this); }
			inline iterator end() { return iterator(nullptr, this); }
			inline const_iterator end() const { return const_iterator(nullptr, this); }
			*/

			inline uint32_t add(const KeyType * key)
			{
				_SearchRes sr;
				auto pp = search_(this, root_(), key, sr);
				if (sr == _SearchRes::EXACT)
				{
					if (pp.first->valueId_ == NoValueId)
					{
						pp.first->valueId_ = valueTop_++;
						return pp.first->valueId_;
					}
					throw DTrieDuplicateKeyException("dtrie_id::add(): key already exists!");
				}
				else
				{
					auto pn = pp.first;
					uint32_t pnId = node_idx_(pn);
					auto pk = pp.second;

					if (sr == _SearchRes::SPLIT_NODE)
					{
						_MatchRes mr;
						uint32_t adv = match_(this, pn, pk, mr);
						split_node_(pnId, adv);
						auto rp = node_ptr_(pnId);
						rp->valueId_ = valueTop_++;
						return rp->valueId_;
					}
					else if (sr == _SearchRes::SPLIT_AND_ADD_NODE)
					{
						_MatchRes mr;
						uint32_t adv = match_(this, pn, pk, mr);
						auto pkl = pk; while (*pkl++);pkl--;
						split_node_(pnId, adv);
						add_node_(pnId, pk + adv, uint32_t(pkl - pk - adv));
						_SearchRes sloc;
						auto pp = search_(this, root_(), key, sloc);
						if (sloc != _SearchRes::EXACT || pp.first->valueId_ != NoValueId)
							throw DTrieInvalidUseException("dtrie_id::add(): invalid add operation!");
						pp.first->valueId_ = valueTop_++;
						return pp.first->valueId_;
					}
					else if (sr == _SearchRes::ADD_NODE)
					{
						auto pkl = pk; while (*pkl++);pkl--;
						add_node_(pnId, pk, uint32_t(pkl - pk));
						_SearchRes sloc;
						auto pp = search_(this, root_(), key, sloc);
						if (sloc != _SearchRes::EXACT || pp.first->valueId_ != NoValueId)
							throw DTrieInvalidUseException("dtrie_id::add(): invalid add operation!");
						pp.first->valueId_ = valueTop_++;
						return pp.first->valueId_;
					}
					else throw DTrieInvalidUseException("dtrie_id::add(): invalid results!");
				}
			}
			inline void remove(const KeyType * key)
			{
				_SearchRes sr;
				auto pp = search_(this, root_(), key, sr);
				if (sr != _SearchRes::EXACT)
					throw DTrieMissingKeyException("dtrie_id::remove(): key does not exist!");
				if (pp.first->valueId_ == NoValueId)
					throw DTrieMissingKeyException("dtrie_id::remove(): key does not exist!");
				pp.first->valueId_ = NoValueId;
			}
			inline bool exists(const KeyType * key) const
			{
				return search_value_(key) != NoValueId;
			}
			inline uint32_t get(const KeyType * key)
			{
				return search_value_(key);
			}
			inline uint32_t get(const KeyType * key) const
			{
				return search_value_(key);
			}
		};
	}
}