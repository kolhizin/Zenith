#pragma once

#include "ioconv\io_config.h"

#define ZENITH_MAKE_VERSION(major, minor, patch) \
    (((major) << 22) | ((minor) << 12) | (patch))


#define ZENITH_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define ZENITH_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define ZENITH_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

namespace zenith
{
	struct version32_t
	{
		version32_t(uint32_t major, uint32_t minor, uint32_t patch = 0)
			: major(major), minor(minor), patch(patch) {}
		version32_t(uint32_t version = 0)
			: version(version) {}
		union
		{
			struct
			{
				uint32_t patch : 12;
				uint32_t minor : 10;
				uint32_t major : 10;
			};
			uint32_t version;			
		};
		//implicit conversion to uint32_t
		operator uint32_t() const { return version; }
	};

	template<class It, zenith::util::ioconv::InternalType intType>
	class zenith::util::ioconv::io_handler_impl<zenith::version32_t, It, intType>
	{
	public:
		inline static void input(zenith::version32_t &val, const It &it)
		{
			uint32_t tmp;
			zenith::util::ioconv::input_named_required(tmp, it, "major"); val.major = tmp;
			zenith::util::ioconv::input_named_optional(tmp, it, "minor", uint32_t(0)); val.minor = tmp;
			zenith::util::ioconv::input_named_optional(tmp, it, "patch", uint32_t(0)); val.patch = tmp;
		}
		inline static void output(const zenith::version32_t &val, It &it)
		{
			zenith::util::ioconv::output_single(val.major, it.append_value("major", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			zenith::util::ioconv::output_single(val.minor, it.append_value("minor", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
			zenith::util::ioconv::output_single(val.patch, it.append_value("patch", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
		}
	};
}