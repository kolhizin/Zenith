#pragma once

#include "manager_base.h"
#include <vector>
#include <deque>
#include <limits>
#include <algorithm>
#include <mutex>

namespace zenith
{
	namespace util
	{
		namespace manager
		{
			template<class DescriptorType, class ObjUID>
			class ManagerDescriptor
			{
			public:
				typedef uint32_t Handle;
				static const Handle NoHandle = 0xFFFFFFFF;
			private:
				std::deque<DescriptorType> objs_;
				std::map<ObjUID, Handle> idMap_;

				Handle find_() const
				{
					const DescriptorType &lst = objs_.back();
					for (Handle i = 0; i < objs_.size() - 1; i++)
						if (lst == objs_[i])
							return i;
					return NoHandle;
				}
			public:
				inline bool hasUID(const ObjUID &uid) const
				{
					return idMap_.find(uid) == idMap_.end();
				}
				inline bool validHandle(Handle h) const { return h < objs_.size(); }
				inline Handle getHandle(const ObjUID &uid) const
				{
					auto it = idMap_.find(uid);
					if (it == idMap_.end())
						return NoHandle;
					return it->second;
				}
				template<typename... Args>
				inline Handle getHandle(Args&&... args) const
				{
					DescriptorType inst(std::forward<Args>(args)...);
					for (Handle h = 0; h < objs_.size(); h++)
						if (objs_[h] == inst)
							return h;
					return NoHandle;
				}

				inline const DescriptorType &get(Handle h) const
				{
					if (h < objs_.size())
						return objs_[h];
					throw ManagerException("ManagerDescriptor::get: invalid handle.");
				}
				inline const DescriptorType &get(const ObjUID &uid) const
				{
					Handle h = getHandle(uid);
					if (h < objs_.size())
						return objs_[h];
					throw ManagerException("ManagerDescriptor::get: invalid uid.");
				}
				
				template<typename... Args>
				inline Handle create(Args&&... args)
				{
					if(getHandle(std::forward<Args>(args)...) != NoHandle)
						throw ManagerException("ManagerDescriptor::create: descriptor already exists.");
					objs_.emplace_back(std::forward<Args>(args)...);
					return objs_.size() - 1;
				}
				template<typename... Args>
				inline Handle createNamed(const ObjUID &uid, Args&&... args)
				{
					if (getHandle(std::forward<Args>(args)...) != NoHandle)
						throw ManagerException("ManagerDescriptor::createNamed: descriptor already exists.");
					if(idMap_.find(uid) != idMap_.end())
						throw ManagerException("ManagerDescriptor::createNamed: name already exists.");
					objs_.emplace_back(std::forward<Args>(args)...);
					Handle h = objs_.size() - 1;
					idMap_[uid] = h;
					return h;
				}
				template<typename... Args>
				inline Handle request(Args&&... args)
				{
					auto h = getHandle(std::forward<Args>(args)...);
					if (h != NoHandle)
						return h;
					objs_.emplace_back(std::forward<Args>(args)...);
					return objs_.size() - 1;
				}
			};
		}
	}
}