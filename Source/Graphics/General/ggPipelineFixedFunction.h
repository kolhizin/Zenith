#pragma once

#include "ggraphics_base.h"
#include "ggUtil_Pipeline.h"

namespace zenith
{
	namespace gengraphics
	{
		class ggPipelineProgramRasterize
		{
			ggPipelineRasterizeProgramDescriptor descr_;
		public:
			inline ggPipelineProgramRasterize()
			{
				descr_.fDiscard = false;
				descr_.faceDef = ggFaceDef::UNDEF;
				descr_.cullMode = ggCullMode::NONE;
			}
			inline ggPipelineProgramRasterize(const ggPipelineRasterizeProgramDescriptor &d) : descr_(d)
			{
				if(descr_.cullMode != ggCullMode::NONE && descr_.faceDef == ggFaceDef::UNDEF)
					throw GenGraphicsException("ggPipelineProgramRasterize: face not defined, but cull mode is present.");
			}
			inline ggPipelineProgramRasterize(const ggPipelineProgramRasterize &d) : descr_(d.descr_) {}

			const ggPipelineRasterizeProgramDescriptor &descriptor() const { return descr_; }

			inline bool is_discardable() const { return descr_.fDiscard; }
			inline ggCullMode cull_mode() const { return descr_.cullMode; }
			inline ggFaceDef face_def() const { return descr_.faceDef; }
		};

		class ggPipelineProgramMultisample
		{
			ggPipelineMultisampleProgramDescriptor descr_;
		public:
			inline ggPipelineProgramMultisample()
			{
				descr_.numSamples = 1;
			}
			inline ggPipelineProgramMultisample(const ggPipelineMultisampleProgramDescriptor &d) : descr_(d)
			{
			}
			inline ggPipelineProgramMultisample(const ggPipelineProgramMultisample &d) : descr_(d.descr_) {}

			const ggPipelineMultisampleProgramDescriptor &descriptor() const { return descr_; }

			inline uint32_t num_samples() const { return descr_.numSamples; }
		};

		class ggPipelineProgramStencil
		{
			ggPipelineStencilProgramDescriptor descr_;
		public:
			inline ggPipelineProgramStencil()
			{
				descr_.fTest = false;
			}
			inline ggPipelineProgramStencil(const ggPipelineStencilProgramDescriptor &d) : descr_(d)
			{
			}
			inline ggPipelineProgramStencil(const ggPipelineProgramStencil &d) : descr_(d.descr_) {}

			const ggPipelineStencilProgramDescriptor &descriptor() const { return descr_; }

			inline bool is_enabled() const { return descr_.fTest; }
		};

		class ggPipelineProgramDepth
		{
			ggPipelineDepthProgramDescriptor descr_;
		public:
			inline ggPipelineProgramDepth()
			{
				descr_.fTest = false;
				descr_.fWrite = false;
				descr_.compareOp = ggCompareOp::UNDEF;
			}
			inline ggPipelineProgramDepth(const ggPipelineDepthProgramDescriptor &d) : descr_(d)
			{
				if (descr_.fTest && descr_.compareOp == ggCompareOp::UNDEF)
					throw GenGraphicsException("ggPipelineProgramDepth: compare op is undefined, while test is enabled.");
			}
			inline ggPipelineProgramDepth(const ggPipelineProgramDepth &d) : descr_(d.descr_) {}

			const ggPipelineDepthProgramDescriptor &descriptor() const { return descr_; }

			inline bool is_testable() const { return descr_.fTest; }
			inline bool is_writable() const { return descr_.fWrite; }
			inline ggCompareOp compare_op() const { return descr_.compareOp; }
		};

		class ggPipelineProgramFixed
		{
			ggPipelineProgramDepth depth_;
			ggPipelineProgramStencil stencil_;
			ggPipelineProgramMultisample multisample_;
			ggPipelineProgramRasterize rasterize_;
		public:
			inline const ggPipelineProgramDepth &depth() const { return depth_; }
			inline const ggPipelineProgramStencil &stencil() const { return stencil_; }
			inline const ggPipelineProgramMultisample &multisample() const { return multisample_; }
			inline const ggPipelineProgramRasterize &rasterize() const { return rasterize_; }

			inline void set_depth(const ggPipelineProgramDepth &d) { depth_ = d; }
			inline void set_stencil(const ggPipelineProgramStencil &d) { stencil_ = d; }
			inline void set_multisample(const ggPipelineProgramMultisample &d) { multisample_ = d; }
			inline void set_rasterize(const ggPipelineProgramRasterize &d) { rasterize_ = d; }
		};

		template<class It>
		class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineProgramFixed, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineProgramFixed value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineProgramFixed &val, const It &it)
			{
				zenith::gengraphics::ggPipelineProgramDepth depth;
				zenith::util::ioconv::input_named_optional(depth, it, "depth");
				val.set_depth(depth);

				zenith::gengraphics::ggPipelineProgramStencil stencil;
				zenith::util::ioconv::input_named_optional(stencil, it, "stencil");
				val.set_stencil(stencil);

				zenith::gengraphics::ggPipelineProgramRasterize rasterize;
				zenith::util::ioconv::input_named_optional(rasterize, it, "rasterize");
				val.set_rasterize(rasterize);

				zenith::gengraphics::ggPipelineProgramMultisample multisample;
				zenith::util::ioconv::input_named_optional(multisample, it, "multisample");
				val.set_multisample(multisample);
			}
		};


		template<class It>
		class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineProgramRasterize, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineProgramRasterize value_type;
			typedef zenith::gengraphics::ggPipelineRasterizeProgramDescriptor descr_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineProgramRasterize &val, const It &it)
			{
				typename descr_type descr;
				zenith::util::ioconv::io_handler_impl<typename descr_type, It>::input(descr, it);
				val = zenith::gengraphics::ggPipelineProgramRasterize(descr);
			}
		};
		template<class It>
		class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineProgramMultisample, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineProgramMultisample value_type;
			typedef zenith::gengraphics::ggPipelineMultisampleProgramDescriptor descr_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineProgramMultisample &val, const It &it)
			{
				typename descr_type descr;
				zenith::util::ioconv::io_handler_impl<typename descr_type, It>::input(descr, it);
				val = zenith::gengraphics::ggPipelineProgramMultisample(descr);
			}
		};
		template<class It>
		class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineProgramDepth, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineProgramDepth value_type;
			typedef zenith::gengraphics::ggPipelineDepthProgramDescriptor descr_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineProgramDepth &val, const It &it)
			{
				typename descr_type descr;
				zenith::util::ioconv::io_handler_impl<typename descr_type, It>::input(descr, it);
				val = zenith::gengraphics::ggPipelineProgramDepth(descr);
			}
		};
		template<class It>
		class zenith::util::ioconv::io_handler_impl<zenith::gengraphics::ggPipelineProgramStencil, It>
		{
		public:
			typedef zenith::gengraphics::ggPipelineProgramStencil value_type;
			typedef zenith::gengraphics::ggPipelineStencilProgramDescriptor descr_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::gengraphics::ggPipelineProgramStencil &val, const It &it)
			{
				typename descr_type descr;
				zenith::util::ioconv::io_handler_impl<typename descr_type, It>::input(descr, it);
				val = zenith::gengraphics::ggPipelineProgramStencil(descr);
			}
		};
	}
}