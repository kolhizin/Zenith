#pragma once

#include <cstdint>
#include <limits>
#include "../../Utils/log_config.h"
#include "../../Utils/alloc_config.h"

namespace zenith
{
	namespace spirv
	{
		class SPIRVException : public zenith::util::LoggedException
		{
		public:
			SPIRVException() : zenith::util::LoggedException() {}
			SPIRVException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
			{
			}
			SPIRVException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
			{
			}
			virtual ~SPIRVException() {}
		};

		class SPIRVResourceException : public SPIRVException
		{
		public:
			SPIRVResourceException() : SPIRVException() {}
			SPIRVResourceException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : SPIRVException(p, type)
			{
			}
			SPIRVResourceException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : SPIRVException(str, type)
			{
			}
			virtual ~SPIRVResourceException() {}
		};
		class SPIRVInstructionException : public SPIRVException
		{
		public:
			SPIRVInstructionException() : SPIRVException() {}
			SPIRVInstructionException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : SPIRVException(p, type)
			{
			}
			SPIRVInstructionException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : SPIRVException(str, type)
			{
			}
			virtual ~SPIRVInstructionException() {}
		};
		class SPIRVLogicException : public SPIRVException
		{
		public:
			SPIRVLogicException() : SPIRVException() {}
			SPIRVLogicException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : SPIRVException(p, type)
			{
			}
			SPIRVLogicException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : SPIRVException(str, type)
			{
			}
			virtual ~SPIRVLogicException() {}
		};


		enum class DescriptorBindingType : uint8_t { UNDEF = 0, UNIFORM_BUFFER, STORAGE_BUFFER, INPUT_ATTACHMENT, STORAGE_TEXEL_BUFFER, UNIFORM_TEXEL_BUFFER, TEXTURE, SAMPLER, COMBINED_TEXTURE_SAMPLER, STORAGE_TEXTURE};
		enum class TypeMETA : uint8_t { UNDEF = 0, SIMPLE, IMAGE, SAMPLER, SAMPLED_IMAGE, ARRAY, POINTER, STRUCT, FUNCTION};
		enum class TypeBase : uint8_t {UNDEF = 0, VOID, BOOL, UINT8, UINT16, UINT32, UINT64, INT8, INT16, INT32, INT64, FLOAT16, FLOAT32, FLOAT64, FLOAT80};
		enum class TypeDim : uint8_t { UNDEF = 0, SINGLE=1, VEC2=2,VEC3=3,VEC4=4,
			MAT1X2, MAT1X3, MAT1X4,
			MAT2X2, MAT2X3, MAT2X4,
			MAT3X2, MAT3X3, MAT3X4,
			MAT4X2, MAT4X3, MAT4X4
		};

		enum class TypeImageDim : uint8_t {UNDEF = 0, I1D=1,I2D, I3D, CUBE, RECT, BUFFER, SUBPASS};

		typedef uint16_t RawDataHandle;
		typedef uint16_t SDataHandle;
		typedef uint32_t SPIRVHandle;

		static const SPIRVHandle SPIRVNull = std::numeric_limits<SPIRVHandle>::max();
		static const RawDataHandle RawDataNull = std::numeric_limits<RawDataHandle>::max();
		static const SDataHandle SDataNull = std::numeric_limits<SDataHandle>::max();

		class Module;
		class LayoutInfo;

		struct LayoutBindingData
		{
			const char * pName;
			uint16_t set, binding;			
			uint16_t arraySize;
			DescriptorBindingType bindingType;
			bool isArray;			
		};

		class TypeInfo
		{
			friend class LayoutBindingInfo;
			const Module * module_;
			SDataHandle typeHandle_;
			RawDataHandle memberBegin_, memberEnd_;

			TypeInfo(const Module * module, SDataHandle typeHandle);
		public:
			TypeInfo(const TypeInfo &li) : module_(li.module_), typeHandle_(li.typeHandle_), memberBegin_(li.memberBegin_), memberEnd_(li.memberEnd_) {}
			TypeInfo(TypeInfo &&li) : module_(li.module_), typeHandle_(li.typeHandle_), memberBegin_(li.memberBegin_), memberEnd_(li.memberEnd_) {}

