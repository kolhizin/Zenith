#pragma once
#include "Memory\alloc_virtual.h"

namespace zenith
{
	namespace util
	{
		template<class KeyType, class ValueType, size_t MaxValues = 10, size_t MaxKeys = 100>
		class prefix_dag
		{
			static const uint32_t NoValueId = 0xFFFFFFFF;
			struct prefix_dag_node_
			{
				KeyType * keyBegin_ = nullptr;
				uint32_t keyNum_ = 0;

				prefix_dag_node_ * dagBegin_ = nullptr;
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

			prefix_dag_node_ * alloc_node_(uint32_t num = 1)
			{
				auto req = NodeSize * num;
				auto blk = allocNode_.allocate(req);
				if(blk.size < req)
					throw std::out_of_range("prefix_dag::alloc_node_(): out of memory");
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
			inline void copy_node_(prefix_dag_node_ * pd, const prefix_dag_node_ * ps)
			{
				pd->keyBegin_ = ps->keyBegin_;
				pd->keyNum_ = ps->keyNum_;
				pd->dagBegin_ = ps->dagBegin_;
				pd->dagNum_ = ps->dagNum_;
				pd->valueId_ = ps->valueId_;
			}

			KeyType * alloc_key_(uint32_t length)
			{
				auto blk = allocKey_.allocate(length * KeySize);
				if (blk.size < length * KeySize)
					throw std::out_of_range("prefix_dag::alloc_key_(): out of memory");
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

			uint32_t alloc_value_()
			{
				auto tmp = valueBuffTop_++;
				if (valueBuffTop_ > MaxValues)
					throw std::out_of_range("prefix_dag::alloc_value_(): out of memory");
				return tmp;
			}
			inline ValueType * get_value_(uint32_t id) { return reinterpret_cast<ValueType *>(&valueBuff_[id * ValueSize]); }
			inline const ValueType * get_value_(uint32_t id) const { return reinterpret_cast<const ValueType *>(&valueBuff_[id * ValueSize]); }

			inline void delete_values_recursive_(prefix_dag_node_ * n)
			{
				if (n->valueId_ != NoValueId)
				{
					get_value_(n->valueId_)->~ValueType();
					n->valueId_ = NoValueId;
				}
				for (uint32_t i = 0; i < n->dagNum_; i++)
					delete_values_recursive_(n->dagBegin_ + i);
			}
			inline uint32_t new_value_(ValueType &&v)
			{
				uint32_t id = alloc_value_();
				new (get_value_(id)) ValueType(std::move(v));
				return id;
			}

			inline prefix_dag_node_ * root_() { return reinterpret_cast<prefix_dag_node_ *>(&nodeBuff_[0]); }
			inline const prefix_dag_node_ * root_() const { return reinterpret_cast<const prefix_dag_node_ *>(&nodeBuff_[0]); }
			void create_root_node_()
			{
				auto p = alloc_node_();
				if (p != root_())
				{
					dealloc_node_(p);
					throw std::logic_error("prefix_dag::create_root_node_(): failed during root-node creation!");
				}
				new (p) prefix_dag_node_();
			}

			inline prefix_dag_node_ * resize_nodes_(prefix_dag_node_ * p, uint32_t numCur, uint32_t numNew)
			{
				uint32_t numCopy = (numCur > numNew ? numNew : numCur);
				if (numNew == 0)
				{
					delete_nodes_(p, numCur);
					return nullptr;
				}
				
				prefix_dag_node_ * n = new_nodes_(numNew);
				for (uint32_t i = 0; i < numCopy; i++)
				{
					copy_node_(n + i, p + i);
					p[i].keyNum_ = 0;
					p[i].keyBegin_ = nullptr;
					p[i].valueId_ = NoValueId;
				}
				for (uint32_t i = numCopy; i < numCur; i++)
				{
					auto valId = p[i].valueId_;
					get_value_(valId)->~ValueType(); //but do not free
					p[i].valueId_ = NoValueId;
				}

				delete_nodes_(p, numCur);
				return n;
			}


			
			inline void create_value_(prefix_dag_node_ * p, ValueType &&v)
			{
				if (p->valueId_ != NoValueId)
					throw std::runtime_error("prefix_dag::create_value: value already exists!");
				p->valueId_ = new_value_(std::move(v));
			}
			inline void delete_value_(prefix_dag_node_ * p)
			{
				if (p->valueId_ == NoValueId)
					throw std::runtime_error("prefix_dag::delete_value: value does not exist!");
				get_value_(p->valueId_)->~ValueType();
				p->valueId_ = NoValueId;
			}
			inline ValueType * get_value_(prefix_dag_node_ * p)
			{
				if (p->valueId_ == NoValueId)
					return nullptr;
				return get_value_(p->valueId_);
			}
			inline const ValueType * get_value_(prefix_dag_node_ * p) const
			{
				if (p->valueId_ == NoValueId)
					return nullptr;
				return get_value_(p->valueId_);
			}
			inline prefix_dag_node_ * attach_node_(prefix_dag_node_ * n, const KeyType * k, uint32_t klen)
			{
				uint32_t oldNum = n->dagNum_;
				uint32_t newNum = oldNum + 1;
				n->dagBegin_ = resize_nodes_(n->dagBegin_, oldNum, newNum);
				n->dagNum_ = newNum;

				prefix_dag_node_ * p = n->dagBegin_ + oldNum; //new node
				p->keyBegin_ = alloc_key_(klen);
				p->keyNum_ = klen;
				copy_key_(p->keyBegin_, k, klen);

				p->dagBegin_ = nullptr;
				p->dagNum_ = 0;
				p->valueId_ = NoValueId;
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

				return std::pair<prefix_dag_node_ *, prefix_dag_node_ *>(n, nn);
			}
			inline uint32_t check_node_key_final_(const prefix_dag_node_ * n, const KeyType * pk, bool &finalNode) const
			{
				finalNode = false;
				uint32_t i = 0;
				const KeyType * p0 = n->keyBegin_;
				for (;i < n->keyNum_; i++)
				{
					if (!*pk)
					{
						finalNode = true;
						return i;
					}
					if (*pk++ == *p0++)
						continue;
					finalNode = true;
					return i;
				}
				return i;
			}
			inline uint32_t check_node_key_exact_(const prefix_dag_node_ * n, const KeyType * pk, bool &exact) const
			{
				exact = true;
				uint32_t i = 0;
				const KeyType * p0 = n->keyBegin_;
				for (;i < n->keyNum_; i++)
				{
					if (!*pk)
						return i;
					if (*pk++ == *p0++)
						continue;
					exact = false;
					return i;
				}
				return i;
			}
			inline std::pair<prefix_dag_node_ *, const KeyType *> search_(prefix_dag_node_ * pn, const KeyType * pk, bool &exact)
			{
				if (!pn || !pk)
				{
					exact = false;
					return std::pair<prefix_dag_node_ *, const KeyType *>(nullptr, nullptr);
				}
				if (!*pk)
				{
					exact = true;
					return std::pair<prefix_dag_node_ *, const KeyType *>(pn, pk);
				}

				bool finalNode = false;
				for (uint32_t i = 0; i < pn->dagNum_; i++)
				{
					uint32_t iadv = check_node_key_final_(&pn->dagBegin_[i], pk, finalNode);
					if (iadv == 0)
						continue;
					if (!finalNode)
						return search_(&pn->dagBegin_[i], pk + iadv, exact);
					//final node
					exact = (iadv == pn->dagBegin_[i].keyNum_);
					return std::pair<prefix_dag_node_ *, const KeyType *>(&pn->dagBegin_[i], pk);
				}
				exact = false;
				return std::pair<prefix_dag_node_ *, const KeyType *>(pn, pk);
			}

			prefix_dag(const prefix_dag &);
			prefix_dag(prefix_dag &&);
		public:
			~prefix_dag()
			{
				delete_values_recursive_(root_());
			}
			prefix_dag() : valueBuffTop_(0)
			{
				create_root_node_();
			}
			inline void add(const KeyType * key, ValueType &&val)
			{
				bool exact = false;
				auto pp = search_(root_(), key, exact);
				if (exact)
				{
					std::cout << "ERROR: prefix_dag::add(): key already exists!";
					return;
				}
				auto pn = pp.first;
				auto pk = pp.second;
				exact = true;
				uint32_t adv = check_node_key_exact_(pn, pk, exact);
				if (adv == 0)
				{
					auto pkl = pk;
					while (*pkl)pkl++;
					auto newp = attach_node_(pn, pk, pkl - pk);
					create_value_(newp, std::move(val));
					return;
				}
				auto psp = split_node_(pn, adv);
				if (exact)
					create_value_(psp.first, std::move(val));
				else
				{
					auto pkl = pk + adv;
					while (*pkl)pkl++;
					auto newp = attach_node_(psp.first, pk + adv, pkl - pk - adv);
					create_value_(newp, std::move(val));
				}
			}
			inline ValueType &get(const KeyType * key)
			{
				bool exact = false;
				auto pp = search_(root_(), key, exact);
				if (!exact)
				{
					std::cout << "ERROR: prefix_dag::get(): key does not exist!";
					return *reinterpret_cast<ValueType*>(&valueBuff_[0]);
				}
				return *get_value_(pp.first);
			}
		};
	}
}