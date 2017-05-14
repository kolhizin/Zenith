#include "spirv-reflection.h"
#include "Utils\destructor_utils.h"

void zenith::spirv::Module::handleDecorateDescriptor_(uint16_t opCode, uint32_t trgt, uint32_t idx, uint32_t arg, bool dscSet)
{
	SDataHandle h = SDataNull;
	uint16_t idx16 = 0xFFFF;
	if (opCode == 72)
		throw SPIRVInstructionException("Module::handleDecorateDescriptor_: member decorate for descriptor-set / descriptor-binding is not supported.");
	for(SDataHandle h0 = 0; h0 < sAlloc_.top(); h0++)
		if (sDataBuff_[h0].spirvID == trgt && sDataBuff_[h0].type == InfoType::DESCRIPTOR_INFO)
		{
			h = h0;
			break;
		}
	if (h == SDataNull)
	{
		h = newSInfo_();
		sDataBuff_[h].spirvID = trgt;
		sDataBuff_[h].type = InfoType::DESCRIPTOR_INFO;
	}

	
	DescriptorInfo &db = sDataBuff_[h].descInfo;
	if(dscSet)
		db.dscSet = arg;
	else
		db.dscBind = arg;
}

void zenith::spirv::Module::handleDecorateBlock_(uint16_t opCode, uint32_t trgt, uint32_t idx, uint32_t arg, bool isSSBOlike)
{
	SDataHandle h = SDataNull;
	uint16_t idx16 = 0xFFFF;
	if (opCode == 72)
		throw SPIRVInstructionException("Module::handleDecorateBlock_: member decorate for descriptor-set / descriptor-binding is not supported.");
	for (SDataHandle h0 = 0; h0 < sAlloc_.top(); h0++)
		if (sDataBuff_[h0].spirvID == trgt && sDataBuff_[h0].type == InfoType::BLOCK_INFO)
		{
			h = h0;
			break;
		}
	if (h == SDataNull)
	{
		h = newSInfo_();
		sDataBuff_[h].spirvID = trgt;
		sDataBuff_[h].type = InfoType::BLOCK_INFO;
	}


	BlockInfo &db = sDataBuff_[h].blockInfo;
	db.isSSBOlike = isSSBOlike;
}

void zenith::spirv::Module::handleVariable_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs)
{
	if (opCode == 59)
	{
		uint32_t typeID = *(opArgs++);
		uint32_t spirvID = *(opArgs++);
		SDataHandle th = newSInfo_();
		StructuredInfo &si = sDataBuff_[th];
		si.spirvID = spirvID;
		si.type = InfoType::VAR_INFO;
		VariableInfo &vi = si.varInfo;
		vi.typeHandle = getHandleByID_(typeID, InfoType::TYPE_INFO);
		vi.storageType = *(opArgs++);
	}
}

void zenith::spirv::Module::handleConstant_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs)
{
	if (opCode >= 41 && opCode <= 43)
	{
		uint32_t typeID = *(opArgs++);
		uint32_t spirvID = *(opArgs++);
		SDataHandle th = newSInfo_();
		StructuredInfo &si = sDataBuff_[th];
		si.spirvID = spirvID;
		si.type = InfoType::CONST_INFO;
		ConstantInfo &ci = si.constInfo;
		ci.typeHandle = getHandleByID_(typeID, InfoType::TYPE_INFO);
		ci.type = ConstantType::OTHER;
		ci.value0 = 0;
		if (opCode == 41)
			ci.type = ConstantType::BOOL_TRUE;
		else if (opCode == 42)
			ci.type = ConstantType::BOOL_FALSE;
		else if (opCode == 43 &&
			(sDataBuff_[ci.typeHandle].typeInfo.simple.base == TypeBase::UINT32 || sDataBuff_[ci.typeHandle].typeInfo.simple.base == TypeBase::UINT16 || sDataBuff_[ci.typeHandle].typeInfo.simple.base == TypeBase::UINT8
				|| sDataBuff_[ci.typeHandle].typeInfo.simple.base == TypeBase::INT32 || sDataBuff_[ci.typeHandle].typeInfo.simple.base == TypeBase::INT16 || sDataBuff_[ci.typeHandle].typeInfo.simple.base == TypeBase::INT8))
		{
			ci.type = ConstantType::UINT32_VAL;
			ci.value0 = *(opArgs++);
		}
		else ci.type = ConstantType::OTHER;			
	}
}