			TypeInfo &operator =(const TypeInfo &li)
			{
				module_ = li.module_;
				typeHandle_ = li.typeHandle_;
				memberBegin_ = li.memberBegin_;
				memberEnd_ = li.memberEnd_;
				return *this;
			}
			TypeInfo &operator =(TypeInfo &&li)
			{
				module_ = li.module_;
				typeHandle_ = li.typeHandle_;
				memberBegin_ = li.memberBegin_;
				memberEnd_ = li.memberEnd_;
				return *this;
			}			
			inline uint16_t numMembers() const { return (memberEnd_ - memberBegin_) / sizeof(SPIRVHandle); }
			const char * getMemberName(uint16_t idx) const;
			TypeInfo getMemberTypeInfo(uint16_t idx) const;

			const char * getTypeName() const;
		};

		class LayoutBindingInfo
		{
			friend class LayoutSetInfo;
			const Module * module_;
			SDataHandle userDscBindingHandle_;
			LayoutBindingData bindingData_;

			LayoutBindingInfo(const Module * module, SDataHandle bindingHandle);
		public:
			LayoutBindingInfo(const LayoutBindingInfo &li) : module_(li.module_), userDscBindingHandle_(li.userDscBindingHandle_), bindingData_(li.bindingData_){}
			LayoutBindingInfo(LayoutBindingInfo &&li) : module_(li.module_), userDscBindingHandle_(li.userDscBindingHandle_), bindingData_(li.bindingData_){}

			LayoutBindingInfo &operator =(const LayoutBindingInfo &li)
			{
				module_ = li.module_;
				userDscBindingHandle_ = li.userDscBindingHandle_;
				bindingData_ = li.bindingData_;
				return *this;
			}
			LayoutBindingInfo &operator =(LayoutBindingInfo &&li)
			{
				module_ = li.module_;
				userDscBindingHandle_ = li.userDscBindingHandle_;
				bindingData_ = li.bindingData_;
				return *this;
			}
			inline const LayoutBindingData &raw() const { return bindingData_; }
			inline uint16_t getSetSlot() const { return bindingData_.set; }
			inline uint16_t getBindingSlot() const { return bindingData_.binding; }
			inline uint16_t getNumBindingSlots() const { return bindingData_.arraySize; }
			inline uint16_t getArraySize() const { return bindingData_.arraySize; }
			inline DescriptorBindingType getType() const { return bindingData_.bindingType; }
			inline bool isArray() const { return bindingData_.isArray; }
			TypeInfo getTypeInfo() const;
		};

		class LayoutSetInfo
		{
			friend class LayoutInfo;
			const Module * module_;
			SDataHandle userDscSetHandle_;
			uint16_t maxBinding_;
			uint16_t setSlot_;
			SDataHandle userDscBindingInfoBegin_ = SDataNull, userDscBindingInfoEnd_ = SDataNull;
			LayoutSetInfo(const Module * mod, uint16_t setSlot, uint16_t maxBinding, SDataHandle userDscSetHandle, SDataHandle userDscBindingInfoBegin, SDataHandle userDscBindingInfoEnd) : module_(mod), maxBinding_(maxBinding),
				userDscSetHandle_(userDscSetHandle), userDscBindingInfoBegin_(userDscBindingInfoBegin), userDscBindingInfoEnd_(userDscBindingInfoEnd), setSlot_(setSlot)
			{								 
			}
		public:
			LayoutSetInfo(const LayoutSetInfo &li) : module_(li.module_), maxBinding_(li.maxBinding_), userDscBindingInfoBegin_(li.userDscBindingInfoBegin_), userDscBindingInfoEnd_(li.userDscBindingInfoEnd_), userDscSetHandle_(li.userDscSetHandle_), setSlot_(li.setSlot_){}
			LayoutSetInfo(LayoutSetInfo &&li) : module_(li.module_), maxBinding_(li.maxBinding_), userDscBindingInfoBegin_(li.userDscBindingInfoBegin_), userDscBindingInfoEnd_(li.userDscBindingInfoEnd_), userDscSetHandle_(li.userDscSetHandle_), setSlot_(li.setSlot_){}

			LayoutSetInfo &operator =(const LayoutSetInfo &li)
			{
				module_ = li.module_;
				maxBinding_ = li.maxBinding_;
				userDscBindingInfoBegin_ = li.userDscBindingInfoBegin_;
				userDscBindingInfoEnd_ = li.userDscBindingInfoEnd_;
				userDscSetHandle_ = li.userDscSetHandle_;
				setSlot_ = li.setSlot_;
				return *this;
			}
			LayoutSetInfo &operator =(LayoutSetInfo &&li)
			{
				module_ = li.module_;
				maxBinding_ = li.maxBinding_;
				userDscBindingInfoBegin_ = li.userDscBindingInfoBegin_;
				userDscBindingInfoEnd_ = li.userDscBindingInfoEnd_;
				userDscSetHandle_ = li.userDscSetHandle_;
				setSlot_ = li.setSlot_;
				return *this;
			}
			inline uint16_t getSetSlot() const { return setSlot_; }
			inline uint16_t getMaxBindingSlot() const { return maxBinding_; }
			inline uint16_t getNumBindingInfo() const { return userDscBindingInfoEnd_ - userDscBindingInfoBegin_; }

