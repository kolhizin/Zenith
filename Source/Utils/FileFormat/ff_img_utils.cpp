#include "ff_img_utils.h"
#define STB_IMAGE_IMPLEMENTATION 1
#include <stb\stb_image.h>

using namespace zenith::util::zfile_format;

zImgDescription zenith::util::zfile_format::zimg_from_png(void * src, uint64_t size)
{
	int w = 0, h = 0, ch = 0;
	stbi_uc * data = stbi_load_from_memory(static_cast<const stbi_uc *>(src), size, &w, &h, &ch, 0);
	if (!data)
		throw ZFileException("zimg_from_png(): failed to load png from memory.");

	zImgDescription r;
	r.width = w;
	r.height = h;
	r.arraySize = 1;
	r.depth = 1;
	r.imageType = ImageType::IMG2D;
	r.mipLevels = 1;
	
	if (ch == 1)
		r.imageFormat = ImageFormat::R8;
	else if (ch == 3)
		r.imageFormat = ImageFormat::R8G8B8;
	else if (ch == 4)
		r.imageFormat = ImageFormat::R8G8B8A8;
	else
	{
		stbi_image_free(data);
		throw ZFileException("zimg_from_png(): unknown image channels/format.");
	}
	
	r.levelDescr[0] = new zImgDataDescription;
	*r.levelDescr[0] = zImgDataDescription::create2D(w, h, uint64_t(w) * getPixelSize(r.imageFormat));
	uint64_t dstSize = r.levelDescr[0]->getRequiredDataSize();
	r.levelData[0] = new uint8_t[dstSize];

	memcpy_s(r.levelData[0], dstSize, data, w*h*ch);

	stbi_image_free(data);
	return r;
}

zImgDescription zenith::util::zfile_format::zimg_from_img(void * src, uint64_t size, const char * fmt)
{
	if (_stricmp(fmt, "png") == 0)
		return zimg_from_png(src, size);
	throw ZFileException("zimg_from_img(): unknown file format.");
}

typedef void(*PFN_ZIMG_FormatConverter)(const uint8_t *srcElem, uint8_t *dstElem);

template<uint8_t elemSize>
void zimg_fmtcnv_identity(const uint8_t *srcElem, uint8_t *dstElem)
{
	memcpy_s(dstElem, elemSize, srcElem, elemSize);
}
template<uint8_t copySize, uint8_t fillSize>
void zimg_fmtcnv_expand(const uint8_t *srcElem, uint8_t *dstElem)
{
	memcpy_s(dstElem, copySize, srcElem, copySize);
	memset(dstElem + copySize, 0, fillSize);
}

PFN_ZIMG_FormatConverter zimg_get_format_converter(ImageFormat src, ImageFormat dst)
{
	if (src == dst)
	{
		switch (src)
		{
		case zenith::util::zfile_format::ImageFormat::R8: return zimg_fmtcnv_identity<1>;
		case zenith::util::zfile_format::ImageFormat::R16: return zimg_fmtcnv_identity<2>;
		case zenith::util::zfile_format::ImageFormat::R32: return zimg_fmtcnv_identity<4>;
		case zenith::util::zfile_format::ImageFormat::R32F: return zimg_fmtcnv_identity<4>;
		case zenith::util::zfile_format::ImageFormat::R8G8: return zimg_fmtcnv_identity<2>;
		case zenith::util::zfile_format::ImageFormat::R16G16: return zimg_fmtcnv_identity<4>;
		case zenith::util::zfile_format::ImageFormat::R32G32: return zimg_fmtcnv_identity<8>;
		case zenith::util::zfile_format::ImageFormat::R32G32F: return zimg_fmtcnv_identity<8>;
		case zenith::util::zfile_format::ImageFormat::R8G8B8: return zimg_fmtcnv_identity<3>;
		case zenith::util::zfile_format::ImageFormat::R16G16B16: return zimg_fmtcnv_identity<6>;
		case zenith::util::zfile_format::ImageFormat::R32G32B32: return zimg_fmtcnv_identity<12>;
		case zenith::util::zfile_format::ImageFormat::R32G32B32F: return zimg_fmtcnv_identity<12>;
		case zenith::util::zfile_format::ImageFormat::R8G8B8A8: return zimg_fmtcnv_identity<4>;
		case zenith::util::zfile_format::ImageFormat::R16G16B16A16: return zimg_fmtcnv_identity<8>;
		case zenith::util::zfile_format::ImageFormat::R32G32B32A32: return zimg_fmtcnv_identity<16>;
		case zenith::util::zfile_format::ImageFormat::R32G32B32A32F: return zimg_fmtcnv_identity<16>;
		default: throw ZFileException("zimg_get_format_converter(): no suitable identity converter found for requested format.");
		}
	}
	if (src == ImageFormat::R8G8B8 && dst == ImageFormat::R8G8B8A8)
		return zimg_fmtcnv_expand<3, 1>;
	throw ZFileException("zimg_get_format_converter(): no suitable identity converter found for requested format.");
}