void zenith::spirv::Module::handleType_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs)
{
	if (opCode >= 19 && opCode <= 33)
	{
		uint32_t resID = *(opArgs++);
		SDataHandle th = newSInfo_();
		StructuredInfo &si = sDataBuff_[th];
		si.spirvID = resID;
		si.type = InfoType::TYPE_INFO;
		TypeInfo &tp = si.typeInfo;
		if (opCode <= 24)
		{
			tp.metaType = TypeMETA::SIMPLE;
			TypeSimple &ts = tp.simple;
			if (opCode <= 22)
			{
				ts.dim = TypeDim::SINGLE;
				if (opCode == 19)
					ts.base = TypeBase::VOID;
				else if (opCode == 20)
					ts.base = TypeBase::BOOL;
				else if (opCode == 21)
				{
					uint32_t w = *(opArgs++);
					bool sign = (*(opArgs++) > 0);
					switch (w)
					{
					case 8: ts.base = (sign ? TypeBase::INT8 : TypeBase::UINT8); break;
					case 16: ts.base = (sign ? TypeBase::INT16 : TypeBase::UINT16); break;
					case 32: ts.base = (sign ? TypeBase::INT32 : TypeBase::UINT32); break;
					case 64: ts.base = (sign ? TypeBase::INT64 : TypeBase::UINT64); break;
					default: throw SPIRVInstructionException("Module::handleType_: unsupported integer type width.");
					}
				}
				else /*if (opCode == 22)*/
				{
					uint32_t w = *(opArgs++);
					switch (w)
					{
					case 16: ts.base = TypeBase::FLOAT16; break;
					case 32: ts.base = TypeBase::FLOAT32; break;
					case 64: ts.base = TypeBase::FLOAT64; break;
					case 80: ts.base = TypeBase::FLOAT80; break;
					default: throw SPIRVInstructionException("Module::handleType_: unsupported float type width.");
					}
				}
				return;
			}
			else if (opCode == 23)
			{
				uint32_t subID = *(opArgs++);
				uint32_t w = *(opArgs++);
				SDataHandle sth = getHandleByID_(subID, InfoType::TYPE_INFO);
				TypeInfo &stp = sDataBuff_[sth].typeInfo;
				ts = stp.simple;
				switch (w)
				{
				case 2: ts.dim = TypeDim::VEC2; return;
				case 3: ts.dim = TypeDim::VEC3; return;
				case 4: ts.dim = TypeDim::VEC4; return;
				default: throw SPIRVInstructionException("Module::handleType_: unsupported vec type width.");
				}
			}
			else if (opCode == 24)
			{
				uint32_t subID = *(opArgs++);
				uint32_t w = *(opArgs++);
				SDataHandle sth = getHandleByID_(subID, InfoType::TYPE_INFO);
				TypeInfo &stp = sDataBuff_[sth].typeInfo;
				ts = stp.simple;
				switch (w)
				{
				case 2: ts.dim = (stp.simple.dim == TypeDim::VEC2 ? TypeDim::MAT2X2 : (stp.simple.dim == TypeDim::VEC3 ? TypeDim::MAT3X2 : TypeDim::MAT4X2)); return;
				case 3: ts.dim = (stp.simple.dim == TypeDim::VEC2 ? TypeDim::MAT2X3 : (stp.simple.dim == TypeDim::VEC3 ? TypeDim::MAT3X3 : TypeDim::MAT4X3)); return;
				case 4: ts.dim = (stp.simple.dim == TypeDim::VEC2 ? TypeDim::MAT2X4 : (stp.simple.dim == TypeDim::VEC3 ? TypeDim::MAT3X4 : TypeDim::MAT4X4)); return;
				default: throw SPIRVInstructionException("Module::handleType_: unsupported mat type width.");
				}
			}
			throw SPIRVInstructionException("Module::handleType_: unsupported type instruction.");
		}
		else if (opCode == 25)
		{
			tp.metaType = TypeMETA::IMAGE;
			tp.image.baseTypeHandle = getHandleByID_(*(opArgs++), InfoType::TYPE_INFO);
			uint32_t dim = *(opArgs++);
			tp.image.isDepth = *(opArgs++);
			tp.image.isArray = *(opArgs++);
			tp.image.isMS = *(opArgs++);
			tp.image.isSampled = *(opArgs++);
			tp.image.imageFormat = *(opArgs++);
			tp.image.accessQual = *(opArgs++);
			switch (dim)
			{
			case 0: tp.image.imageDim = TypeImageDim::I1D; return;
			case 1: tp.image.imageDim = TypeImageDim::I2D; return;
			case 2: tp.image.imageDim = TypeImageDim::I3D; return;
			case 3: tp.image.imageDim = TypeImageDim::CUBE; return;
			case 4: tp.image.imageDim = TypeImageDim::RECT; return;
			case 5: tp.image.imageDim = TypeImageDim::BUFFER; return;
			case 6: tp.image.imageDim = TypeImageDim::SUBPASS; return;
			}
			throw SPIRVInstructionException("Module::handleType_: unsupported image dimension.");
		}
		else if (opCode <= 27)
		{
			if (opCode == 26)
			{
				tp.metaType = TypeMETA::SAMPLER;
			}
			else
			{
				tp.metaType = TypeMETA::SAMPLED_IMAGE;
				tp.reference.refTypeHandle = getHandleByID_(*(opArgs++), InfoType::TYPE_INFO);
			}
			return;
		}
		else if (opCode <= 29)
		{
			tp.metaType = TypeMETA::ARRAY;
			tp.reference.refTypeHandle = getHandleByID_(*(opArgs++), InfoType::TYPE_INFO);
			tp.reference.sizeHandle = SDataNull;
			if (opCode == 28)
				tp.reference.sizeHandle = getHandleByID_(*(opArgs++), InfoType::CONST_INFO);
			return;
		}
		else if (opCode == 30)
		{
			tp.metaType = TypeMETA::STRUCT;
			auto bePair = newSubTypes_(opLen - 2);
			tp.combination.beginHandle = bePair.first;
			tp.combination.endHandle = bePair.second;
			for (RawDataHandle h0 = tp.combination.beginHandle; h0 < tp.combination.endHandle; h0+=sizeof(SPIRVHandle))
				*reinterpret_cast<SPIRVHandle *>(&rawDataBuff_[h0]) = *(opArgs++);
			return;
		}
		else if (opCode == 31)
		{
			throw SPIRVInstructionException("Module::handleType_: unsupported opaque types.");
		}
		else if (opCode == 32)
		{
			tp.metaType = TypeMETA::POINTER;
			tp.reference.storageType = *(opArgs++);
			tp.reference.refTypeHandle = getHandleByID_(*(opArgs++), InfoType::TYPE_INFO);
			return;
		}
		else if (opCode == 33)
		{
			tp.metaType = TypeMETA::FUNCTION;
			auto bePair = newSubTypes_(opLen - 2);
			tp.combination.beginHandle = bePair.first;
			tp.combination.endHandle = bePair.second;
			for (RawDataHandle h0 = tp.combination.beginHandle; h0 < tp.combination.endHandle; h0 += sizeof(SPIRVHandle))
				*reinterpret_cast<SPIRVHandle *>(&rawDataBuff_[h0]) = *(opArgs++);
			return;
		}
	}
	else throw SPIRVInstructionException("Module::handleType_: unsupported type-definition instruction.");
}