			SDataHandle getBindingHandleByIndex(uint16_t bindingIndex)  const;
			SDataHandle getBindingHandleBySlot(uint16_t bindingSlot)  const;

			LayoutBindingInfo getBindingInfoByHandle(SDataHandle bindingHandle) const;
			inline LayoutBindingInfo getBindingInfoByIndex(uint16_t bindingIndex) const { return getBindingInfoByHandle(getBindingHandleByIndex(bindingIndex)); }
			inline LayoutBindingInfo getBindingInfoBySlot(uint16_t bindingSlot) const { return getBindingInfoByHandle(getBindingHandleBySlot(bindingSlot)); }
		};

		class LayoutInfo
		{
			friend class Module;
			const Module * module_;
			uint16_t maxSet_;
			SDataHandle userDscSetInfoBegin_ = SDataNull, userDscSetInfoEnd_ = SDataNull;
			LayoutInfo(const Module * mod, uint16_t maxSet, SDataHandle userDscSetInfoBegin, SDataHandle userDscSetInfoEnd) : module_(mod), maxSet_(maxSet),
				userDscSetInfoBegin_(userDscSetInfoBegin), userDscSetInfoEnd_(userDscSetInfoEnd)
			{
			}
		public:
			LayoutInfo(const LayoutInfo &li) : module_(li.module_), maxSet_(li.maxSet_), userDscSetInfoBegin_(li.userDscSetInfoBegin_), userDscSetInfoEnd_(li.userDscSetInfoEnd_) {}
			LayoutInfo(LayoutInfo &&li) : module_(li.module_), maxSet_(li.maxSet_), userDscSetInfoBegin_(li.userDscSetInfoBegin_), userDscSetInfoEnd_(li.userDscSetInfoEnd_) {}

			LayoutInfo &operator =(const LayoutInfo &li)
			{
				module_ = li.module_;
				maxSet_ = li.maxSet_;
				userDscSetInfoBegin_ = li.userDscSetInfoBegin_;
				userDscSetInfoEnd_ = li.userDscSetInfoEnd_;
				return *this;
			}
			LayoutInfo &operator =(LayoutInfo &&li)
			{
				module_ = li.module_;
				maxSet_ = li.maxSet_;
				userDscSetInfoBegin_ = li.userDscSetInfoBegin_;
				userDscSetInfoEnd_ = li.userDscSetInfoEnd_;
				return *this;
			}
			inline uint16_t getMaxSetSlot() const {return maxSet_;};
			inline uint16_t getNumSetInfo()const { return userDscSetInfoEnd_ - userDscSetInfoBegin_; }

			SDataHandle getSetHandleByIndex(uint16_t setIndex)  const;
			SDataHandle getSetHandleBySlot(uint16_t setSlot)  const;

			LayoutSetInfo getSetInfoByHandle(SDataHandle setHandle) const;
			inline LayoutSetInfo getSetInfoByIndex(uint16_t setIndex) const { return getSetInfoByHandle(getSetHandleByIndex(setIndex)); }
			inline LayoutSetInfo getSetInfoBySlot(uint16_t setSlot) const { return getSetInfoByHandle(getSetHandleBySlot(setSlot)); }
		};

		class Module
		{
			friend class LayoutInfo;
			friend class LayoutSetInfo;
			friend class LayoutBindingInfo;
			friend class TypeInfo;

			static const size_t RawDataBuffSize = 4096; /*4KB of raw data (dynamic arrays & names)*/
			static const size_t StructuredDataBuffSize = 1024; /*1024 structured data entries.*/
			static const size_t StructuredDataBuffBlockBits = 4; /*8KB of structured data: descriptors, types, etc.*/


			struct TypeSimple
			{
				TypeBase base = TypeBase::UNDEF;
				TypeDim dim = TypeDim::UNDEF;
			};
			struct TypeImage
			{
				SDataHandle baseTypeHandle;
				TypeImageDim imageDim;
				uint8_t isDepth : 2;
				uint8_t isArray : 1;
				uint8_t isMS : 1;
				uint8_t isSampled : 2;
				uint8_t accessQual : 2;
				uint8_t imageFormat;
			};
			struct TypeReference
			{				
				union {					
					uint32_t storageType;
					SDataHandle sizeHandle;
				};
				SDataHandle refTypeHandle;
			};
			struct TypeCombinantion
			{
				RawDataHandle beginHandle, endHandle;
			};

