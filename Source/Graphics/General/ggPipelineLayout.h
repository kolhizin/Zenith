#pragma once
#include "ggraphics_base.h"
#include "ggPipelineBase.h"
#include "ggUtil_Pipeline.h"
#include <Utils\hdict.h>
#include <vector>

namespace zenith
{
	namespace gengraphics
	{
		class GenGraphicsPipelineLayoutException : public GenGraphicsPipelineException
		{
		public:
			GenGraphicsPipelineLayoutException() : GenGraphicsPipelineException() {}
			GenGraphicsPipelineLayoutException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : GenGraphicsPipelineException(p, type)
			{
			}
			GenGraphicsPipelineLayoutException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : GenGraphicsPipelineException(str, type)
			{
			}
			virtual ~GenGraphicsPipelineLayoutException() {}
		};

		class ggPipelineResource
		{
			static const uint32_t NoGroup = 0xFFFFFFFF;
			static const uint32_t NoLocation = 0xFFFFFFFF;

			ggPipelineResourceDescriptor descr_;
		public:
			inline ggPipelineResource(const ggPipelineResourceDescriptor &d) : descr_(d)
			{
				if (descr_.type == ggPipelineResourceType::UNDEF)
					throw GenGraphicsException("ggPipelineResource: undefined resources type passed into constructor.");
				if (descr_.binding.group == NoGroup || descr_.binding.location == NoLocation)
					throw GenGraphicsException("ggPipelineResource: undefined location passed into constructor.");
			}
			inline ggPipelineResource(const ggPipelineResource &d) : descr_(d.descr_) {}

			const ggPipelineResourceDescriptor &descriptor() const { return descr_; }
			const ggPipelineResourceBinding &binding() const { return descr_.binding; }
			uint32_t binding_location() const { return descr_.binding.location; }
			uint32_t binding_group() const { return descr_.binding.group; }

			zenith::util::bitenum<ggPipelineStage> stages() const { return descr_.stages; }
			bool used_at(ggPipelineStage stg) const { return descr_.stages.includes(stg); }

			ggPipelineResourceType type() const { return descr_.type; }
			bool is_dynamic() const { return descr_.dynamic; }
		};

		class ggPipelineLayoutGroup
		{
			zenith::util::hdict<uint32_t> names_; //map names to offsets
			std::vector<ggPipelineResource> resources_; //same resources, one group, sorted by index
			inline void assert_nonempty_() const
			{
				if(resources_.empty())
					GenGraphicsPipelineLayoutException("ggPipelineLayoutGroup: no resources in group.");
			}
			template<class T>
			inline void add_(const char * name, const T &v, uint32_t loc)
			{
				uint32_t newPos = static_cast<uint32_t>(resources_.size());
				for(uint32_t i = 0; i < resources_.size(); i++)
					if (resources_[i].binding_location() > loc)
					{
						newPos = i;
						break;
					}
				resources_.insert(resources_.begin() + newPos, v);
				for (auto it = names_.begin(); it != names_.end(); ++it)
					if (it.value() >= newPos)
						it.value()++;
				names_.add(name, newPos);
			}
		public:
			inline ggPipelineLayoutGroup()
			{
			}
			inline ggPipelineLayoutGroup(const zenith::util::hdict<ggPipelineResourceDescriptor> &rD)
			{
				resources_.reserve(rD.size());
				for (auto it = rD.begin(); it != rD.end(); ++it)
					add(it.key(), it.value());
			}

			inline void add(const char * name, const ggPipelineResourceDescriptor &rd)
			{
				if (!resources_.empty())
					if (rd.binding.group != resources_.front().binding_group())
						throw GenGraphicsPipelineLayoutException("ggPipelineLayoutGroup::add(): new resources group is not the same as current group id.");
				if(names_.exists(name))
					throw GenGraphicsPipelineLayoutException("ggPipelineLayoutGroup::add(): resource with this name already exists.");
				add_(name, rd, rd.binding.location);
			}
			inline void add(const char * name, const ggPipelineResource &r)
			{
				if (!resources_.empty())
					if (r.binding_group() != resources_.front().binding_group())
						throw GenGraphicsPipelineLayoutException("ggPipelineLayoutGroup::add(): new resources group is not the same as current group id.");
				if (names_.exists(name))
					throw GenGraphicsPipelineLayoutException("ggPipelineLayoutGroup::add(): resource with this name already exists.");
				add_(name, r, r.binding_location());
			}
			inline bool empty() const { return resources_.empty(); }
			inline uint32_t binding_group() const { assert_nonempty_(); return resources_.front().binding_group(); }
			inline uint32_t size() const { return static_cast<uint32_t>(resources_.size()); }

			inline bool exists(const char * name) const { return names_.exists(name); }
			inline const ggPipelineResource &at_name(const char * name) const { return resources_.at(names_.at(name)); }
			inline const ggPipelineResource &at(uint32_t idx) const { return resources_.at(idx); }