void zenith::spirv::Module::handleDecorate_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs)
{
	if (opCode > 72)
		throw SPIRVInstructionException("Module::handleDecorate_: group decorations are not supported.");
	uint32_t trgt = *(opArgs++);
	uint32_t indx = 0;
	if (opCode == 72)
		indx = *(opArgs++);

	uint32_t decoration = *(opArgs++);

	switch (decoration)
	{
	case 2: return handleDecorateBlock_(opCode, trgt, indx, *opArgs, false);
	case 3: return handleDecorateBlock_(opCode, trgt, indx, *opArgs, true);
	case 33: return handleDecorateDescriptor_(opCode, trgt, indx, *opArgs, false);
	case 34: return handleDecorateDescriptor_(opCode, trgt, indx, *opArgs, true); 
	default: return;
	}
}

void zenith::spirv::Module::handleName_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs)
{
	auto si = newSInfo_();
	sDataBuff_[si].type = InfoType::NAME_INFO;
	sDataBuff_[si].spirvID = *(opArgs++);
	NameInfo &nb = sDataBuff_[si].nameInfo;
	nb.idx = 0;
	opLen -= 2;
	if (opCode == 6)
	{
		nb.idx = *(opArgs++);
		opLen--;
	}
	else nb.idx = 0xFFFF;

	size_t slen = strlen(reinterpret_cast<const char *>(opArgs));
	nb.name = newName_(static_cast<uint32_t>(slen));

	strcpy_s(reinterpret_cast<char *>(&rawDataBuff_[nb.name]), slen+1, reinterpret_cast<const char *>(opArgs));
}

