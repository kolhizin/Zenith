
#pragma once
#include <Utils\object_map.h>
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
		inline void to_objmap(const TerraGenParam_Box2D &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "TerraGenParam_Box2D", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.x0, buff, 128);
			om.addValue("x0", buff, zenith::util::ObjectMapValueHint::ATTR);
			zenith::util::str_cast(obj.x1, buff, 128);
			om.addValue("x1", buff, zenith::util::ObjectMapValueHint::ATTR);
			zenith::util::str_cast(obj.y0, buff, 128);
			om.addValue("y0", buff, zenith::util::ObjectMapValueHint::ATTR);
			zenith::util::str_cast(obj.y1, buff, 128);
			om.addValue("y1", buff, zenith::util::ObjectMapValueHint::ATTR);
		}
		inline void from_objmap(TerraGenParam_Box2D &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.x0, "x0");
			OBJMAP_GET_ONE_VALUE(om, obj.x1, "x1");
			OBJMAP_GET_ONE_VALUE(om, obj.y0, "y0");
			OBJMAP_GET_ONE_VALUE(om, obj.y1, "y1");
		}


		struct TerraGenParam_Box1D
		{
			double x0;
			double x1;
		};
		inline void to_objmap(const TerraGenParam_Box1D &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "TerraGenParam_Box1D", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.x0, buff, 128);
			om.addValue("x0", buff, zenith::util::ObjectMapValueHint::ATTR);
			zenith::util::str_cast(obj.x1, buff, 128);
			om.addValue("x1", buff, zenith::util::ObjectMapValueHint::ATTR);
		}
		inline void from_objmap(TerraGenParam_Box1D &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.x0, "x0");
			OBJMAP_GET_ONE_VALUE(om, obj.x1, "x1");
		}


		struct TerraGenParam_MuSigma
		{
			double mu;
			double sigma;
		};
		inline void to_objmap(const TerraGenParam_MuSigma &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "TerraGenParam_MuSigma", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.mu, buff, 128);
			om.addValue("mu", buff, zenith::util::ObjectMapValueHint::ATTR);
			zenith::util::str_cast(obj.sigma, buff, 128);
			om.addValue("sigma", buff, zenith::util::ObjectMapValueHint::ATTR);
		}
		inline void from_objmap(TerraGenParam_MuSigma &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.mu, "mu", 0.0);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.sigma, "sigma", 1.0);
		}


		struct TerraGenParam_CatDistrDouble
		{
			double value;
			double prob;
		};
		inline void to_objmap(const TerraGenParam_CatDistrDouble &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "TerraGenParam_CatDistrDouble", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.value, buff, 128);
			om.addValue("value", buff, zenith::util::ObjectMapValueHint::ATTR);
			zenith::util::str_cast(obj.prob, buff, 128);
			om.addValue("prob", buff, zenith::util::ObjectMapValueHint::ATTR);
		}
		inline void from_objmap(TerraGenParam_CatDistrDouble &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.value, "value");
			OBJMAP_GET_ONE_VALUE(om, obj.prob, "prob");
		}


		struct TerraGenParam_CatDistrInt
		{
			int64_t value;
			double prob;
		};
		inline void to_objmap(const TerraGenParam_CatDistrInt &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "TerraGenParam_CatDistrInt", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.value, buff, 128);
			om.addValue("value", buff, zenith::util::ObjectMapValueHint::ATTR);
			zenith::util::str_cast(obj.prob, buff, 128);
			om.addValue("prob", buff, zenith::util::ObjectMapValueHint::ATTR);
		}
		inline void from_objmap(TerraGenParam_CatDistrInt &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.value, "value");
			OBJMAP_GET_ONE_VALUE(om, obj.prob, "prob");
		}


		struct TerraGenParam_CatDistrString
		{
			std::string value;
			double prob;
		};
		inline void to_objmap(const TerraGenParam_CatDistrString &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "TerraGenParam_CatDistrString", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("value", obj.value.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			zenith::util::str_cast(obj.prob, buff, 128);
			om.addValue("prob", buff, zenith::util::ObjectMapValueHint::ATTR);
		}
		inline void from_objmap(TerraGenParam_CatDistrString &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.value, "value");
			OBJMAP_GET_ONE_VALUE(om, obj.prob, "prob");
		}

	}
}


#pragma warning(pop)
