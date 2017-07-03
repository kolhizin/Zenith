#include "NodeFactory.h"
#include "MountainNode.h"
#include <Utils\ioconv\io_config.h>
#include <Utils\ioconv\output_objmap.h>
#include <Utils\ioconv\input_objmap.h>

using namespace zenith::terragen;

template<class Gen, class GenParam>
Gen * constructGenerator_(const zenith::util::ObjectMap<char, char> &descr)
{
	GenParam gp;
	zenith::util::ioconv::input_objmap_root om_it0(descr);
	zenith::util::ioconv::input(gp, om_it0);
	return constructGenerator<Gen, GenParam>(gp);
}
template<class Gen, class GenParam>
size_t constructGenerator_(const zenith::util::ObjectMap<char, char> &descr, void * buffPtr, size_t buffSize)
{
	GenParam gp;
	zenith::util::ioconv::input_objmap_root om_it0(descr);
	zenith::util::ioconv::input(gp, om_it0);
	return constructGenerator<Gen, GenParam>(gp, buffPtr, buffSize);
}


template<class Gen, class GenParam>
Gen * constructMetaGenerator_(const zenith::util::ObjectMap<char, char> &descr)
{
	GenParam gp;
	zenith::util::ioconv::input_objmap_root om_it0(descr);
	zenith::util::ioconv::input(gp, om_it0);
	return constructMetaGenerator<Gen, GenParam>(gp);
}
template<class Gen, class GenParam>
size_t constructMetaGenerator_(const zenith::util::ObjectMap<char, char> &descr, void * buffPtr, size_t buffSize)
{
	GenParam gp;
	zenith::util::ioconv::input_objmap_root om_it0(descr);
	zenith::util::ioconv::input(gp, om_it0);
	return constructMetaGenerator<Gen, GenParam>(gp, buffPtr, buffSize);
}


inline void tgnfParseArguments_(const zenith::util::ObjectMap<char, char> &descr, std::string &gType, std::string &gTypeParams)
{
	OBJMAP_GET_ONE_VALUE(descr, gType, "type");
	if (descr.getNumValues("paramType") > 1)
		throw TerraGenFactoryException("constructGenerator(from ObjectMap): too many paramTypes!");
	if (descr.getNumValues("paramType") == 1)
	{
		OBJMAP_GET_ONE_VALUE(descr, gTypeParams, "paramType");
	}
	else
		gTypeParams = gType + "_Params";
}

#define TGNF_GEN_NEW(gen,param)if(gType == #gen && gTypeParams == #param)return constructGenerator_<gen,param>(descr)
#define TGNF_GEN_INP(gen,param)if(gType == #gen && gTypeParams == #param)return constructGenerator_<gen,param>(descr,buffPtr,buffSize)

#define TGNF_METAGEN_NEW(gen,param)if(gType == #gen && gTypeParams == #param)return constructMetaGenerator_<gen,param>(descr)
#define TGNF_METAGEN_INP(gen,param)if(gType == #gen && gTypeParams == #param)return constructMetaGenerator_<gen,param>(descr,buffPtr,buffSize)


BaseNodeGenerator * zenith::terragen::constructGenerator(const zenith::util::ObjectMap<char, char> &descr)
{
	std::string gType, gTypeParams;
	tgnfParseArguments_(descr, gType, gTypeParams);
	TGNF_GEN_NEW(MountainTopGenerator1, MountainTopGenerator1_Params);
	TGNF_GEN_NEW(MountainTopRidgeGenerator1, MountainTopRidgeGenerator1_Params);
	TGNF_GEN_NEW(MountainContGenerator1, MountainContGenerator1_Params);
	TGNF_GEN_NEW(MountainForkGenerator1, MountainForkGenerator1_Params);
	throw TerraGenFactoryException("constructGenerator(from ObjectMap): unknown type-paramTypes combination!");
}
size_t zenith::terragen::constructGenerator(const zenith::util::ObjectMap<char, char> &descr, void * buffPtr, size_t buffSize)
{
	std::string gType, gTypeParams;
	tgnfParseArguments_(descr, gType, gTypeParams);
	TGNF_GEN_INP(MountainTopGenerator1, MountainTopGenerator1_Params);
	TGNF_GEN_INP(MountainTopRidgeGenerator1, MountainTopRidgeGenerator1_Params);
	TGNF_GEN_INP(MountainContGenerator1, MountainContGenerator1_Params);
	TGNF_GEN_INP(MountainForkGenerator1, MountainForkGenerator1_Params);
	throw TerraGenFactoryException("constructGenerator(from ObjectMap): unknown type-paramTypes combination!");
}

BaseMetaGenerator * zenith::terragen::constructMetaGenerator(const zenith::util::ObjectMap<char, char>& descr)
{
	std::string gType, gTypeParams;
	tgnfParseArguments_(descr, gType, gTypeParams);
	TGNF_METAGEN_NEW(MountainMetaGenerator1, MountainMetaGenerator1_Params);
	throw TerraGenFactoryException("constructMetaGenerator(from ObjectMap): unknown type-paramTypes combination!");
}

size_t zenith::terragen::constructMetaGenerator(const zenith::util::ObjectMap<char, char>& descr, void * buffPtr, size_t buffSize)
{
	std::string gType, gTypeParams;
	tgnfParseArguments_(descr, gType, gTypeParams);
	TGNF_METAGEN_INP(MountainMetaGenerator1, MountainMetaGenerator1_Params);
	throw TerraGenFactoryException("constructMetaGenerator(from ObjectMap): unknown type-paramTypes combination!");
}

#undef TGNF_GEN_NEW
#undef TGNF_GEN_INP

#undef TGNF_METAGEN_NEW
#undef TGNF_METAGEN_INP
