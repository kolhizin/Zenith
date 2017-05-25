#pragma once
#include "NodeGenerator.h"
#include <Utils\nameid.h>
#include <Utils\object_map.h>
#include <vector>


namespace zenith
{
	namespace terragen
	{
		//typedef util::nameid (*PFN_TerraGenMetaGenerator)(void * ptr, const GeneratorArguments * arg);
		//typedef void(*PFN_TerraGenMetaSetSeed)(void * ptr, uint32_t seed);

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
		class TerraGenFactoryUnkGenException : public TerraGenFactoryException
		{
		public:
			TerraGenFactoryUnkGenException() : TerraGenFactoryException() {}
			TerraGenFactoryUnkGenException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenFactoryException(p, type)
			{
			}
			TerraGenFactoryUnkGenException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenFactoryException(str, type)
			{
			}
			virtual ~TerraGenFactoryUnkGenException() {}
		};

		template<class Gen, class GenParam>
		inline Gen * constructGenerator(const GenParam &gp)
		{
			return new Gen(gp);
		}
		template<class Gen, class GenParam>
		inline size_t constructGenerator(const GenParam &gp, void * buffPtr, size_t buffSize)
		{
			if (sizeof(Gen) > buffSize)
				return 0;
			Gen * res = new (buffPtr) Gen(gp);
			return sizeof(Gen);
		}

		template<class Gen, class GenParam>
		inline Gen * constructMetaGenerator(const GenParam &gp)
		{
			return new Gen(gp);
		}
		template<class Gen, class GenParam>
		inline size_t constructMetaGenerator(const GenParam &gp, void * buffPtr, size_t buffSize)
		{
			if (sizeof(Gen) > buffSize)
				return 0;
			Gen * res = new (buffPtr) Gen(gp);
			return sizeof(Gen);
		}

		BaseNodeGenerator * constructGenerator(const zenith::util::ObjectMap<char, char> &descr);
		size_t constructGenerator(const zenith::util::ObjectMap<char, char> &descr, void * buffPtr, size_t buffSize);

		BaseMetaGenerator * constructMetaGenerator(const zenith::util::ObjectMap<char, char> &descr);
		size_t constructMetaGenerator(const zenith::util::ObjectMap<char, char> &descr, void * buffPtr, size_t buffSize);


		template<size_t BuffSize, size_t Align = 16>
		class NodeGeneratorFactory
		{
			util::nameid_map<BaseNodeGenerator *> generators_;
			util::nameid_map<BaseMetaGenerator *> metaGenerators_;
					
			uint8_t buffer_[BuffSize];
			size_t bufferTop_ = 0;
			inline void freeGens_()
			{
				for (const auto &x : generators_.names())
					generators_.at(x)->~BaseNodeGenerator();
				for (const auto &x : metaGenerators_.names())
					metaGenerators_.at(x)->~BaseMetaGenerator();
				generators_.clear();
				metaGenerators_.clear();
			}
			inline size_t align_(size_t sz) const
			{
				size_t rem = sz % Align;
				if (rem == 0)
					return sz;
				return sz + (Align - rem);
			}
			inline void assertNoDup_(const zenith::util::nameid &uid) const
			{
				if (generators_.exist(uid) || metaGenerators_.exist(uid))
					throw TerraGenFactoryException("NodeGeneratorFactory: specified generator already exists!");
			}
		public:
			static const size_t BufferSize = BuffSize;
			~NodeGeneratorFactory()
			{
				freeGens_();
			}
			template<class Gen, class GenParam>
			inline void constructNodeGenerator(const util::nameid &name, const GenParam &gp)
			{
				assertNoDup_(name);
				size_t usedSpace = constructGenerator<Gen, GenParam>(gp, static_cast<void *>(buffer_ + bufferTop_), BufferSize - bufferTop_);
				if(usedSpace == 0)
					throw TerraGenFactoryOutOfMemException();
				generators_.insert(name, reinterpret_cast<BaseNodeGenerator *>(buffer_ + bufferTop_));
				bufferTop_ += align_(usedSpace);
			}
			inline void constructNodeGenerator(const util::nameid &name, const zenith::util::ObjectMap<char, char> &descr)
			{
				assertNoDup_(name);
				size_t usedSpace = constructGenerator(descr, static_cast<void *>(buffer_ + bufferTop_), BufferSize - bufferTop_);
				if (usedSpace == 0)
					throw TerraGenFactoryOutOfMemException();
				generators_.insert(name, reinterpret_cast<BaseNodeGenerator *>(buffer_ + bufferTop_));
				bufferTop_ += align_(usedSpace);
			}
			template<class Gen, class GenParam>
			inline void constructMetaGenerator(const util::nameid &name, const GenParam &gp)
			{
				assertNoDup_(name);
				size_t usedSpace = terragen::constructMetaGenerator<Gen, GenParam>(gp, static_cast<void *>(buffer_ + bufferTop_), BufferSize - bufferTop_);
				if (usedSpace == 0)
					throw TerraGenFactoryOutOfMemException();
				metaGenerators_.insert(name, reinterpret_cast<BaseMetaGenerator *>(buffer_ + bufferTop_));
				bufferTop_ += align_(usedSpace);
			}
			inline void constructMetaGenerator(const util::nameid &name, const zenith::util::ObjectMap<char, char> &descr)
			{
				assertNoDup_(name);
				size_t usedSpace = terragen::constructMetaGenerator(descr, static_cast<void *>(buffer_ + bufferTop_), BufferSize - bufferTop_);
				if (usedSpace == 0)
					throw TerraGenFactoryOutOfMemException();
				metaGenerators_.insert(name, reinterpret_cast<BaseMetaGenerator *>(buffer_ + bufferTop_));
				bufferTop_ += align_(usedSpace);
			}