void zenith::spirv::Module::handleOpCode_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs)
{
	if(opCode == 5 || opCode == 6)
		return handleName_(opLen, opCode, opArgs);
	if (opCode >= 19 && opCode <= 33)
		return handleType_(opLen, opCode, opArgs);
	if (opCode == 59)
		return handleVariable_(opLen, opCode, opArgs);
	if (opCode >= 41 && opCode <= 43)
		return handleConstant_(opLen, opCode, opArgs);
	if (opCode >= 71 && opCode <= 75)
		return handleDecorate_(opLen, opCode, opArgs);
}

void zenith::spirv::Module::deduceBindingType_(SDataHandle userBindingHandle)
{
	UserDscBindingInfo &ubi = sDataBuff_[userBindingHandle].userBindingInfo;
	if (sDataBuff_[ubi.fkBaseType].typeInfo.metaType == TypeMETA::STRUCT)
	{
		if (ubi.fkBlock == SDataNull)
			throw SPIRVLogicException("Module::deduceBindingType_: UBO/SSBO should have been decorated appropriately in SPIR-V code.");
		if (sDataBuff_[ubi.fkBlock].blockInfo.isSSBOlike)
			ubi.bindingType = DescriptorBindingType::STORAGE_BUFFER;
		else
			ubi.bindingType = DescriptorBindingType::UNIFORM_BUFFER;
	}
	else if (sDataBuff_[ubi.fkBaseType].typeInfo.metaType == TypeMETA::IMAGE)
	{
		if (ubi.fkSampler != SDataNull)
			ubi.bindingType = DescriptorBindingType::COMBINED_TEXTURE_SAMPLER;
		else
		{
			if(sDataBuff_[ubi.fkBaseType].type != InfoType::TYPE_INFO || sDataBuff_[ubi.fkBaseType].typeInfo.metaType != TypeMETA::IMAGE)
				throw SPIRVLogicException("Module::deduceBindingType_: incorrect underlying type of image.");
			TypeImage &ti = sDataBuff_[ubi.fkBaseType].typeInfo.image;
			if (ti.imageFormat = 0) /*unknown format => sampled / not buffer*/
			{
				
				if (ti.imageDim == TypeImageDim::BUFFER)
					ubi.bindingType = DescriptorBindingType::UNIFORM_TEXEL_BUFFER;
				else if (ti.imageDim == TypeImageDim::SUBPASS)
					ubi.bindingType = DescriptorBindingType::INPUT_ATTACHMENT;
				else
					ubi.bindingType = DescriptorBindingType::TEXTURE;
			}
			else /*known format => not sampled / buffer*/
			{
				if (ti.imageDim == TypeImageDim::BUFFER)
					ubi.bindingType = DescriptorBindingType::STORAGE_TEXEL_BUFFER;
				else
					ubi.bindingType = DescriptorBindingType::STORAGE_TEXTURE;
			}
		}
	}
	else if (sDataBuff_[ubi.fkBaseType].typeInfo.metaType == TypeMETA::SAMPLER)
		ubi.bindingType = DescriptorBindingType::SAMPLER;
	else
		throw SPIRVLogicException("Module::deduceBindingType_: unable to deduce binding type.");
}

