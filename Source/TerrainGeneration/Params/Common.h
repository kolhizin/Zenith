
#pragma once
#include <Utils\ioconv\io_config.h>
#include <Utils\macro_version.h>
#include <Utils\nameid.h>
#pragma warning(push)
#pragma warning(disable:4101)


namespace zenith
{
	namespace terragen
	{
		struct TerraGenParam_Box2D
		{
			double x0;
			double x1;
			double y0;
			double y1;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::terragen::TerraGenParam_Box2D, It, intType>
		{
		public:
			typedef zenith::terragen::TerraGenParam_Box2D value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::terragen::TerraGenParam_Box2D &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.x0, it, "x0");
				zenith::util::ioconv::input_named_required(val.x1, it, "x1");
				zenith::util::ioconv::input_named_required(val.y0, it, "y0");
				zenith::util::ioconv::input_named_required(val.y1, it, "y1");
			}
			inline static void output(const zenith::terragen::TerraGenParam_Box2D &val, It &it)
			{
				zenith::util::ioconv::output_single(val.x0, it.append_value("x0", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.x1, it.append_value("x1", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.y0, it.append_value("y0", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.y1, it.append_value("y1", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct TerraGenParam_Box1D
		{
			double x0;
			double x1;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::terragen::TerraGenParam_Box1D, It, intType>
		{
		public:
			typedef zenith::terragen::TerraGenParam_Box1D value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::terragen::TerraGenParam_Box1D &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.x0, it, "x0");
				zenith::util::ioconv::input_named_required(val.x1, it, "x1");
			}
			inline static void output(const zenith::terragen::TerraGenParam_Box1D &val, It &it)
			{
				zenith::util::ioconv::output_single(val.x0, it.append_value("x0", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.x1, it.append_value("x1", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct TerraGenParam_MuSigma
		{
			double mu;
			double sigma;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::terragen::TerraGenParam_MuSigma, It, intType>
		{
		public:
			typedef zenith::terragen::TerraGenParam_MuSigma value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::terragen::TerraGenParam_MuSigma &val, const It &it)
			{
				val.mu = 0.0;
				zenith::util::ioconv::input_named_optional(val.mu, it, "mu");
				val.sigma = 1.0;
				zenith::util::ioconv::input_named_optional(val.sigma, it, "sigma");
			}
			inline static void output(const zenith::terragen::TerraGenParam_MuSigma &val, It &it)
			{
				zenith::util::ioconv::output_single(val.mu, it.append_value("mu", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.sigma, it.append_value("sigma", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct TerraGenParam_CatDistrDouble
		{
			double value;
			double prob;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::terragen::TerraGenParam_CatDistrDouble, It, intType>
		{
		public:
			typedef zenith::terragen::TerraGenParam_CatDistrDouble value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::terragen::TerraGenParam_CatDistrDouble &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.value, it, "value");
				zenith::util::ioconv::input_named_required(val.prob, it, "prob");
			}
			inline static void output(const zenith::terragen::TerraGenParam_CatDistrDouble &val, It &it)
			{
				zenith::util::ioconv::output_single(val.value, it.append_value("value", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.prob, it.append_value("prob", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct TerraGenParam_CatDistrInt
		{
			int64_t value;
			double prob;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::terragen::TerraGenParam_CatDistrInt, It, intType>
		{
		public:
			typedef zenith::terragen::TerraGenParam_CatDistrInt value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::terragen::TerraGenParam_CatDistrInt &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.value, it, "value");
				zenith::util::ioconv::input_named_required(val.prob, it, "prob");
			}
			inline static void output(const zenith::terragen::TerraGenParam_CatDistrInt &val, It &it)
			{
				zenith::util::ioconv::output_single(val.value, it.append_value("value", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.prob, it.append_value("prob", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};


		struct TerraGenParam_CatDistrString
		{
			std::string value;
			double prob;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::terragen::TerraGenParam_CatDistrString, It, intType>
		{
		public:
			typedef zenith::terragen::TerraGenParam_CatDistrString value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::terragen::TerraGenParam_CatDistrString &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.value, it, "value");
				zenith::util::ioconv::input_named_required(val.prob, it, "prob");
			}
			inline static void output(const zenith::terragen::TerraGenParam_CatDistrString &val, It &it)
			{
				zenith::util::ioconv::output_single(val.value, it.append_value("value", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_single(val.prob, it.append_value("prob", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			}
		};

	}
}


#pragma warning(pop)
