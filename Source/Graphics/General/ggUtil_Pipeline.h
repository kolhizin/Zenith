
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
		struct ggPipelineResourceBinding
		{
			uint32_t group;
			uint32_t location;
		};
		template<class It> class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineResourceBinding, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineResourceBinding value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineResourceBinding &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.group, it, "group");
				zenith::util::ioconv::input_named_required(val.location, it, "location");
			}
			inline static void output(const zenith::gengraphics::ggPipelineResourceBinding &val, It &it)
			{
				zenith::util::ioconv::output_single(val.group, it.append_value("group", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.location, it.append_value("location", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct ggPipelineResourceDescriptor
		{
			ggPipelineResourceBinding binding;
			ggPipelineResourceType type;
			bool dynamic;
			zenith::util::bitenum<ggPipelineStage> stages;
		};
		template<class It> class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineResourceDescriptor, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineResourceDescriptor value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineResourceDescriptor &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.binding, it, "binding");
				zenith::util::ioconv::input_named_required(val.type, it, "type");
				val.dynamic = false;
				zenith::util::ioconv::input_named_optional(val.dynamic, it, "dynamic");
				zenith::util::ioconv::input_named_required(val.stages, it, "stages");
			}
			inline static void output(const zenith::gengraphics::ggPipelineResourceDescriptor &val, It &it)
			{
				zenith::util::ioconv::output_single(val.binding, it.append_complex("binding"));
				zenith::util::ioconv::output_single(val.type, it.append_value("type", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.dynamic, it.append_value("dynamic", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.stages, it.append_value("stages", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct ggPipelineDepthProgramDescriptor
		{
			bool fTest;
			bool fWrite;
			ggCompareOp compareOp;
		};
		template<class It> class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineDepthProgramDescriptor, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineDepthProgramDescriptor value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineDepthProgramDescriptor &val, const It &it)
			{
				val.fTest = false;
				zenith::util::ioconv::input_named_optional(val.fTest, it, "fTest");
				val.fWrite = false;
				zenith::util::ioconv::input_named_optional(val.fWrite, it, "fWrite");
				val.compareOp = zenith::gengraphics::ggCompareOp::LESS;
				zenith::util::ioconv::input_named_optional(val.compareOp, it, "compareOp");
			}
			inline static void output(const zenith::gengraphics::ggPipelineDepthProgramDescriptor &val, It &it)
			{
				zenith::util::ioconv::output_single(val.fTest, it.append_value("fTest", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.fWrite, it.append_value("fWrite", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.compareOp, it.append_value("compareOp", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct ggPipelineStencilProgramDescriptor
		{
			bool fTest;
		};
		template<class It> class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineStencilProgramDescriptor, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineStencilProgramDescriptor value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineStencilProgramDescriptor &val, const It &it)
			{
				val.fTest = false;
				zenith::util::ioconv::input_named_optional(val.fTest, it, "fTest");
			}
			inline static void output(const zenith::gengraphics::ggPipelineStencilProgramDescriptor &val, It &it)
			{
				zenith::util::ioconv::output_single(val.fTest, it.append_value("fTest", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct ggPipelineRasterizeProgramDescriptor
		{
			bool fDiscard;
			ggCullMode cullMode;
			ggFaceDef faceDef;
		};
		template<class It> class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineRasterizeProgramDescriptor, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineRasterizeProgramDescriptor value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineRasterizeProgramDescriptor &val, const It &it)
			{
				val.fDiscard = false;
				zenith::util::ioconv::input_named_optional(val.fDiscard, it, "fDiscard");
				val.cullMode = zenith::gengraphics::ggCullMode::NONE;
				zenith::util::ioconv::input_named_optional(val.cullMode, it, "cullMode");
				val.faceDef = zenith::gengraphics::ggFaceDef::CLOCKWISE;
				zenith::util::ioconv::input_named_optional(val.faceDef, it, "faceDef");
			}
			inline static void output(const zenith::gengraphics::ggPipelineRasterizeProgramDescriptor &val, It &it)
			{
				zenith::util::ioconv::output_single(val.fDiscard, it.append_value("fDiscard", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.cullMode, it.append_value("cullMode", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.faceDef, it.append_value("faceDef", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct ggPipelineMultisampleProgramDescriptor
		{
			uint32_t numSamples;
		};
		template<class It> class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineMultisampleProgramDescriptor, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineMultisampleProgramDescriptor value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineMultisampleProgramDescriptor &val, const It &it)
			{
				val.numSamples = 1;
				zenith::util::ioconv::input_named_optional(val.numSamples, it, "numSamples");
			}
			inline static void output(const zenith::gengraphics::ggPipelineMultisampleProgramDescriptor &val, It &it)
			{
				zenith::util::ioconv::output_single(val.numSamples, it.append_value("numSamples", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};

	}
}


#pragma warning(pop)