zImgDescription zenith::util::zfile_format::zimg_change_format(const zImgDescription &src, ImageFormat dstFormat)
{
	zImgDescription r;
	r.width = src.width;
	r.height = src.height;
	r.arraySize = src.arraySize;
	r.depth = src.depth;
	r.imageType = src.imageType;
	r.mipLevels = src.mipLevels;
	r.imageFormat = dstFormat;

	if(r.imageType != ImageType::IMG1D && r.imageType != ImageType::IMG1D_ARRAY
		&& r.imageType != ImageType::IMG2D && r.imageType != ImageType::IMG2D_ARRAY
		&& r.imageType != ImageType::IMG3D && r.imageType != ImageType::IMG3D_ARRAY)
		throw ZFileException("zimg_change_format(): unknown image type.");

	uint8_t dstPixSize = getPixelSize(dstFormat);
	uint8_t srcPixSize = getPixelSize(src.imageFormat);

	PFN_ZIMG_FormatConverter fmtConverter = zimg_get_format_converter(src.imageFormat, dstFormat);

	for (uint32_t i = 0; i < r.mipLevels; i++)
	{
		r.levelDescr[i] = new zImgDataDescription;
		switch (r.imageType)
		{
		case ImageType::IMG1D: *r.levelDescr[i] = zImgDataDescription::create1D(dstPixSize, r.width); break;
		case ImageType::IMG1D_ARRAY: *r.levelDescr[i] = zImgDataDescription::create1DArr(dstPixSize, r.width, r.arraySize, uint64_t(dstPixSize) * r.width); break;
		case ImageType::IMG2D: *r.levelDescr[i] = zImgDataDescription::create2D(r.width, r.height, uint64_t(dstPixSize) * r.width); break;
		case ImageType::IMG2D_ARRAY: *r.levelDescr[i] = zImgDataDescription::create2DArr(r.width, r.height, r.arraySize, uint64_t(dstPixSize) * r.width, uint64_t(dstPixSize) * r.width * r.height); break;
		case ImageType::IMG3D: *r.levelDescr[i] = zImgDataDescription::create3D(r.width, r.height, r.depth, uint64_t(dstPixSize) * r.width, uint64_t(dstPixSize) * r.width * r.height); break;
		case ImageType::IMG3D_ARRAY: *r.levelDescr[i] = zImgDataDescription::create3DArr(r.width, r.height, r.depth, r.arraySize, uint64_t(dstPixSize) * r.width, uint64_t(dstPixSize) * r.width * r.height, uint64_t(dstPixSize) * r.width * r.height * r.depth); break;
		}
		uint64_t dstSize = r.levelDescr[i]->getRequiredDataSize();
		r.levelData[i] = new uint8_t[dstSize];

		uint64_t srcAPitch = src.levelDescr[i]->getArrayPitch();
		uint64_t dstAPitch = r.levelDescr[i]->getArrayPitch();

		uint64_t srcDPitch = src.levelDescr[i]->getDepthPitch();
		uint64_t dstDPitch = r.levelDescr[i]->getDepthPitch();

		uint64_t srcRPitch = src.levelDescr[i]->getRowPitch();
		uint64_t dstRPitch = r.levelDescr[i]->getRowPitch();
		for (uint32_t ia = 0; ia < r.arraySize; ia++)
		{
			const uint8_t * psa = reinterpret_cast<const uint8_t *>(src.levelData[i]) + srcAPitch * ia;
			uint8_t * pda = reinterpret_cast<uint8_t *>(r.levelData[i]) + dstAPitch * ia;			
			for (uint32_t id = 0; id < r.depth; id++)
			{
				const uint8_t * psd = psa + srcDPitch * id;
				uint8_t * pdd = pda + dstDPitch * id;
				for (uint32_t ih = 0; ih < r.height; ih++)
				{
					const uint8_t * psh = psd + srcRPitch * ih;
					uint8_t * pdh = pdd + dstRPitch * ih;
					for (uint32_t iw = 0; iw < r.width; iw++)
					{
						const uint8_t * ps = psh + uint64_t(iw) * srcPixSize;
						uint8_t * pd = pdh + uint64_t(iw) * dstPixSize;
						fmtConverter(ps, pd);
					}
				}
			}
		}
	}
	return r;
}

