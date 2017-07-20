#pragma once
#include "ggraphics_base.h"
#include "ggUtil_Pipeline.h"
#include <Utils\hdict.h>

namespace zenith
{
	namespace gengraphics
	{
		/*
		class ggPipelineResourceLocation
		{
			uint32_t groupId_, locationId_;
		public:
			static const uint32_t NoGroup = 0xFFFFFFFF;
			static const uint32_t NoLocation = 0xFFFFFFFF;
			ggPipelineResourceLocation(uint32_t groupId = NoGroup, uint32_t locationId = NoLocation)
				: groupId_(groupId), locationId_(locationId) {}

			inline bool isValid() const { return (groupId_ != NoGroup) && (locationId_ != NoLocation); }
			inline uint32_t location() const { return locationId_; }
			inline uint32_t group() const { return groupId_; }
		};*/

		/*
		
		
		
		class ggPipelineResources
		{
			zenith::util::hdict<ggPipelineResource> resources_;
		public:
			inline const ggPipelineResource &get(const char * name) const { return resources_.at(name); }
			inline void add(const char * ptr, const ggPipelineResource &r)
			{
				if (resources_.exists(ptr))
					throw GenGraphicsException("ggPipelineResource: resource with this name already exists.");
				resources_.add(ptr, r);
			}
			inline void add(const char * ptr, const ggPipelineResourceDescriptor &d)
			{
				if (resources_.exists(ptr))
					throw GenGraphicsException("ggPipelineResource: resource with this name already exists.");
				ggPipelineResource res(d);
				resources_.add(ptr, std::move(res));
			}
			inline zenith::util::hdict<ggPipelineResource>::const_iterator begin() const { return resources_.begin(); }
			inline zenith::util::hdict<ggPipelineResource>::const_iterator end() const { return resources_.end(); }
		};

		template<class It>
		class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineResources, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineResources value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineResources &val, const It &it)
			{
				zenith::util::hdict<zenith::gengraphics::ggPipelineResourceDescriptor> descrs;
				zenith::util::ioconv::input_named_multiple_charmap(descrs, it, "resource", "name");
				for (auto it = descrs.begin(); it != descrs.end(); ++it)
					val.add(it.key(), it.value());
			}
		};
		*/
	}
}