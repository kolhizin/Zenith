#pragma once

#include "ggraphics_base.h"

namespace zenith
{
	namespace gengraphics
	{
		class GenGraphicsPipelineException : public GenGraphicsException
		{
		public:
			GenGraphicsPipelineException() : GenGraphicsException() {}
			GenGraphicsPipelineException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : GenGraphicsException(p, type)
			{
			}
			GenGraphicsPipelineException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : GenGraphicsException(str, type)
			{
			}
			virtual ~GenGraphicsPipelineException() {}
		};
	}
}