			inline util::nameid construct(const zenith::util::ObjectMap<char, char> &descr)
			{
				util::nameid name = descr.getValues("uid", util::ObjectMapPresence::ONE).first->second.first.c_str();
				std::string isMetaStr = descr.getValues("isMeta", util::ObjectMapPresence::ONE).first->second.first.c_str();
				std::transform(isMetaStr.begin(), isMetaStr.end(), isMetaStr.begin(), tolower);
				bool isMeta = (isMetaStr == "true");
				if (isMeta)
					constructMetaGenerator(name, descr);
				else
					constructNodeGenerator(name, descr);
				return name;
			}

			inline bool existNodeGenerator(const util::nameid &uid) { return generators_.exist(uid); }
			inline BaseNodeGenerator * getNodeGenerator(const util::nameid &uid) { return generators_.at(uid); }
			inline const BaseNodeGenerator * getNodeGenerator(const util::nameid &uid) const { return generators_.at(uid); }

			inline bool existMetaGenerator(const util::nameid &uid) { return metaGenerators_.exist(uid); }
			inline BaseMetaGenerator * getMetaGenerator(const util::nameid &uid) { return metaGenerators_.at(uid); }
			inline const BaseMetaGenerator * getMetaGenerator(const util::nameid &uid) const { return metaGenerators_.at(uid); }

		};

		template<size_t BuffSize, size_t Align=16>
		class NodeFactory
		{
			struct NodeMeta
			{
				uint32_t level = 0;
				NodeMeta(uint32_t level = 0) : level(level) {}
			};
			util::nameid_map<BaseNodeGenerator *> generators_;
			util::nameid_map<BaseMetaGenerator *> metaGenerators_;

			BaseMetaGenerator * mainMetaGenerator_ = nullptr;