			enum class InfoType : uint8_t { UNDEF = 0, TYPE_INFO, VAR_INFO, CONST_INFO, DESCRIPTOR_INFO, NAME_INFO, BLOCK_INFO, USER_DSC_BINDING_INFO, USER_DSC_SET_INFO };
			enum class ConstantType : uint8_t { UNDEF = 0, BOOL_TRUE, BOOL_FALSE, UINT32_VAL, OTHER };

			struct TypeInfo
			{
				union
				{
					TypeSimple simple;
					TypeImage image;
					TypeReference reference;
					TypeCombinantion combination;
				};
				TypeMETA metaType;				
			};
			struct VariableInfo
			{
				SDataHandle typeHandle;
				uint32_t storageType;
			};
			struct ConstantInfo
			{
				uint32_t value0;
				SDataHandle typeHandle;
				ConstantType type;
			};
			struct DescriptorInfo
			{
				uint16_t dscSet, dscBind;
				//uint16_t idx;
			};
			struct BlockInfo
			{
				bool isSSBOlike;
				SDataHandle baseType;
			};
			struct NameInfo
			{
				RawDataHandle name;
				uint16_t idx;								
			};		

			struct UserDscBindingInfo
			{
				RawDataHandle fkName;
				SDataHandle fkBaseType;
				SDataHandle fkDescriptor;
				union {
					SDataHandle fkBlock;/*for UBO & SSBO*/
					SDataHandle fkSampler;/*for Image-based*/
				};
				uint16_t isArrayed : 1;
				uint16_t arraySize : 15;//0 means dynamic
				DescriptorBindingType bindingType;
			};
			struct UserDscSetInfo
			{
				uint16_t setIndex;
				SDataHandle firstBindingInfo;
				uint16_t numBindingInfo;
				uint16_t maxBindingSlot;
			};

			struct StructuredInfo
			{
				StructuredInfo() {}
				SPIRVHandle spirvID;
				InfoType type;
				union
				{
					TypeInfo typeInfo;
					VariableInfo varInfo;
					ConstantInfo constInfo;
					DescriptorInfo descInfo;
					BlockInfo blockInfo;
					NameInfo nameInfo;
					UserDscBindingInfo userBindingInfo;
					UserDscSetInfo userSetInfo;
				};				
			};

			static const size_t SizeOfTypeInfo = sizeof(TypeInfo);
			static const size_t SizeOfVariableInfo = sizeof(VariableInfo);
			static const size_t SizeOfConstantInfo = sizeof(ConstantInfo);
			static const size_t SizeOfDescriptorInfo = sizeof(DescriptorInfo);
			static const size_t SizeOfNameInfo = sizeof(NameInfo);
			static const size_t SizeOfUserBindingInfo = sizeof(UserDscBindingInfo);
			static const size_t SizeOfStructuredInfo = sizeof(StructuredInfo);

			uint8_t rawDataBuff_[RawDataBuffSize];
			util::memory::MemAllocVirtual_LinearStatic<util::memory::MemAllocVirtual_BASE, RawDataBuffSize> rawAlloc_;

			StructuredInfo sDataBuff_[StructuredDataBuffSize];
			util::memory::MemAllocVirtual_LinearStatic<util::memory::MemAllocVirtual_BASE, (StructuredDataBuffSize >> StructuredDataBuffBlockBits)> sAlloc_; /*linear alloc in blocks of 16 bytes*/

			SDataHandle userDscBindingInfoBegin_ = SDataNull, userDscBindingInfoEnd_ = SDataNull;
			SDataHandle userDscSetInfoBegin_ = SDataNull, userDscSetInfoEnd_ = SDataNull;
			RawDataHandle descriptorLayoutHandle_ = RawDataNull;
			