zImgDescription zenith::util::zfile_format::zimg_clone(const zImgDescription &src)
{
	zImgDescription r;
	r.width = src.width;
	r.height = src.height;
	r.arraySize = src.arraySize;
	r.depth = src.depth;
	r.imageType = src.imageType;
	r.mipLevels = src.mipLevels;
	r.imageFormat = src.imageFormat;

	uint8_t pixSize = getPixelSize(src.imageFormat);

	for (uint32_t i = 0; i < r.mipLevels; i++)
	{
		r.levelDescr[i] = new zImgDataDescription;
		switch (r.imageType)
		{
		case ImageType::IMG1D: *r.levelDescr[i] = zImgDataDescription::create1D(pixSize, r.width); break;
		case ImageType::IMG1D_ARRAY: *r.levelDescr[i] = zImgDataDescription::create1DArr(pixSize, r.width, r.arraySize, src.levelDescr[i]->getArrayPitch()); break;
		case ImageType::IMG2D: *r.levelDescr[i] = zImgDataDescription::create2D(r.width, r.height, src.levelDescr[i]->getRowPitch()); break;
		case ImageType::IMG2D_ARRAY: *r.levelDescr[i] = zImgDataDescription::create2DArr(r.width, r.height, r.arraySize, src.levelDescr[i]->getRowPitch(), src.levelDescr[i]->getArrayPitch()); break;
		case ImageType::IMG3D: *r.levelDescr[i] = zImgDataDescription::create3D(r.width, r.height, r.depth, src.levelDescr[i]->getRowPitch(), src.levelDescr[i]->getDepthPitch()); break;
		case ImageType::IMG3D_ARRAY: *r.levelDescr[i] = zImgDataDescription::create3DArr(r.width, r.height, r.depth, r.arraySize, src.levelDescr[i]->getRowPitch(), src.levelDescr[i]->getDepthPitch(), src.levelDescr[i]->getArrayPitch()); break;
		}
		uint64_t dstSize = r.levelDescr[i]->getRequiredDataSize();
		r.levelData[i] = new uint8_t[dstSize];

		uint64_t srcAPitch = src.levelDescr[i]->getArrayPitch();
		uint64_t dstAPitch = r.levelDescr[i]->getArrayPitch();

		uint64_t srcDPitch = src.levelDescr[i]->getDepthPitch();
		uint64_t dstDPitch = r.levelDescr[i]->getDepthPitch();

		uint64_t srcRPitch = src.levelDescr[i]->getRowPitch();
		uint64_t dstRPitch = r.levelDescr[i]->getRowPitch();
		for (uint32_t ia = 0; ia < r.arraySize; ia++)
		{
			const uint8_t * psa = reinterpret_cast<const uint8_t *>(src.levelData[i]) + srcAPitch * ia;
			uint8_t * pda = reinterpret_cast<uint8_t *>(r.levelData[i]) + dstAPitch * ia;
			for (uint32_t id = 0; id < r.depth; id++)
			{
				const uint8_t * psd = psa + srcDPitch * id;
				uint8_t * pdd = pda + dstDPitch * id;
				for (uint32_t ih = 0; ih < r.height; ih++)
				{
					const uint8_t * psh = psd + srcRPitch * ih;
					uint8_t * pdh = pdd + dstRPitch * ih;
					memcpy_s(pdh, srcRPitch, pdg, dstRPitch);
				}
			}
		}
	}
	return r;
}


void zenith::util::zfile_format::zimg_free(zImgDescription &d)
{
	for (uint32_t i = 0; i < d.mipLevels; i++)
	{
		delete d.levelDescr[i];
		delete[] d.levelData[i];
	}
}