void zenith::spirv::Module::sortBindings_(SDataHandle begin, SDataHandle end)
{
	/*naive bubble sort*/
	uint32_t opsPerformed = 1;
	StructuredInfo tmp;
	while (opsPerformed > 0)
	{
		opsPerformed = 0;
		SDataHandle cur = begin + 1, prev = begin;
		uint32_t prevSet = sDataBuff_[sDataBuff_[prev].userBindingInfo.fkDescriptor].descInfo.dscSet;
		uint32_t prevBind = sDataBuff_[sDataBuff_[prev].userBindingInfo.fkDescriptor].descInfo.dscBind;
		for (; cur < end; cur++, prev++)
		{
			uint32_t curSet = sDataBuff_[sDataBuff_[cur].userBindingInfo.fkDescriptor].descInfo.dscSet;
			uint32_t curBind = sDataBuff_[sDataBuff_[cur].userBindingInfo.fkDescriptor].descInfo.dscBind;
			if (curSet < prevSet || (curSet == prevSet && curBind < prevBind))
			{
				opsPerformed++;
				memcpy(&tmp, &sDataBuff_[cur], sizeof(sDataBuff_[0]));
				memcpy(&sDataBuff_[cur], &sDataBuff_[prev], sizeof(sDataBuff_[0]));
				memcpy(&sDataBuff_[prev], &tmp, sizeof(sDataBuff_[0]));
			}
			else
			{
				prevSet = curSet;
				prevBind = curBind;
			}
		}
	}
}

void zenith::spirv::Module::checkBindings_(SDataHandle begin, SDataHandle end)
{
	SDataHandle cur = begin + 1, prev = begin;
	uint32_t prevSet = sDataBuff_[sDataBuff_[prev].userBindingInfo.fkDescriptor].descInfo.dscSet;
	uint32_t prevBind = sDataBuff_[sDataBuff_[prev].userBindingInfo.fkDescriptor].descInfo.dscBind;
	for (; cur < end; cur++, prev++)
	{
		uint32_t curSet = sDataBuff_[sDataBuff_[cur].userBindingInfo.fkDescriptor].descInfo.dscSet;
		uint32_t curBind = sDataBuff_[sDataBuff_[cur].userBindingInfo.fkDescriptor].descInfo.dscBind;
		if (curSet == prevSet)
		{
			if (sDataBuff_[prev].userBindingInfo.isArrayed && sDataBuff_[prev].userBindingInfo.arraySize == 0)
			{
				/*do nothing -- dynamic arrays*/
			}
			else if (sDataBuff_[prev].userBindingInfo.isArrayed)
			{
				if (sDataBuff_[prev].userBindingInfo.arraySize + prevBind > curBind)
					throw SPIRVLogicException("Module::checkBindings_: check failed due to array binding overlapping with next.");
			}
			else
			{
				if (1 + prevBind > curBind)
					throw SPIRVLogicException("Module::checkBindings_: check failed due to overlapping bindings.");
			}
		}
		prevSet = curSet;
		prevBind = curBind;
	}
}

void zenith::spirv::Module::createDescriptorSet_(SDataHandle begin, SDataHandle end)
{
	if (begin == end)
		return;
	SDataHandle hNew = newSInfo_();
	sDataBuff_[hNew].type = InfoType::USER_DSC_SET_INFO;
	sDataBuff_[hNew].spirvID = sDataBuff_[begin].spirvID;
	UserDscSetInfo &usi = sDataBuff_[hNew].userSetInfo;
	usi.firstBindingInfo = begin;
	usi.numBindingInfo = end - begin;
	usi.setIndex = sDataBuff_[sDataBuff_[begin].userBindingInfo.fkDescriptor].descInfo.dscSet;

	uint32_t slotNum = (sDataBuff_[begin].userBindingInfo.isArrayed && sDataBuff_[begin].userBindingInfo.arraySize > 0 ? sDataBuff_[begin].userBindingInfo.arraySize : 1);
	usi.maxBindingSlot = sDataBuff_[sDataBuff_[begin].userBindingInfo.fkDescriptor].descInfo.dscBind + slotNum - 1;
	
	for (SDataHandle h = begin + 1; h < end; h++)
	{
		DescriptorInfo &di = sDataBuff_[sDataBuff_[h].userBindingInfo.fkDescriptor].descInfo;
		if (di.dscSet != usi.setIndex)
			throw SPIRVLogicException("Module::createDescriptorSet_: descriptor-set mismatch inside one block.");

		uint32_t slotNum = (sDataBuff_[h].userBindingInfo.isArrayed && sDataBuff_[h].userBindingInfo.arraySize > 0 ? sDataBuff_[h].userBindingInfo.arraySize : 1);
		uint32_t maxSlot = di.dscBind + slotNum - 1;
		if (maxSlot > usi.maxBindingSlot)
			usi.maxBindingSlot = maxSlot;
	}
}

