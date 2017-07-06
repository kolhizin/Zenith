
#pragma once
#include <Utils\ioconv\io_config.h>
#include <Utils\macro_version.h>
#include <Utils\nameid.h>
#include "ggraphics_base.h"

#pragma warning(push)
#pragma warning(disable:4101)

namespace zenith
{
	namespace gengraphics
	{
		struct ggPipelineResourceLocation
		{
			uint32_t group;
			uint32_t index;
		};
		template<class It> class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineResourceLocation, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineResourceLocation value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineResourceLocation &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.group, it, "group");
				zenith::util::ioconv::input_named_required(val.index, it, "index");
			}
			inline static void output(const zenith::gengraphics::ggPipelineResourceLocation &val, It &it)
			{
				zenith::util::ioconv::output_single(val.group, it.append_value("group", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.index, it.append_value("index", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct ggPipelineResourceDescriptor
		{
			ggPipelineResourceLocation location;
			ggPipelineResourceType type;
			zenith::util::bitenum<ggPipelineStage> stages;
		};
		template<class It> class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineResourceDescriptor, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineResourceDescriptor value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineResourceDescriptor &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.location, it, "location");
				zenith::util::ioconv::input_named_required(val.type, it, "type");
				zenith::util::ioconv::input_named_required(val.stages, it, "stages");
			}
			inline static void output(const zenith::gengraphics::ggPipelineResourceDescriptor &val, It &it)
			{
				zenith::util::ioconv::output_single(val.location, it.append_complex("location"));
				zenith::util::ioconv::output_single(val.type, it.append_value("type", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.stages, it.append_value("stages", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};

	}
}


#pragma warning(pop)