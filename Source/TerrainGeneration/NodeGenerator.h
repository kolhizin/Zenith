#pragma once
#include "BaseTerraGen.h"
#include <random>

namespace zenith
{
	namespace terragen
	{	
		class TerraGenGeneratorException : public TerraGenException
		{
		public:
			TerraGenGeneratorException() : TerraGenException() {}
			TerraGenGeneratorException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenException(p, type)
			{
			}
			TerraGenGeneratorException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenException(str, type)
			{
			}
			virtual ~TerraGenGeneratorException() {}
		};

		class BaseNodeGenerator;

		class BaseNode
		{
		private:
			const BaseNode * parentNode_; //does not own
			const BaseNodeGenerator * generator_; //does not own
		public:
			static constexpr char * ClassName = "BaseNode";
			static constexpr char * DistanceDispatchGroup = "BaseNode";

			inline BaseNode(const BaseNode * p, const BaseNodeGenerator * g) : parentNode_(p), generator_(g){}
			inline virtual ~BaseNode() {}

			inline const BaseNode * getParent() const { return parentNode_; }
			inline bool hasParent() const { return (parentNode_ != nullptr); }

			inline virtual const char * nodeClass() const { return ClassName; }
			inline virtual const char * distanceDispatchGroup() const { return DistanceDispatchGroup; }

			inline const BaseNodeGenerator * getGenerator() const { return generator_; }
		};


		class RootNode : public BaseNode
		{
		public:
			static constexpr char * ClassName = "RootNode";
			static constexpr char * DistanceDispatchGroup = "RootNode";

			inline RootNode() : BaseNode(nullptr, nullptr) {}
			inline virtual ~RootNode() {}

			inline virtual const char * nodeClass() const { return ClassName; }
			inline virtual const char * distanceDispatchGroup() const { return DistanceDispatchGroup; }
		};
				
		struct GeneratorArguments
		{
			const BaseNode * previousNode;

			uint32_t allNodesNum;
			const BaseNode ** allNodesPtr;

			uint32_t seedNumber;
		};

		class BaseNodeGenerator
		{
		protected:
			std::mt19937 randomEngine_;
			uint32_t numGenerated_;
			const BaseNode * curParent_;
			inline void setState_(uint32_t numGenerated, const BaseNode * parent)
			{
				numGenerated_ = numGenerated;
				curParent_ = parent;
			}
			inline void checkId_(uint32_t nodeId) const
			{
				if (nodeId >= numGenerated_)
					throw TerraGenGeneratorException("generator-get(): node-id out of bounds!");
			}
			inline void setSeed_(uint32_t seed)
			{
				if(seed)
					randomEngine_.seed(seed);
			}
			template<class T>
			inline size_t checkSize_(size_t buffSize) const
			{
				size_t reqSize = sizeof(T);
				if (reqSize > buffSize)
					return 0;
				return reqSize;
			}
		public:
			inline BaseNodeGenerator() {}
			inline virtual ~BaseNodeGenerator() {}

			virtual uint32_t generate(const GeneratorArguments * arg) = 0;
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const = 0; /*returns used up space*/
			inline virtual void setSeed(uint32_t seed)
			{
				randomEngine_.seed(seed);
			}
		};
	}
}
