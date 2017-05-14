#pragma once
#include "NodeGenerator.h"
#include <Utils\nameid.h>
#include <vector>

namespace zenith
{
	namespace terragen
	{
		typedef util::nameid (*PFN_TerraGenMetaGenerator)(void * ptr, const GeneratorArguments * arg);
		typedef void(*PFN_TerraGenMetaSetSeed)(void * ptr, uint32_t seed);

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
			struct NodeMeta
			{
				uint32_t level = 0;
				NodeMeta(uint32_t level = 0) : level(level) {}
			};
			util::nameid_map<BaseNodeGenerator *> generators_;
			std::vector<const BaseNode *> nodes_;
			std::vector<NodeMeta> nodeMeta_;
			uint8_t buffer_[BuffSize];
			size_t bufferTop_ = 0;

			void * metaGeneratorUserData_ = nullptr;
			PFN_TerraGenMetaGenerator metaGenerator_ = nullptr;
			PFN_TerraGenMetaSetSeed metaSetSeed_ = nullptr;

			inline void freeLastNNodes_(uint32_t numNodes)
			{
				for (uint32_t i = 0; i < numNodes; i++)
				{
					delete nodes_.back();
					nodes_.pop_back();
					nodeMeta_.pop_back();
				}
			}

			inline void generateNodes_(const BaseNode * p, uint32_t seedNumber, uint32_t * nodeNumber = nullptr, uint32_t * nodeIndex = nullptr)
			{
				GeneratorArguments arg;
				arg.previousNode = p;
				arg.allNodesNum = static_cast<uint32_t>(nodes_.size());
				if (arg.allNodesNum > 0)
					arg.allNodesPtr = &nodes_[0];
				else
					arg.allNodesPtr = nullptr;

				arg.seedNumber = seedNumber;

				uint32_t newLevel = 0;
				if (p)
				{
					for(uint32_t i = 0; i < nodes_.size(); i++)
						if (nodes_[i] == p)
						{
							newLevel = nodeMeta_[i].level + 1;
							break;
						}
					if(newLevel == 0)
						throw TerraGenFactoryException("generateNodes_(): unable to find parent node!");
				}

				if(!metaGenerator_)
					throw TerraGenFactoryException("generateNodes_(): meta-generator is not specified!");

				auto metaGen = metaGenerator_(metaGeneratorUserData_, &arg);
				if (!generators_.exist(metaGen))
				{
					if (nodeNumber)
						*nodeNumber = 0;
				}
				BaseNodeGenerator * generator = generators_.at(metaGen);

				uint32_t numNewNodes = generator->generate(&arg);
				if (numNewNodes == 0)
				{
					if (nodeNumber)
						*nodeNumber = 0;
					return;
				}
				size_t lastIndex = nodes_.size();
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
					nodeMeta_.push_back(NodeMeta(newLevel));
					currentTop += usedSpace;
				}
				bufferTop_ = currentTop;
				if (nodeNumber)
					*nodeNumber = numNewNodes;
				if (nodeIndex)
					*nodeIndex = static_cast<uint32_t>(lastIndex);
			}
		public:
			static const size_t BufferSize = BuffSize;

			inline void registerGenerator(const util::nameid &name, BaseNodeGenerator * generator)
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

			inline void registerMetaGenerator(PFN_TerraGenMetaGenerator metaGenerator, PFN_TerraGenMetaSetSeed metaSetSeed, void * userData = nullptr)
			{
				metaGenerator_ = metaGenerator;
				metaSetSeed_ = metaSetSeed;
				metaGeneratorUserData_ = userData;
			}
			inline void setSeed(uint32_t seedNumber)
			{
				metaSetSeed_(metaGeneratorUserData_, seedNumber);
				auto gNames = generators_.names();
				for (auto &name : gNames)
					generators_.at(name)->setSeed(seedNumber);
			}
			inline uint32_t generateNewWave(uint32_t seedNumber = 0) //returns number of generated nodes
			{
				uint32_t waveLevel = 0;
				for (uint32_t i = 0; i < nodes_.size(); i++)
					if (nodeMeta_[i].level > waveLevel)
						waveLevel = nodeMeta_[i].level;
				size_t nodesNum = nodes_.size();
				uint32_t numGenerated = 0;
				if (nodesNum == 0)
					generateNodes_(nullptr, seedNumber, &numGenerated);
				else
				{
					for (uint32_t i = 0; i < nodesNum; i++)
						if (nodeMeta_[i].level == waveLevel)
						{
							uint32_t numGen = 0;
							generateNodes_(nodes_[i], seedNumber, &numGen);
							numGenerated += numGen;
						}
				}
				return numGenerated;
			}

			inline size_t numNodes() const { return nodes_.size(); }
			inline const BaseNode * getNode(size_t i) const { return nodes_.at(i); }
		};
	}
}