			inline zenith::util::hdict<uint32_t>::const_iterator names_begin() const { return names_.begin(); }
			inline zenith::util::hdict<uint32_t>::const_iterator names_end() const { return names_.end(); }

			inline std::vector<ggPipelineResource>::const_iterator begin() const { return resources_.begin(); }
			inline std::vector<ggPipelineResource>::const_iterator end() const { return resources_.end(); }
		};

		class ggPipelineLayout
		{
			zenith::util::hdict<uint32_t> names_; //map names to groups
			std::vector<ggPipelineLayoutGroup> groups_;

			template<class O, class T>
			class iterator_t_
			{
				friend class ggPipelineLayout;
				uint32_t group_idx_, location_idx_;
				O * obj_;
				iterator_t_(O * obj, uint32_t gidx, uint32_t lidx) : obj_(obj), group_idx_(gidx), location_idx_(lidx)
				{

				}
				inline bool valid_() const
				{
					return obj_ && (group_idx_ < obj_->num_groups()) && (location_idx_ < obj_->at(group_idx_).size());
				}
				inline std::pair<uint32_t, uint32_t> next_() const
				{
					uint32_t ngrp = group_idx_;
					uint32_t nloc = location_idx_ + 1;
					if (nloc >= obj_->at(group_idx_).size())
					{
						nloc = 0;
						ngrp++;
					}
					if (ngrp >= obj_->num_groups() || nloc >= obj_->at(group_idx_).size())
						return std::pair<uint32_t, uint32_t>(0xFFFFFFFF, 0xFFFFFFFF);
					return std::make_pair(ngrp, nloc);
				}
			public:
				inline iterator_t_() : obj_(nullptr), group_idx_(0xFFFFFFFF), location_idx_(0xFFFFFFFF) {}
				inline iterator_t_(const iterator_t_ &it) : obj_(it.obj_), group_idx_(it.group_idx_), location_idx_(it.location_idx_) {}
				inline iterator_t_ &operator =(const iterator_t_ &it)
				{
					obj_ = it.obj_;
					group_idx_ = it.group_idx_;
					location_idx_ = it.location_idx_;
					return *this;
				}
				template<class O2, class T2>
				inline bool operator ==(const iterator_t_<O2, T2> &it) const
				{
					if ((!valid_()) || (!it.valid_()))
					{
						if ((!valid_()) && (!it.valid_()))
							return true;
						return false;
					}
					return (obj_ == it.obj_) && (group_idx_ == it.group_idx_) && (location_idx_ == it.location_idx_);
				}
				template<class O2, class T2>
				inline bool operator !=(const iterator_t_<O2, T2> &it) const
				{
					if ((!valid_()) || (!it.valid_()))
					{
						if ((!valid_()) && (!it.valid_()))
							return false;
						return true;
					}
					return (obj_ != it.obj_) || (group_idx_ != it.group_idx_) || (location_idx_ != it.location_idx_);
				}

				inline iterator_t_ &operator++()
				{
					if (!valid_())
					{
						obj_ = nullptr;
						group_idx_ = 0xFFFFFFFF;
						location_idx_ = 0xFFFFFFFF;
						return *this;
					}
					auto n = next_();
					if (n.first >= obj_->num_groups() || n.second >= obj_->at(n.first).size())
					{
						obj_ = nullptr;
						group_idx_ = 0xFFFFFFFF;
						location_idx_ = 0xFFFFFFFF;
						return *this;
					}
					group_idx_ = n.first;
					location_idx_ = n.second;
					return *this;
				}
				inline iterator_t_ operator++(int)
				{
					iterator_t_ tmp(*this);
					if (!valid_())
					{
						obj_ = nullptr;
						group_idx_ = 0xFFFFFFFF;
						location_idx_ = 0xFFFFFFFF;
						return tmp;
					}
					auto n = next_();
					if (n.first >= obj_->num_groups() || n.second >= obj_->at(n.first).size())
					{
						obj_ = nullptr;
						group_idx_ = 0xFFFFFFFF;
						location_idx_ = 0xFFFFFFFF;
						return tmp;
					}
					group_idx_ = n.first;
					location_idx_ = n.second;
					return tmp;
				}

				inline const T &operator *() const
				{
					if (!valid_())
						throw GenGraphicsPipelineLayoutException("ggPipelineLayout::iterator: dereferencing invalid iterator.");
					return obj_->at(group_idx_, location_idx_);
				}
				inline const T * operator ->() const
				{
					if (!valid_())
						throw GenGraphicsPipelineLayoutException("ggPipelineLayout::iterator: dereferencing invalid iterator.");
					return &obj_->at(group_idx_, location_idx_);
				}
			};
		public:
			typedef iterator_t_<const ggPipelineLayout, const ggPipelineResource> iterator;

