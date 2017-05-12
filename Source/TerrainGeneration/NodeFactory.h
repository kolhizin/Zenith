#pragma once
#include "NodeGenerator.h"
#include <Utils\nameid.h>
#include <vector>

namespace zenith
{
	namespace terragen
	{
		typedef util::nameid (*PFN_TerraGenMetaGenerator)(void * ptr, const GeneratorArguments * arg);

		class TerraGenFactoryException : public TerraGenException
		{
		public:
			TerraGenFactoryException() : TerraGenException() {}
			TerraGenFactoryException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenException(p, type)
			{
			}
			TerraGenFactoryException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenException(str, type)
			{
			}
			virtual ~TerraGenFactoryException() {}
		};
		class TerraGenFactoryOutOfMemException : public TerraGenFactoryException
		{
		public:
			TerraGenFactoryOutOfMemException() : TerraGenFactoryException() {}
			TerraGenFactoryOutOfMemException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenFactoryException(p, type)
			{
			}
			TerraGenFactoryOutOfMemException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenFactoryException(str, type)
			{
			}
			virtual ~TerraGenFactoryOutOfMemException() {}
		};

		template<size_t BuffSize>
		class NodeFactory
		{
			util::nameid_map<const BaseNodeGenerator *> generators_;
			std::vector<const BaseNode *> nodes_;
			uint8_t buffer_[BuffSize];
			size_t bufferTop_ = 0;

			void * metaGeneratorUserData_ = nullptr;
			PFN_TerraGenMetaGenerator metaGenerator_ = nullptr;

			inline void freeLastNNodes_(uint32_t numNodes)
			{
				for (uint32_t i = 0; i < numNodes; i++)
				{
					delete nodes_.back();
					nodes_.pop_back();
				}
			}

			inline void generateNodes_(BaseNode * p, uint32_t seedNumber, uint32_t * nodeNumber = nullptr, uint32_t * nodeIndex = nullptr)
			{
				GeneratorArguments arg;
				arg.previousNode = p;
				arg.allNodesNum = nodes_.size();
				if (arg.allNodesNum > 0)
					arg.allNodesPtr = &nodes_[0];
				else
					arg.allNodesPtr = nullptr;

				arg.seedNumber = seedNumber;

				if(!metaGenerator_)
					throw TerraGenFactoryException("generateNodes_(): meta-generator is not specified!");

				auto metaGen = metaGenerator_(metaGeneratorUserData_, arg);
				if (!generators_.exist(metaGen))
				{
					if (nodeNumber)
						*nodeNumber = 0;
				}
				BaseNodeGenerator * generator = generators_[metaGen];

				uint32_t numNewNodes = generator->generate(&arg);
				if (numNewNodes == 0)
				{
					if (nodeNumber)
						*nodeNumber = 0;
					return;
				}
				uint32_t lastIndex = nodes_.size();
				size_t currentTop = bufferTop_;
				for (uint32_t i = 0; i < numNewNodes; i++)
				{
					size_t usedSpace = 0;
					try {
						usedSpace = generator->get(i, buffer_ + currentTop, BufferSize - currentTop);
					}
					catch (...)
					{
						freeLastNNodes_(i);
						throw;
					}
					if (usedSpace == 0)
					{
						freeLastNNodes_(i);
						throw TerraGenFactoryOutOfMemException();
					}
					nodes_.push_back(reinterpret_cast<const BaseNode *>(buffer_ + currentTop));
					currentTop += usedSpace;
				}
				bufferTop_ = currentTop;
			}
		public:
			static const size_t BufferSize = BuffSize;

			inline void registerGenerator(const util::nameid &name, const BaseNodeGenerator * generator)
			{
				if (generators_.exist(name))
					throw TerraGenFactoryException("registerGenerator(): specified generator already exists!");
				if(!generator)
					throw TerraGenFactoryException("registerGenerator(): generator specified is nullptr!");
				generators_.insert(name, generator);
			}
			inline void deregisterGenerator(const util::nameid &name)
			{
				if (!generators_.exist(name))
					throw TerraGenFactoryException("deregisterGenerator(): specified generator does not exist!");
				generators_.remove(name);
			}
			inline bool generatorExists(const util::nameid &name) const
			{
				return generators_.exist(name);
			}

			inline void registerMetaGenerator(PFN_TerraGenMetaGenerator metaGenerator, void * userData = nullptr)
			{
				metaGenerator_ = metaGenerator;
				metaGeneratorUserData_ = userData;
			}
		};
	}
}
