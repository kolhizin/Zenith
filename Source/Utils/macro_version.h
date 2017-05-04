#pragma once
#define ZENITH_MAKE_VERSION(major, minor, patch) \
    (((major) << 22) | ((minor) << 12) | (patch))


#define ZENITH_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define ZENITH_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define ZENITH_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)