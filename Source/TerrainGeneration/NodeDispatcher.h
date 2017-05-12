#pragma once
#include "NodeGenerator.h"
#include "GeometryNodes.h"

namespace zenith
{
	namespace terragen
	{
		class TerraGenDispatcherException : public TerraGenException
		{
		public:
			TerraGenDispatcherException() : TerraGenException() {}
			TerraGenDispatcherException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenException(p, type)
			{
			}
			TerraGenDispatcherException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenException(str, type)
			{
			}
			virtual ~TerraGenDispatcherException() {}
		};

		inline double getNodeDistance(const BaseNode * p1, const BaseNode * p2, double distanceCutoff)
		{
			if (p1->distanceDispatchGroup() == BaseNode::DistanceDispatchGroup || p2->distanceDispatchGroup() == BaseNode::DistanceDispatchGroup)
				throw TerraGenDispatcherException("getNodeDistance(): one of nodes is BaseNode!");
			if (p1->distanceDispatchGroup() == RootNode::DistanceDispatchGroup || p2->distanceDispatchGroup() == RootNode::DistanceDispatchGroup)
				return TerraGenInfty;
			if (p1->distanceDispatchGroup() == PointNode::DistanceDispatchGroup)
			{
				if (p2->distanceDispatchGroup() == PointNode::DistanceDispatchGroup)
					return getNodeDistance(static_cast<const PointNode *>(p1), static_cast<const PointNode *>(p2));
				if (p2->distanceDispatchGroup() == SegmentNode::DistanceDispatchGroup)
					return getNodeDistance(static_cast<const PointNode *>(p1), static_cast<const SegmentNode *>(p2));
			}else
			if (p1->distanceDispatchGroup() == SegmentNode::DistanceDispatchGroup)
			{
				if (p2->distanceDispatchGroup() == PointNode::DistanceDispatchGroup)
					return getNodeDistance(static_cast<const SegmentNode *>(p1), static_cast<const PointNode *>(p2));
				if (p2->distanceDispatchGroup() == SegmentNode::DistanceDispatchGroup)
				{
					double dlow = getNodeFastLowerDistance(static_cast<const SegmentNode *>(p1), static_cast<const SegmentNode *>(p2));
					if (dlow > distanceCutoff)
						return dlow;
					return getNodeDistance(static_cast<const SegmentNode *>(p1), static_cast<const SegmentNode *>(p2));
				}
			}
			throw TerraGenDispatcherException("getNodeDistance(): unknown arguments for getNodeDistance!");
		}
	}
}