void zenith::spirv::Module::createDescriptorLayout_(SDataHandle begin, SDataHandle end)
{
	if (begin == end)
	{
		userDscSetInfoBegin_ = userDscSetInfoEnd_ = begin;
		return;
	}
	SDataHandle h = begin;
	SDataHandle sBegin = begin;
	
	SDataHandle tmpTop = static_cast<SDataHandle>(sAlloc_.top());

	uint16_t prevSet = sDataBuff_[sDataBuff_[h].userBindingInfo.fkDescriptor].descInfo.dscSet;
	h++;
	for (; h < end; h++)
	{
		uint16_t curSet = sDataBuff_[sDataBuff_[h].userBindingInfo.fkDescriptor].descInfo.dscSet;
		if (curSet != prevSet)
		{
			createDescriptorSet_(sBegin, h);
			sBegin = h;
		}
		prevSet = curSet;
	}
	createDescriptorSet_(sBegin, end);

	userDscSetInfoBegin_ = tmpTop;
	userDscSetInfoEnd_ = static_cast<SDataHandle>(sAlloc_.top());
}

void zenith::spirv::Module::compileBindingInfo()
{
	SDataHandle maxSData = static_cast<SDataHandle>(sAlloc_.top());
	auto cleanup = zenith::util::doOnScopeExit([&]() {sAlloc_.freeUntil(maxSData);}); /*in case of exception free all done allocations.*/
	for(SDataHandle h = 0; h < maxSData; h++)
		if (sDataBuff_[h].type == InfoType::DESCRIPTOR_INFO)
		{
			SDataHandle hNew = newSInfo_();
			sDataBuff_[hNew].type = InfoType::USER_DSC_BINDING_INFO;
			sDataBuff_[hNew].spirvID = sDataBuff_[h].spirvID;
			UserDscBindingInfo &ubi = sDataBuff_[hNew].userBindingInfo;

			ubi.bindingType = DescriptorBindingType::UNDEF;
			ubi.fkDescriptor = h;
			ubi.isArrayed = false;
			ubi.arraySize = 1;
			SDataHandle th = SDataNull;
			for(SDataHandle h1 = 0; h1 < maxSData; h1++)
				if (sDataBuff_[h1].type == InfoType::VAR_INFO && sDataBuff_[h1].spirvID == sDataBuff_[h].spirvID)
				{
					th = sDataBuff_[h1].varInfo.typeHandle;
					if (sDataBuff_[h1].varInfo.storageType != 2 && sDataBuff_[h1].varInfo.storageType != 0) /*check that storage is <uniform> or <uniform-constant>*/
						throw SPIRVLogicException("Module::compileBindingInfo: storage type of uniform variable should be <uniform> or <uniform-constant>.");
					break;
				}
			for (SDataHandle h1 = 0; h1 < maxSData; h1++)
				if (sDataBuff_[h1].type == InfoType::NAME_INFO && sDataBuff_[h1].spirvID == sDataBuff_[h].spirvID)
				{
					ubi.fkName = sDataBuff_[h1].nameInfo.name;
					break;
				}
			if(th == SDataNull)
				throw SPIRVLogicException("Module::compileBindingInfo: could not find corresponding type for decoration.");
			while (sDataBuff_[th].typeInfo.metaType == TypeMETA::POINTER || sDataBuff_[th].typeInfo.metaType == TypeMETA::ARRAY)
			{
				if (sDataBuff_[th].typeInfo.metaType == TypeMETA::ARRAY)
				{
					if (sDataBuff_[th].typeInfo.reference.sizeHandle != SDataNull)
					{
						auto &ci = sDataBuff_[sDataBuff_[th].typeInfo.reference.sizeHandle].constInfo;
						if(ci.type != ConstantType::UINT32_VAL)
							throw SPIRVLogicException("Module::compileBindingInfo: invalid array size type.");
						ubi.isArrayed = true;
						ubi.arraySize = ubi.arraySize * ci.value0;
					}
					else
					{
						ubi.isArrayed = true;
						ubi.arraySize = 0;
					}
				}
				th = sDataBuff_[th].typeInfo.reference.refTypeHandle;
			}
			ubi.fkBaseType = th;
			ubi.fkBlock = SDataNull;
			auto &ti = sDataBuff_[th].typeInfo;
			if (ti.metaType == TypeMETA::SAMPLED_IMAGE)
			{
				ubi.fkBaseType = ti.reference.refTypeHandle;
				ubi.fkSampler = th;
			}
			else if (ti.metaType == TypeMETA::STRUCT)
			{
				for (SDataHandle h1 = 0; h1 < maxSData; h1++)
					if (sDataBuff_[h1].type == InfoType::BLOCK_INFO && sDataBuff_[h1].spirvID == sDataBuff_[th].spirvID)
					{
						ubi.fkBlock = h1;
						break;
					}
			}

			deduceBindingType_(hNew);
		}

	sortBindings_(maxSData, static_cast<SDataHandle>(sAlloc_.top()));
	checkBindings_(maxSData, static_cast<SDataHandle>(sAlloc_.top()));

	SDataHandle userBindingInfoEnd = static_cast<SDataHandle>(sAlloc_.top());

	createDescriptorLayout_(maxSData, static_cast<SDataHandle>(sAlloc_.top()));

	cleanup.deactivate();
	userDscBindingInfoBegin_ = maxSData;
	userDscBindingInfoEnd_ = userBindingInfoEnd;
}

