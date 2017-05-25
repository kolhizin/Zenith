
#pragma once
#pragma warning(push)
#pragma warning(disable:4101)
#include "Common.h"

namespace zenith
{
	namespace terragen
	{
		struct MountainMetaGenerator1_Params
		{
			zenith::util::nameid uid;
			std::string type;
			std::vector<TerraGenParam_CatDistrString> general;
			std::vector<TerraGenParam_CatDistrString> afterNull;
			std::vector<TerraGenParam_CatDistrString> afterTop;
		};
		inline void to_objmap(const MountainMetaGenerator1_Params &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "MountainMetaGenerator1_Params", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			om.addValue("type", obj.type.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			for (auto x : obj.general)
				to_objmap(x, om.addObject("general"));
			for (auto x : obj.afterNull)
				to_objmap(x, om.addObject("afterNull"));
			for (auto x : obj.afterTop)
				to_objmap(x, om.addObject("afterTop"));
		}
		inline void from_objmap(MountainMetaGenerator1_Params &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.type, "type");
			{
				auto v = om.getObjects("general");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.general.resize(obj.general.size()+1);
					from_objmap(obj.general.back(), it->second);
				}
			}
			{
				auto v = om.getObjects("afterNull");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.afterNull.resize(obj.afterNull.size()+1);
					from_objmap(obj.afterNull.back(), it->second);
				}
			}
			{
				auto v = om.getObjects("afterTop");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.afterTop.resize(obj.afterTop.size()+1);
					from_objmap(obj.afterTop.back(), it->second);
				}
			}
		}


		struct MountainTopGenerator1_Params
		{
			zenith::util::nameid uid;
			std::string type;
			TerraGenParam_Box2D boundingBox;
			double heightLambda;
			double sizeLambda;
			std::vector<TerraGenParam_CatDistrInt> numTops;
			double minNodeDistance;
			double minMountainTopDistance;
			uint32_t numTries;
		};
		inline void to_objmap(const MountainTopGenerator1_Params &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "MountainTopGenerator1_Params", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			om.addValue("type", obj.type.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			to_objmap(obj.boundingBox, om.addObject("boundingBox"));
			zenith::util::str_cast(obj.heightLambda, buff, 128);
			om.addValue("heightLambda", buff);
			zenith::util::str_cast(obj.sizeLambda, buff, 128);
			om.addValue("sizeLambda", buff);
			for (auto x : obj.numTops)
				to_objmap(x, om.addObject("numTops"));
			zenith::util::str_cast(obj.minNodeDistance, buff, 128);
			om.addValue("minNodeDistance", buff);
			zenith::util::str_cast(obj.minMountainTopDistance, buff, 128);
			om.addValue("minMountainTopDistance", buff);
			zenith::util::str_cast(obj.numTries, buff, 128);
			om.addValue("numTries", buff);
		}
		inline void from_objmap(MountainTopGenerator1_Params &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.type, "type");
			from_objmap(obj.boundingBox, om.getObjects("boundingBox", zenith::util::ObjectMapPresence::ONE).first->second);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.heightLambda, "heightLambda", 100.0);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.sizeLambda, "sizeLambda", 100.0);
			{
				auto v = om.getObjects("numTops");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.numTops.resize(obj.numTops.size()+1);
					from_objmap(obj.numTops.back(), it->second);
				}
			}
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.minNodeDistance, "minNodeDistance", 10.0);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.minMountainTopDistance, "minMountainTopDistance", 50.0);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.numTries, "numTries", 1);
		}


		struct MountainTopRidgeGenerator1_Params
		{
			zenith::util::nameid uid;
			std::string type;
			TerraGenParam_MuSigma alpha;
			TerraGenParam_MuSigma beta;
			TerraGenParam_Box1D betaBox;
			TerraGenParam_MuSigma len;
			TerraGenParam_Box1D lenBox;
			std::vector<TerraGenParam_CatDistrInt> numRidges;
			double minNodeDistance;
			uint32_t numTries;
		};
		inline void to_objmap(const MountainTopRidgeGenerator1_Params &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "MountainTopRidgeGenerator1_Params", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			om.addValue("type", obj.type.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			to_objmap(obj.alpha, om.addObject("alpha"));
			to_objmap(obj.beta, om.addObject("beta"));
			to_objmap(obj.betaBox, om.addObject("betaBox"));
			to_objmap(obj.len, om.addObject("len"));
			to_objmap(obj.lenBox, om.addObject("lenBox"));
			for (auto x : obj.numRidges)
				to_objmap(x, om.addObject("numRidges"));
			zenith::util::str_cast(obj.minNodeDistance, buff, 128);
			om.addValue("minNodeDistance", buff);
			zenith::util::str_cast(obj.numTries, buff, 128);
			om.addValue("numTries", buff);
		}
		inline void from_objmap(MountainTopRidgeGenerator1_Params &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.type, "type");
			from_objmap(obj.alpha, om.getObjects("alpha", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.beta, om.getObjects("beta", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.betaBox, om.getObjects("betaBox", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.len, om.getObjects("len", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.lenBox, om.getObjects("lenBox", zenith::util::ObjectMapPresence::ONE).first->second);
			{
				auto v = om.getObjects("numRidges");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.numRidges.resize(obj.numRidges.size()+1);
					from_objmap(obj.numRidges.back(), it->second);
				}
			}
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.minNodeDistance, "minNodeDistance", 10.0);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.numTries, "numTries", 1);
		}


		struct MountainContGenerator1_Params
		{
			zenith::util::nameid uid;
			std::string type;
			TerraGenParam_MuSigma alpha;
			TerraGenParam_Box1D alphaBox;
			TerraGenParam_MuSigma beta;
			TerraGenParam_Box1D betaBox;
			TerraGenParam_MuSigma mix;
			TerraGenParam_Box1D mixBox;
			TerraGenParam_MuSigma len;
			TerraGenParam_Box1D lenBox;
			double minNodeDistance;
			uint32_t numTries;
		};
		inline void to_objmap(const MountainContGenerator1_Params &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "MountainContGenerator1_Params", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			om.addValue("type", obj.type.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			to_objmap(obj.alpha, om.addObject("alpha"));
			to_objmap(obj.alphaBox, om.addObject("alphaBox"));
			to_objmap(obj.beta, om.addObject("beta"));
			to_objmap(obj.betaBox, om.addObject("betaBox"));
			to_objmap(obj.mix, om.addObject("mix"));
			to_objmap(obj.mixBox, om.addObject("mixBox"));
			to_objmap(obj.len, om.addObject("len"));
			to_objmap(obj.lenBox, om.addObject("lenBox"));
			zenith::util::str_cast(obj.minNodeDistance, buff, 128);
			om.addValue("minNodeDistance", buff);
			zenith::util::str_cast(obj.numTries, buff, 128);
			om.addValue("numTries", buff);
		}
		inline void from_objmap(MountainContGenerator1_Params &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.type, "type");
			from_objmap(obj.alpha, om.getObjects("alpha", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.alphaBox, om.getObjects("alphaBox", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.beta, om.getObjects("beta", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.betaBox, om.getObjects("betaBox", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.mix, om.getObjects("mix", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.mixBox, om.getObjects("mixBox", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.len, om.getObjects("len", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.lenBox, om.getObjects("lenBox", zenith::util::ObjectMapPresence::ONE).first->second);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.minNodeDistance, "minNodeDistance", 10.0);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.numTries, "numTries", 1);
		}


		struct MountainForkGenerator1_Params
		{
			zenith::util::nameid uid;
			std::string type;
			TerraGenParam_MuSigma alpha0;
			TerraGenParam_Box1D alpha0Box;
			TerraGenParam_MuSigma alpha1;
			TerraGenParam_Box1D alpha1Box;
			TerraGenParam_MuSigma beta;
			TerraGenParam_Box1D betaBox;
			TerraGenParam_MuSigma mix;
			TerraGenParam_Box1D mixBox;
			TerraGenParam_MuSigma len;
			TerraGenParam_Box1D lenBox;
			double minNodeDistance;
			uint32_t numTries;
		};
		inline void to_objmap(const MountainForkGenerator1_Params &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "MountainForkGenerator1_Params", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			om.addValue("type", obj.type.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			to_objmap(obj.alpha0, om.addObject("alpha0"));
			to_objmap(obj.alpha0Box, om.addObject("alpha0Box"));
			to_objmap(obj.alpha1, om.addObject("alpha1"));
			to_objmap(obj.alpha1Box, om.addObject("alpha1Box"));
			to_objmap(obj.beta, om.addObject("beta"));
			to_objmap(obj.betaBox, om.addObject("betaBox"));
			to_objmap(obj.mix, om.addObject("mix"));
			to_objmap(obj.mixBox, om.addObject("mixBox"));
			to_objmap(obj.len, om.addObject("len"));
			to_objmap(obj.lenBox, om.addObject("lenBox"));
			zenith::util::str_cast(obj.minNodeDistance, buff, 128);
			om.addValue("minNodeDistance", buff);
			zenith::util::str_cast(obj.numTries, buff, 128);
			om.addValue("numTries", buff);
		}
		inline void from_objmap(MountainForkGenerator1_Params &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.type, "type");
			from_objmap(obj.alpha0, om.getObjects("alpha0", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.alpha0Box, om.getObjects("alpha0Box", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.alpha1, om.getObjects("alpha1", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.alpha1Box, om.getObjects("alpha1Box", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.beta, om.getObjects("beta", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.betaBox, om.getObjects("betaBox", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.mix, om.getObjects("mix", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.mixBox, om.getObjects("mixBox", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.len, om.getObjects("len", zenith::util::ObjectMapPresence::ONE).first->second);
			from_objmap(obj.lenBox, om.getObjects("lenBox", zenith::util::ObjectMapPresence::ONE).first->second);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.minNodeDistance, "minNodeDistance", 10.0);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.numTries, "numTries", 1);
		}

	}
}


#pragma warning(pop)