			inline std::pair<RawDataHandle, RawDataHandle> newSubTypes_(uint32_t num)
			{
				size_t reqSize = num * sizeof(SPIRVHandle);
				if(reqSize == 0)
					return std::make_pair<RawDataHandle, RawDataHandle>(rawAlloc_.top(), rawAlloc_.top());
				auto h1 = rawAlloc_.top();
				auto res = rawAlloc_.allocate(reqSize);
				if(res.size < reqSize)
					throw SPIRVResourceException("Module::newSubTypes_: could not allocate new buffer -- out of memory for raw-data.");
				auto h2 = h1 + reqSize;
				return std::make_pair(h1, h2);
			}
			inline RawDataHandle newName_(uint32_t len)
			{
				size_t reqSize = (len+1) * sizeof(char);
				auto h = rawAlloc_.top();
				auto res = rawAlloc_.allocate(reqSize);
				if (res.size < reqSize)
					throw SPIRVResourceException("Module::newName_: could not allocate new buffer -- out of memory for raw-data.");
				return h;
			}
			template<class T>
			inline RawDataHandle newRawT_(uint32_t num = 1)
			{
				size_t reqSize = num * sizeof(T);
				if (reqSize == 0)
					return rawAlloc_.top();
				auto h1 = rawAlloc_.top();
				auto res = rawAlloc_.allocate(reqSize);
				if (res.size < reqSize)
					throw SPIRVResourceException("Module::newRawT_: could not allocate new buffer -- out of memory for raw-data.");
				return h1;
			}
			inline SDataHandle newSInfo_()
			{
				auto res = sAlloc_.allocate(1);
				if (res.size == 0)
					throw SPIRVResourceException("Module::newSInfo_: could not allocate new buffer -- out of memory for structured-data.");
				return reinterpret_cast<SDataHandle>(res.ptr);
			}
			inline StructuredInfo &getSInfo_(SDataHandle h)
			{
				if(h >= sAlloc_.top())
					throw SPIRVLogicException("Module::getSInfo_: invalid handle.");
				return sDataBuff_[h];
			}
			inline const StructuredInfo &getType_(SDataHandle h) const
			{
				if (h >= sAlloc_.top())
					throw SPIRVLogicException("Module::getSInfo_: invalid handle.");
				return sDataBuff_[h];
			}
			inline SDataHandle getHandleByID_(SPIRVHandle spirvID) const
			{
				for (SDataHandle h = 0; h < sAlloc_.top(); h++)
					if (sDataBuff_[h].spirvID == spirvID)
						return h;
				throw SPIRVLogicException("Module::getHandleByID_: could not find type by SPIRV-ID.");
			}
			inline SDataHandle getHandleByID_(SPIRVHandle spirvID, InfoType type) const
			{
				for (SDataHandle h = 0; h < sAlloc_.top(); h++)
					if (sDataBuff_[h].spirvID == spirvID && sDataBuff_[h].type == type)
						return h;
				throw SPIRVLogicException("Module::getHandleByID_: could not find type by SPIRV-ID.");
			}
			void handleDecorateDescriptor_(uint16_t opCode, uint32_t trgt, uint32_t idx, uint32_t arg, bool dscSet);
			void handleDecorateBlock_(uint16_t opCode, uint32_t trgt, uint32_t idx, uint32_t arg, bool isSSBOlike);
			void handleVariable_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs);
			void handleConstant_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs);
			void handleType_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs);
			void handleDecorate_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs);
			void handleName_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs);
			void handleOpCode_(uint16_t opLen, uint16_t opCode, uint32_t * opArgs);

			void deduceBindingType_(SDataHandle userBindingHandle);
			void sortBindings_(SDataHandle begin, SDataHandle end);
			void checkBindings_(SDataHandle begin, SDataHandle end);
			void createDescriptorSet_(SDataHandle begin, SDataHandle end);
			void createDescriptorLayout_(SDataHandle begin, SDataHandle end);
		public:
			void compileBindingInfo();
			LayoutInfo getBindingLayoutInfo() const;

			
			template<class Read, class EoF>
			Module(Read &&r, EoF &&eof)
			{
				static const size_t OpArgBuffSize = 256;
				
				uint32_t magicWord, versionWord, genMagicNumberWord, idBoundWord, zeroWord, opArgBuff[OpArgBuffSize];
				r(reinterpret_cast<char *>(&magicWord), 4);
				r(reinterpret_cast<char *>(&versionWord), 4);
				r(reinterpret_cast<char *>(&genMagicNumberWord), 4);
				r(reinterpret_cast<char *>(&idBoundWord), 4);
				r(reinterpret_cast<char *>(&zeroWord), 4);

				while (!eof())
				{
					uint32_t instr;
					r(reinterpret_cast<char *>(&instr), 4);

					uint16_t opLen = instr >> 16;
					uint16_t opCode = instr & 0xFFFF;
					/*handle opLen > OpArgBuffSize*/

					r(reinterpret_cast<char*>(opArgBuff), 4 * (opLen - 1));

					
					handleOpCode_(opLen, opCode, opArgBuff);
				}
			}
		};
	}
}