			std::vector<const BaseNode *> nodes_;
			std::vector<NodeMeta> nodeMeta_;
			uint8_t buffer_[BuffSize];
			size_t bufferTop_ = 0;
			/*
			void * metaGeneratorUserData_ = nullptr;
			PFN_TerraGenMetaGenerator metaGenerator_ = nullptr;
			PFN_TerraGenMetaSetSeed metaSetSeed_ = nullptr;
			*/
			inline void freeLastNNodes_(uint32_t numNodes)
			{
				for (uint32_t i = 0; i < numNodes; i++)
				{
					nodes_.back()->~BaseNode();
					nodes_.pop_back();
					nodeMeta_.pop_back();
				}
			}
			inline size_t align_(size_t sz) const
			{
				size_t rem = sz % Align;
				if (rem == 0)
					return sz;
				return sz + (Align - rem);
			}
			inline BaseNodeGenerator * selectGenerator_(const GeneratorArguments &arg)
			{
				if (!mainMetaGenerator_)
					throw TerraGenFactoryException("selectGenerator_(): main meta-generator is not specified!");

				util::nameid name = mainMetaGenerator_->select(&arg);
				while (metaGenerators_.exist(name))
					name = metaGenerators_.at(name)->select(&arg);

				if (!generators_.exist(name))
					return nullptr;
				return generators_.at(name);
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

				BaseNodeGenerator * generator = selectGenerator_(arg);
				if (!generator)
				{
					if (nodeNumber)
						*nodeNumber = 0;
					return;
				}

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
					currentTop += align_(usedSpace);
				}
				bufferTop_ = currentTop;
				if (nodeNumber)
					*nodeNumber = numNewNodes;
				if (nodeIndex)
					*nodeIndex = static_cast<uint32_t>(lastIndex);
			}
			inline void assertNameDup_(const util::nameid &name)
			{
				if(metaGenerators_.exist(name) || generators_.exist(name))
					throw TerraGenFactoryException("NodeFactory: duplicate generator name!");
			}
		public:
			static const size_t BufferSize = BuffSize;
			~NodeFactory()
			{
				for (uint32_t i = 0; i < nodes_.size(); i++)
					nodes_[i]->~BaseNode();
				nodes_.clear();
			}
			inline void registerGenerator(const util::nameid &name, BaseNodeGenerator * generator)
			{
				assertNameDup_(name);
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
			inline void registerMetaGenerator(const util::nameid &name, BaseMetaGenerator * generator)
			{
				assertNameDup_(name);
				if (!generator)
					throw TerraGenFactoryException("registerMetaGenerator(): generator specified is nullptr!");
				metaGenerators_.insert(name, generator);
			}
			inline void deregisterMetaGenerator(const util::nameid &name)
			{
				if (!metaGenerators_.exist(name))
					throw TerraGenFactoryException("deregisterMetaGenerator(): specified generator does not exist!");
				if (metaGenerators_.at(name) == mainMetaGenerator_)
					mainMetaGenerator_ = nullptr;
				metaGenerators_.remove(name);
			}
			inline bool generatorExists(const util::nameid &name) const
			{
				return generators_.exist(name);
			}
			inline bool metaGeneratorExists(const util::nameid &name) const
			{
				return metaGenerators_.exist(name);
			}

			inline void setMainMetaGenerator(const util::nameid &name)
			{
				if (!metaGenerators_.exist(name))
					throw TerraGenFactoryException("setMainMetaGenerator(): specified generator does not exist!");
				mainMetaGenerator_ = metaGenerators_.at(name);
			}
			inline void setSeed(uint32_t seedNumber)
			{
				auto gNames = generators_.names();
				for (auto &name : gNames)
					generators_.at(name)->setSeed(seedNumber);
				auto mgNames = metaGenerators_.names();
				for (auto &name : mgNames)
					metaGenerators_.at(name)->setSeed(seedNumber);
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

		template<size_t NodeBuffSize, size_t GenBuffSize, size_t Align = 16>
		class TerraFactory
		{
			NodeFactory<NodeBuffSize, Align> nodeFactory_;
			NodeGeneratorFactory<GenBuffSize, Align> generatorFactory_;
		public:
			static const size_t NodeBufferSize = NodeBuffSize;
			static const size_t GeneratorBufferSize = GenBuffSize;
			static const size_t AlignSize = Align;

			template<class Gen, class GenParam>
			inline void constructNodeGenerator(const util::nameid &name, const GenParam &gp)
			{
				generatorFactory_.constructNodeGenerator<Gen, GenParam>(name, gp);
				nodeFactory_.registerGenerator(name, generatorFactory_.getNodeGenerator(name));
			}
			template<class Gen, class GenParam>
			inline void constructMetaGenerator(const util::nameid &name, const GenParam &gp)
			{
				generatorFactory_.constructMetaGenerator<Gen, GenParam>(name, gp);
				nodeFactory_.registerMetaGenerator(name, generatorFactory_.getMetaGenerator(name));
			}

			inline util::nameid constructGenerator(const zenith::util::ObjectMap<char, char> &descr)
			{
				util::nameid name = generatorFactory_.construct(descr);
				if (generatorFactory_.existMetaGenerator(name))
					nodeFactory_.registerMetaGenerator(name, generatorFactory_.getMetaGenerator(name));
				else
					nodeFactory_.registerGenerator(name, generatorFactory_.getNodeGenerator(name));
				return name;
			}
			inline void setMainMetaGenerator(const util::nameid &name)
			{
				nodeFactory_.setMainMetaGenerator(name);
			}
			inline void setSeed(uint32_t seedNumber)
			{
				nodeFactory_.setSeed(seedNumber);
			}
			inline uint32_t generateNewWave(uint32_t seedNumber = 0)
			{
				return nodeFactory_.generateNewWave(seedNumber);
			}
			inline size_t numNodes() const { return nodeFactory_.numNodes(); }
			inline const BaseNode * getNode(size_t i) const { return nodeFactory_.getNode(i); }
		};
	}
}