zenith::spirv::LayoutInfo zenith::spirv::Module::getBindingLayoutInfo() const
{
	if (userDscBindingInfoBegin_ == SDataNull || userDscSetInfoBegin_ == SDataNull)
		throw SPIRVLogicException("Module::getBindingLayoutInfo: module should call compileBindingInfo() first.");
	uint16_t maxSet = 0;
	for (SDataHandle h = userDscSetInfoBegin_; h < userDscBindingInfoEnd_; h++)
		if (sDataBuff_[h].userSetInfo.setIndex > maxSet)
			maxSet = sDataBuff_[h].userSetInfo.setIndex;
	return LayoutInfo(this, maxSet, userDscSetInfoBegin_, userDscSetInfoEnd_);
}

zenith::spirv::SDataHandle zenith::spirv::LayoutInfo::getSetHandleByIndex(uint16_t setIndex) const
{
	SDataHandle h = userDscSetInfoBegin_ + setIndex;
	if (h < userDscSetInfoEnd_)
		return h;
	return SDataNull;
}

zenith::spirv::SDataHandle zenith::spirv::LayoutInfo::getSetHandleBySlot(uint16_t setSlot) const
{
	for (SDataHandle h = userDscSetInfoBegin_; h < userDscSetInfoEnd_; h++)
		if (module_->sDataBuff_[h].userSetInfo.setIndex < setSlot)
			continue;
		else if (module_->sDataBuff_[h].userSetInfo.setIndex == setSlot)
			return h;
		else break;

	return SDataNull;
}

zenith::spirv::LayoutSetInfo zenith::spirv::LayoutInfo::getSetInfoByHandle(SDataHandle setHandle) const
{
	if(setHandle < userDscSetInfoBegin_ || setHandle >= userDscSetInfoEnd_)
		throw SPIRVLogicException("LayoutInfo::getSetInfoByHandle: invalid handle.");
	const Module::UserDscSetInfo &usi = module_->sDataBuff_[setHandle].userSetInfo;
	return LayoutSetInfo(module_, usi.setIndex, usi.maxBindingSlot, setHandle, usi.firstBindingInfo, usi.firstBindingInfo + usi.numBindingInfo);
}

zenith::spirv::SDataHandle zenith::spirv::LayoutSetInfo::getBindingHandleByIndex(uint16_t bindingIndex) const
{
	SDataHandle h = userDscBindingInfoBegin_ + bindingIndex;
	if (h < userDscBindingInfoEnd_)
		return h;
	return SDataNull;
}

