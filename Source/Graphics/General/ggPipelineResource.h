#pragma once
#include "ggraphics_base.h"
#include "ggUtil_Pipeline.h"
#include <Utils/prefix_dag.h>

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

		
		class ggPipelineResource
		{
			static const uint32_t NoGroup = 0xFFFFFFFF;
			static const uint32_t NoIndex = 0xFFFFFFFF;

			ggPipelineResourceDescriptor descr_;
		public:
			inline ggPipelineResource(const ggPipelineResourceDescriptor &d) : descr_(d) {}
			inline ggPipelineResource(const ggPipelineResource &d) : descr_(d.descr_) {}

			const ggPipelineResourceDescriptor &descriptor() const { return descr_; }
			uint32_t location_index() const { return descr_.location.index; }
			uint32_t location_group() const { return descr_.location.group; }

			zenith::util::bitenum<ggPipelineStage> stages() const { return descr_.stages; }
			bool used_at(ggPipelineStage stg) const { return descr_.stages.includes(stg); }

			ggPipelineResourceType type() const { return descr_.type; }
		};
		
		class ggPipelineResources
		{
			std::vector<ggPipelineResource> resources_;
		};
	}
}