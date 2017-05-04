#pragma once

#include "manager_base.h"
#include <vector>
#include <list>
#include <limits>
#include <algorithm>

namespace zenith
{
	namespace util
	{
		namespace manager
		{
			template<class Object, class ObjectDesc, class GlobalArg>
			class ManagerUnique_Collection_
			{
				std::list<Object> objs_; //objects
				GlobalArg gArg_;
			protected:
				ManagerUnique_Collection_(const GlobalArg &ga) : gArg_(ga)
				{
				}
				inline Object * construct_(const ObjectDesc &d)
				{
					objs_.emplace_back(gArg_, d);
					return &objs_.back();
				}
				inline void destruct_(Object * obj)
				{
					for (auto it = objs_.begin(); it != objs_.end(); it++)
						if (&(*it) == obj)
							break;
					if (it == objs_.end())
						throw ManagerException("ManagerUnique_Collection_::destruct_: object not found!");
					objs_.erase(it);
				}
			};
			template<class Object, class ObjectDesc>
			class ManagerUnique_Collection_<Object, ObjectDesc, void>
			{
				std::list<Object> objs_; //objects
			protected:
				inline Object * construct_(const ObjectDesc &d)
				{
					objs_.emplace_back(d);
					return &objs_.back();
				}
				inline void destruct_(Object * obj)
				{
					for (auto it = objs_.begin(); it != objs_.end(); it++)
						if (&(*it) == obj)
							break;
					if (it == objs_.end())
						throw ManagerException("ManagerUnique_Collection_::destruct_: object not found!");
					objs_.erase(it);
				}
			};

			template<class Object, class ObjectDesc>
			class ManagerUnique_Manager_
			{
				struct Desc_;
			public:
				typedef uint32_t ObjectID;
				typedef uint32_t RefCounter;
				static const ObjectID no_id = std::numeric_limits<RefCounter>::max();

				typedef std::vector<Desc_> ManInfoColl;
			private:
				struct Desc_
				{
					ObjectID objID = no_id;
					ObjectDesc objDesc;
					RefCounter refs = 0;
					ObjectStatus status = ObjectStatus::UNDEF; //may be either UNDEF or READY
					Object * object = nullptr;
					static bool cmp_(const Desc_ &d1, const Desc_ &d2)
					{
						return d1.objDesc < d2.objDesc;
					}
					static bool cmp_(const Desc_ &d1, const ObjectDesc &d2)
					{
						return d1.objDesc < d2;
					}
				};
				
				ManInfoColl descs_; //descriptions -- objID is index

				typename ManInfoColl::const_iterator find_(const ObjectDesc &d) const
				{
					ManInfoColl::const_iterator iter = std::lower_bound(descs_.begin(), descs_.end(), d, Desc_::cmp_);
					if (iter == descs_.end())
						return iter;
					if (d == iter->objDesc)
						return iter;
					return descs_.end();
				}			
				
			protected:
				inline ObjectID findID_(const ObjectDesc &d) const
				{
					ManInfoColl::const_iterator iter = find_(d);
					if (iter == descs_.end())
						return no_id;
					return iter->objID;
				}
				inline ObjectID add_desc_(const ObjectDesc &d)
				{
					auto it = find_(d);
					if (it == descs_.end())
					{
						Desc_ nd;
						nd.objID = no_id;
						nd.object = nullptr;
						nd.refs = 0;
						nd.status = ObjectStatus::UNDEF;
						descs_.push_back(nd);
						return descs_.size() - 1;
					}
					return std::distance(descs_.begin(), it);
				}
				const ObjectDesc &get_desc_(ObjectID id) const
				{
					return descs_.at(id);
				}
				inline ObjectID start_create_(ObjectID id)
				{
					Desc_ &nd = descs_.at(id);
					nd.status = ObjectStatus::UNDEF;
					nd.refs = 0;
					nd.objID = id;
					nd.object = nullptr;
					return nd.objID;
				}
				void finish_create_(Object id, Object * ptr)
				{
					descs_.[id] = ptr;
					descs_.[id].status = ObjectStatus::READY;
				}
				inline void destroy_(ObjectID id)
				{
					descs_[id].object = nullptr;
					descs_[id].status = ObjectStatus::UNDEF;
					descs_[id].objID = no_id;
					descs_[id].refs = 0;
				}
				void release_(ObjectID id)
				{
					if (descs_[id].refs == 0)
						throw ManagerException("ManagerUnique::release_: no references on object you are trying to release!");
					descs_[id].refs--;
					/*insert destruction code!*/
				}
				ObjectID request_(ObjectID id)
				{
					descs_[id].refs++;
					return id;
				}
				inline Object * get_(ObjectID id)
				{
					Desc_ &r = descs_.at(id);
					if (r.status != ObjectStatus::READY)
						throw ManagerException("ManagerUnique::get_: requested object is not ready!");
					return r.object;
				}
				inline const Object * get_(ObjectID id) const
				{
					Desc_ &r = descs_.at(id);
					if (r.status != ObjectStatus::READY)
						throw ManagerException("ManagerUnique::get_: requested object is not ready!");
					return r.object;
				}
				inline ObjectStatus status_(ObjectID id) const
				{
					if (id >= descs_.size())
						throw ManagerException("ManagerUnique::status_: id out of range!");
					return descs_[id].status;
				}
			};

			template<class Object, class ObjectDesc, class GlobalArg>
			class ManagerUnique : public ManagerUnique_Collection_<Object, ObjectDesc, GlobalArg>, public ManagerUnique_Manager_<Object, ObjectDesc>
			{
			public:
				ManagerUnique(const GlobalArg &a) : ManagerUnique_Collection_<Object, ObjectDesc, GlobalArg>(a)
				{

				}
				ObjectID describe(const ObjectDesc &d)
				{
					return add_desc_(d);
				}
				ObjectID request(const ObjectDesc &d)
				{
					auto id = add_desc_(d);
					if (status_(id) == ObjectStatus::UNDEF)
					{
						start_create_(id);
						auto ptr = construct_(d);
						finish_create_(id, ptr);
					}
					return request_(id);
				}
				ObjectID request(ObjectID id)
				{
					if (status_(id) == ObjectStatus::UNDEF)
					{
						start_create_(id);
						auto ptr = construct_(d);
						finish_create_(id, ptr);
					}
					return request_(id);
				}
				void release(ObjectID id)
				{
					if (status_(id) == ObjectStatus::READY)
						release_(id);
					else
						throw ManagerException("ManagerUnique::release: trying to release dead object!")
				}
				Object * get(ObjectID id)
				{
					return get_(id);
				}
				const Object * get(ObjectID id) const
				{
					return get_(id);
				}
				ObjectStatus status(ObjectID id) const
				{
					return status_(id);
				}
			};
		}
	}
}