zenith::spirv::SDataHandle zenith::spirv::LayoutSetInfo::getBindingHandleBySlot(uint16_t bindingSlot) const
{
	for (SDataHandle h = userDscBindingInfoBegin_; h < userDscBindingInfoEnd_; h++)
	{
		const Module::DescriptorInfo &di = module_->sDataBuff_[module_->sDataBuff_[h].userBindingInfo.fkDescriptor].descInfo;
		if (di.dscBind < bindingSlot)
			continue;
		else if (di.dscBind == bindingSlot)
			return h;
		else break;
	}

	return SDataNull;
}

zenith::spirv::LayoutBindingInfo zenith::spirv::LayoutSetInfo::getBindingInfoByHandle(SDataHandle bindingHandle) const
{
	if (bindingHandle < userDscBindingInfoBegin_ || bindingHandle >= userDscBindingInfoEnd_)
		throw SPIRVLogicException("LayoutSetInfo::getBindingInfoByHandle: invalid handle.");
	return LayoutBindingInfo(module_, bindingHandle);
}

zenith::spirv::LayoutBindingInfo::LayoutBindingInfo(const Module * module, SDataHandle bindingHandle) : module_(module), userDscBindingHandle_(bindingHandle)
{
	const Module::UserDscBindingInfo &ubi = module_->sDataBuff_[userDscBindingHandle_].userBindingInfo;
	bindingData_.isArray = ubi.isArrayed;
	if (ubi.isArrayed)
		bindingData_.arraySize = ubi.arraySize;
	else
		bindingData_.arraySize = 1;

	bindingData_.binding = module_->sDataBuff_[ubi.fkDescriptor].descInfo.dscBind;
	bindingData_.set = module_->sDataBuff_[ubi.fkDescriptor].descInfo.dscSet;
	bindingData_.bindingType = ubi.bindingType;
	bindingData_.pName = reinterpret_cast<const char *>(&module_->rawDataBuff_[ubi.fkName]);
	if (*bindingData_.pName == '\0')
		bindingData_.pName = nullptr;
}

zenith::spirv::TypeInfo zenith::spirv::LayoutBindingInfo::getTypeInfo() const
{	
	return TypeInfo(module_, module_->sDataBuff_[userDscBindingHandle_].userBindingInfo.fkBaseType);
}

zenith::spirv::TypeInfo::TypeInfo(const Module * module, SDataHandle typeHandle) : module_(module), typeHandle_(typeHandle), memberBegin_(RawDataNull), memberEnd_(RawDataNull)
{
	if (module_->sDataBuff_[typeHandle_].typeInfo.metaType == TypeMETA::STRUCT)
	{
		memberBegin_ = module_->sDataBuff_[typeHandle_].typeInfo.combination.beginHandle;
		memberEnd_ = module_->sDataBuff_[typeHandle_].typeInfo.combination.endHandle;
	}
}

const char * zenith::spirv::TypeInfo::getMemberName(uint16_t idx) const
{
	if(idx >= numMembers())
		return nullptr;
	SPIRVHandle spirvID = module_->sDataBuff_[typeHandle_].spirvID;
	for(SDataHandle h = 0; h < module_->sAlloc_.top(); h++)
		if (module_->sDataBuff_[h].spirvID == spirvID && module_->sDataBuff_[h].type == Module::InfoType::NAME_INFO && module_->sDataBuff_[h].nameInfo.idx == idx)
		{
			const char * p = reinterpret_cast<const char *>(&module_->rawDataBuff_[module_->sDataBuff_[h].nameInfo.name]);
			if (*p == '\0')
				return nullptr;
			return p;
		}
	return nullptr;
}

zenith::spirv::TypeInfo zenith::spirv::TypeInfo::getMemberTypeInfo(uint16_t idx) const
{
	if (idx >= numMembers())
		throw SPIRVLogicException("TypeInfo::getMemberTypeInfo: invalid member index.");
	SPIRVHandle spirvID = *(reinterpret_cast<const SPIRVHandle *>(&module_->rawDataBuff_[memberBegin_]) + idx);
	for (SDataHandle h = 0; h < module_->sAlloc_.top(); h++)
		if(module_->sDataBuff_[h].spirvID == spirvID && module_->sDataBuff_[h].type == Module::InfoType::TYPE_INFO)
			return TypeInfo(module_, h);
	throw SPIRVLogicException("TypeInfo::getMemberTypeInfo: failed to find matching type info.");
}

const char * zenith::spirv::TypeInfo::getTypeName() const
{
	return nullptr;
}