			ggPipelineLayout()
			{
			}
			ggPipelineLayout(const zenith::util::hdict<ggPipelineResourceDescriptor> &rD)
			{
				groups_.reserve(3);
				for (auto it = rD.begin(); it != rD.end(); ++it)
					add(it.key(), it.value());
			}
			inline void add(const ggPipelineLayoutGroup &rd)
			{
				uint32_t id = static_cast<uint32_t>(groups_.size());
				for(uint32_t i = 0; i < groups_.size(); i++)
					if(groups_[i].binding_group() == rd.binding_group())
						throw GenGraphicsPipelineLayoutException("ggPipelineLayout::add(): resources group with this group id already exists.");
					else if (groups_[i].binding_group() > rd.binding_group())
					{
						id = i;
						break;
					}
				for(auto it = rd.names_begin(); it != rd.names_end(); ++it)
					if(names_.exists(it.key()))
						throw GenGraphicsPipelineLayoutException("ggPipelineLayout::add(): resource with this name already exists.");
				for (auto it = names_.begin(); it != names_.end(); ++it)
					if (it.value() >= id)
						it.value()++;
				groups_.insert(groups_.begin() + id, rd);
				for (auto nit = rd.names_begin(); nit != rd.names_end(); ++nit)
					names_.add(nit.key(), id);
			}
			inline void add(const char * name, const ggPipelineResourceDescriptor &rd)
			{
				uint32_t id = static_cast<uint32_t>(groups_.size());
				for (uint32_t i = 0; i < groups_.size(); i++)
					if (groups_[i].binding_group() >= rd.binding.group)
					{
						id = i;
						break;
					}
				if(names_.exists(name))
					throw GenGraphicsPipelineLayoutException("ggPipelineLayout::add(): resource with this name already exists.");
				if (id < groups_.size() && groups_[id].binding_group() == rd.binding.group)
				{
					groups_[id].add(name, rd);
					names_.add(name, id);
				}
				else
				{
					for (auto it = names_.begin(); it != names_.end(); ++it)
						if (it.value() >= id)
							it.value()++;

					groups_.insert(groups_.begin() + id, ggPipelineLayoutGroup());
					groups_[id].add(name, rd);
					names_.add(name, id);
				}
			}
			inline void add(const char * name, const ggPipelineResource &r)
			{
				uint32_t id = static_cast<uint32_t>(groups_.size());
				for (uint32_t i = 0; i < groups_.size(); i++)
					if (groups_[i].binding_group() >= r.binding_group())
					{
						id = i;
						break;
					}
				if (names_.exists(name))
					throw GenGraphicsPipelineLayoutException("ggPipelineLayout::add(): resource with this name already exists.");
				if (id < groups_.size() && groups_[id].binding_group() == r.binding_group())
				{
					groups_[id].add(name, r);
					names_.add(name, id);
				}
				else
				{
					for (auto it = names_.begin(); it != names_.end(); ++it)
						if (it.value() >= id)
							it.value()++;

					groups_.insert(groups_.begin() + id, ggPipelineLayoutGroup());
					groups_[id].add(name, r);
					names_.add(name, id);
				}
			}
			inline bool empty() const { return groups_.empty(); }
			inline uint32_t size() const { return static_cast<uint32_t>(names_.size()); }
			inline uint32_t num_groups() const { return static_cast<uint32_t>(groups_.size()); }

			inline bool exists(const char * name) const { return names_.exists(name); }
			inline const ggPipelineResource &at_name(const char * name) const { return groups_.at(names_.at(name)).at_name(name); }
			inline const ggPipelineResource &at(uint32_t group_idx, uint32_t location_idx) const { return groups_.at(group_idx).at(location_idx); }
			inline const ggPipelineLayoutGroup &at(uint32_t group_idx) const { return groups_.at(group_idx); }

			inline zenith::util::hdict<uint32_t>::const_iterator names_begin() const { return names_.begin(); }
			inline zenith::util::hdict<uint32_t>::const_iterator names_end() const { return names_.end(); }

			inline std::vector<ggPipelineResource>::const_iterator vbegin(uint32_t group_idx) const { return groups_.at(group_idx).begin(); }
			inline std::vector<ggPipelineResource>::const_iterator vend(uint32_t group_idx) const { return groups_.at(group_idx).end(); }

			inline iterator begin(uint32_t group_idx) const { return iterator(this, group_idx, 0); }
			inline iterator end(uint32_t group_idx) const { return iterator(this, group_idx + 1, 0); }

			inline iterator begin() const { return iterator(this, 0, 0); }
			inline iterator end() const { return iterator(); }
		};

		template<class It>
		class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineLayout, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineLayout value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineLayout &val, const It &it)
			{
				zenith::util::hdict<zenith::gengraphics::ggPipelineResourceDescriptor> hRD;
				zenith::util::ioconv::input_named_multiple_charmap(hRD, it, "resource", "name");
				
				zenith::gengraphics::ggPipelineLayout tmpLayout(hRD);
				val = std::move(tmpLayout);
			}
		};
	}
}