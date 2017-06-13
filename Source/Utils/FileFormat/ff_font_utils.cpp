#include "ff_font_utils.h"
#include "ff_img_utils.h"
#include <Utils\xml_tools.h>
#include <glm/glm.hpp>

using namespace zenith::util::zfile_format;

zFontDescription zfont_from_xml_bmfont_(void * src, uint64_t size)
{
	zFontDescription res = zFontDescription::empty();
	pugi::xml_document doc;
	doc.load_buffer(src, size);
	auto xInfo = doc.child("font").child("info");
	auto xCommon = doc.child("font").child("common");
	auto xChars = doc.child("font").child("chars");
	if(xInfo.empty() || xCommon.empty() || xChars.empty())
		throw ZFileException("zfont_from_xml_bmfont_(): provided xml does not look like BMFont xml.");

	res.setName(xInfo.attribute("face").as_string());
	res.setEncoding("CP1251");
	res.fontTraits.isBold = xInfo.attribute("bold").as_uint();
	res.fontTraits.isItalic = xInfo.attribute("italic").as_uint();
	res.fontSize = xInfo.attribute("size").as_uint();
	res.fontType = FontType::ATLAS_ALPHA;

	uint32_t w = xCommon.attribute("scaleW").as_uint();
	uint32_t h = xCommon.attribute("scaleH").as_uint();

	std::string sPadding = xInfo.attribute("padding").as_string();
	std::string sSpacing = xInfo.attribute("spacing").as_string();

	uint32_t numGlyphs = 0;
	for (const auto &ch : xChars.children("char"))
		numGlyphs++;

	res.setGlyphs(new zFontGlyphDescription[numGlyphs], numGlyphs);
	uint32_t idGlyph = 0;
	for (const auto &ch : xChars.children("char"))
	{
		zFontGlyphDescription &gd = res.getGlyph(idGlyph);
		gd.glyphId = ch.attribute("id").as_uint();

		gd.xOffset = ch.attribute("xoffset").as_int();
		gd.yOffset = ch.attribute("xoffset").as_int();
		gd.xAdvance = ch.attribute("xadvance").as_int();

		gd.x = ch.attribute("x").as_double() / double(w);
		gd.y = ch.attribute("y").as_double() / double(h);
		gd.w = ch.attribute("width").as_double() / double(w);
		gd.h = ch.attribute("height").as_double() / double(h);
		
		idGlyph++;
	}

	res.intAtlasImage.imageType = ImageType::UNDEF;
	res.extAtlasFileName = nullptr;

	return res;
}

zFontDescription zenith::util::zfile_format::zfont_from_xml(void * src, uint64_t size)
{
	try {
		return zfont_from_xml_bmfont_(src, size);
	}
	catch (...) {}
	throw ZFileException("zfont_from_xml(): unknown xml-format.");
}

zFontDescription zenith::util::zfile_format::zfont_from_zimg(const zImgDescription &d)
{
	zFontDescription res = zFontDescription::empty();
	res.intAtlasImage = d;
	return res;
}
void zenith::util::zfile_format::zfont_free(zFontDescription &d)
{
	if(d.intAtlasImage.imageType != ImageType::UNDEF)
		zimg_free(d.intAtlasImage);
	if(d.extAtlasFileName)
		delete[] d.extAtlasFileName;
	if(d.glyphsPtr())
		delete[] d.glyphsPtr();
}

zf_memregion2d_t zenith::util::zfile_format::zfont_extract_glyph(const zFontDescription & d, uint32_t glyphId)
{
	if (glyphId >= d.numGlyphs())
		throw ZFileException("zfont_extract_glyph: glyphId out of bounds!");
	const auto &g = d.getGlyph(glyphId);
	
	uint32_t gw = static_cast<uint32_t>(std::ceil(double(d.intAtlasImage.width) * g.w));
	uint32_t gh = static_cast<uint32_t>(std::ceil(double(d.intAtlasImage.height) * g.h));
	uint32_t x0 = static_cast<uint32_t>(std::floor(double(d.intAtlasImage.width) * g.x));
	uint32_t y0 = static_cast<uint32_t>(std::floor(double(d.intAtlasImage.height) * g.y));

	uint64_t rowPitch = d.intAtlasImage.levelDescr[0]->getRowPitch();
	uint32_t elemSize = getPixelSize(d.intAtlasImage.imageFormat);
	uint32_t elemNum = gw;
	uint32_t rowNum = gh;
	uint8_t * ptr = static_cast<uint8_t *>(d.intAtlasImage.levelData[0]);
	ptr += y0 * rowPitch;
	ptr += x0 * elemSize;
	
	zf_memaddr2d<> res(elemSize, elemSize, rowPitch);

	return zf_memregion2d_t(res, elemNum, rowNum, static_cast<void *>(ptr));
}

struct _zfu_glyph_loc
{
	static const uint32_t NoLoc = std::numeric_limits<uint32_t>::max();
	uint32_t x = NoLoc, y = NoLoc;
	uint32_t w = 0, h = 0;
	uint32_t id = 0;

	inline bool has_location() const { return x != NoLoc && y != NoLoc; }
	inline void remove_location() { x = NoLoc; y = NoLoc; }
};

template<class It>
uint32_t zfont_pack_fill_gready_strip_(It first, It last, uint32_t curX, uint32_t curY,
	uint32_t h, uint32_t w)
{
	uint32_t minHSplit = h;
	if (first == last)
		return curX;

	It rit = last; rit--;
	for (; rit != first; rit--)
		if (rit->has_location())
			continue;
		else
		{
			minHSplit = rit->h;
			break;
		}

	for (It it = first; it != last; it++)
	{
		if (it->has_location())
			continue;
		if (it->h > h || it->w > w)
			continue;
		if (it->h + minHSplit <= h)
		{
			It optIt = last; optIt--;
			uint32_t minHSplit0 = h;
			for (; optIt != it; optIt--)
				if (optIt->has_location())
					continue;
				else
				{ 
					if (optIt->h + it->h > h)
						break;
					if (minHSplit0 > optIt->h)
						minHSplit0 = optIt->h;
				}
			optIt++;
			minHSplit = minHSplit0; //update split size

			if (optIt == last)
			{
				it->x = curX;
				it->y = curY;

				curX += it->w;
				w -= it->w;
				continue;
			}

			uint32_t x1 = zfont_pack_fill_gready_strip_(it, optIt, curX, curY, it->h, w);
			uint32_t x2 = zfont_pack_fill_gready_strip_(optIt, last, curX, curY + it->h, optIt->h, w);
			uint32_t x = (x1 > x2 ? x1 : x2);
			uint32_t dx = x - curX;
			if(dx >= w)
				return x;
			curX += dx;
			w -= dx;
		}
		else
		{
			it->x = curX;
			it->y = curY;

			curX += it->w;
			w -= it->w;
		}
	}
	return curX;
}

double zfont_pack_calc_(const std::vector<_zfu_glyph_loc> &res, uint32_t w, uint32_t h)
{
	uint64_t sumArea = 0, fullArea = uint64_t(w) * uint64_t(h);
	for (const auto &x : res)
		if (x.has_location())
			sumArea += x.w * x.h;
		else
			return -1.0;
	return double(sumArea) / double(fullArea);
}

double zfont_pack_fill_gready_(std::vector<_zfu_glyph_loc> &res, uint32_t w, uint32_t h)
{
	std::sort(res.begin(), res.end(), [](const _zfu_glyph_loc &lh, const _zfu_glyph_loc &rh){
		return (lh.h == rh.h ? lh.w > rh.w : lh.h > rh.h);
	});

	uint32_t curY = 0;
	uint32_t leftH = h;

	for (auto i = 0; i < res.size(); i++)
	{
		if (res[i].has_location())
			continue;
		if (res[i].h > leftH || res[i].w > w)
			return -1.0;
		uint32_t stripH = res[i].h;
		
		zfont_pack_fill_gready_strip_(res.begin() + i, res.end(),
			0, curY, stripH, w);

		curY += stripH;
		leftH -= stripH;
	}
	return zfont_pack_calc_(res, w, h);
}

double zfont_atlas_try_pack_(std::vector<_zfu_glyph_loc> &locs, uint32_t w, uint32_t h)
{	
	uint32_t maxW = 0, maxH = 0;
	uint64_t sumArea = 0;
	for (uint32_t i = 0; i < locs.size(); i++)
	{
		if (locs[i].w > maxW)
			maxW = locs[i].w;
		if (locs[i].h > maxH)
			maxH = locs[i].h;
		sumArea += locs[i].w * locs[i].h;
	}
	if (sumArea > uint64_t(w) * uint64_t(h))
		return -1.0;
	if (maxW > w || maxH > h)
		return -1.0;

	double useRatio = zfont_pack_fill_gready_(locs, w, h);
	if (useRatio <= 0.0)
		return -1.0;
	return useRatio;
}

std::vector<_zfu_glyph_loc> zfont_atlas_init_loc_(const zf_memregion2d_t * glyphs, uint32_t num,
	uint32_t pad_x_neg, uint32_t pad_x_pos, uint32_t pad_y_neg, uint32_t pad_y_pos)
{
	std::vector<_zfu_glyph_loc> res;
	res.reserve(num);
	for (uint32_t i = 0; i < num; i++)
	{
		uint32_t ch = glyphs[i].row_num();
		uint32_t cw = glyphs[i].elem_num();

		_zfu_glyph_loc loc;
		loc.remove_location();
		loc.w = cw + pad_x_neg + pad_x_pos;
		loc.h = ch + pad_y_neg + pad_y_pos;
		loc.id = i;

		res.push_back(loc);
	}
	return res;
}

bool zenith::util::zfile_format::zfont_atlas_is_packable(const zf_memregion2d_t * glyphs, uint32_t num, uint32_t w, uint32_t h, uint32_t padding)
{	
	std::vector<_zfu_glyph_loc> locs = std::move(zfont_atlas_init_loc_(glyphs, num, padding, padding, padding, padding));
	return zfont_atlas_try_pack_(locs, w, h) > 0.0;
}
void zenith::util::zfile_format::zfont_atlas_pack(zFontDescription & dst, zf_memregion2d_t * glyphs, uint32_t padding)
{
	uint32_t pixSize = getPixelSize(dst.intAtlasImage.imageFormat);
	uint64_t rowPitch = dst.intAtlasImage.levelDescr[0]->getRowPitch();
	for(uint32_t i = 0; i < dst.numGlyphs(); i++)
		if(glyphs[i].elem_size() != pixSize)
			throw ZFileException("zfont_atlas_pack: glyphs element size and atlas image format are incompatible!");

	std::vector<_zfu_glyph_loc> locs = std::move(zfont_atlas_init_loc_(glyphs, dst.numGlyphs(), padding, padding, padding, padding));
	double uRatio = zfont_atlas_try_pack_(locs, dst.intAtlasImage.width, dst.intAtlasImage.height);
	if (uRatio <= 0.0)
		throw ZFileException("zfont_atlas_pack: atlas is not packable into provided dimensions!");

	double iw = 1.0 / double(dst.intAtlasImage.width);
	double ih = 1.0 / double(dst.intAtlasImage.height);
	for (uint32_t i = 0; i < locs.size(); i++)
	{
		auto gid = locs[i].id;
		auto &glyph = dst.getGlyph(gid);

		uint32_t realX = locs[i].x + padding;
		uint32_t realY = locs[i].y + padding;
		
		//1 - move into atlas
		zf_memregion2d_t &srcmm = glyphs[gid];
		uint8_t * pdst = static_cast<uint8_t *>(dst.intAtlasImage.levelData[0]) + rowPitch * realY + pixSize * realX;
		zf_memaddr2d<uint64_t> dst_a(pixSize, pixSize, rowPitch);
		zf_memregion2d_t dstmm(dst_a, dst.intAtlasImage.width, dst.intAtlasImage.height, pdst);
		zf_memregion2d_t::memcpy(dstmm, srcmm);

		//2 - update glyph description
		glyph.x = double(realX) * iw;
		glyph.y = double(realY) * ih;
		glyph.w = double(locs[i].w - 2 * padding) * iw;
		glyph.h = double(locs[i].h - 2 * padding) * ih;
	}
}

void zfont_atlas_sigdist_glyph_init_8b_(glm::vec3 * vBuff, uint32_t buffSize, zf_memregion2d_t glyph, int extD)
{
	uint32_t w = glyph.elem_num();
	uint32_t h = glyph.row_num();
	
	uint32_t gi = 0;
	for(int i = -extD; i < int(h)+extD; i++)
		for (int j = -extD; j < int(w) + extD; j++)
		{
			int ci = (i >= 0 ? (i < h ? i : h - 1) : 0);
			int cj = (j >= 0 ? (j < w ? j : w - 1) : 0);
			if (glyph.get<uint8_t>(ci, cj) > 127)
				vBuff[gi] = glm::vec3(0.0f, 0.0f, 1.0f);
			else
				vBuff[gi] = glm::vec3(0.0f, 0.0f, -1.0f);
			gi++;
		}
	for (; gi < buffSize; gi++)
		vBuff[gi] = glm::vec3(0.0f, 0.0f, 0.0f);
}

void zfont_atlas_sigdist_glyph_save_8b_(glm::vec3 * vBuff, uint32_t buffSize, zf_memregion2d_t glyph, float minV, float maxV)
{
	uint32_t w = glyph.elem_num();
	
	for (uint32_t i = 0; i < glyph.row_num(); i++)
		for (uint32_t j = 0; j < w; j++)
		{
			float fl = 0.0f;
			float fLen = glm::length(glm::vec2(vBuff[i * w + j]));
			if (vBuff[i * w + j].z > 0.0f)
				fl = 0.5f + 0.5f * glm::min(fLen, maxV) / maxV;
			else
				fl = 0.5f - 0.5f * glm::max(-fLen, minV) / minV;

			glyph.get<uint8_t>(i, j) = static_cast<uint8_t>(fl * 255.0f);
		}
}

bool zfont_atlas_sigdist_glyph_run1_(glm::vec3 * vBuff, uint32_t x, uint16_t y, uint32_t w, uint32_t h,
	glm::vec3 &newVal)
{
	newVal = vBuff[y*w + x];
	float f0 = newVal.z;
	if (glm::abs(f0) < 1e-7f)
		return false;
	bool updated = false;
	for(int di = -1; di <= 1; di++)
		for (int dj = -1; dj <= 1; dj++)
		{
			if (di == 0 && dj == 0)
				continue;
			int nx = int(x) + dj;
			int ny = int(y) + di;
			if (nx < 0 || ny < 0)
				continue;
			if (nx >= w || ny >= h)
				continue;

			float f1 = vBuff[ny*w + nx].z;
			glm::vec2 cVec = glm::vec2(vBuff[ny*w + nx]);
			float curL = glm::length(glm::vec2(newVal));
			if (f0 * f1 < -0.5f)
			{
				//we are on the border
				if (curL < 1e-7f || curL > glm::length(glm::vec2(dj, di)))
				{
					updated = true;
					newVal = glm::vec3(dj, di, f0);
				}
			}
			else if (f0 * f1 > 0.5f && glm::length(cVec) > 1e-7f)
			{
				//we are outside or inside
				glm::vec2 tVec = cVec + glm::vec2(dj, di);
				if (curL < 1e-7f || curL > glm::length(tVec))
				{
					updated = true;
					newVal = glm::vec3(tVec, f0);
				}
			}
		}
	return updated;
}


void zfont_atlas_sigdist_insert1_(uint32_t * wBuff, uint32_t wbSize, uint32_t wBot, uint32_t &wTop, uint32_t val)
{
	uint32_t i = wBot;
	while (i != wTop)
	{
		if (wBuff[i] == val)
			return;
		i++;
		if (i >= wbSize)
			i = 0;
	}
	wBuff[wTop++] = val;
	if (wTop >= wbSize)
		wTop = 0;
}

void zfont_atlas_sigdist_insert_(uint32_t * wBuff, uint32_t wbSize, uint32_t wBot, uint32_t &wTop,
	uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	for (int di = -1; di <= 1; di++)
		for (int dj = -1; dj <= 1; dj++)
		{
			if (di == 0 && dj == 0)
				continue;
			int nx = int(x) + dj;
			int ny = int(y) + di;
			if (nx < 0 || ny < 0)
				continue;
			if (nx >= w || ny >= h)
				continue;
			uint32_t val = ny * w + nx;
			zfont_atlas_sigdist_insert1_(wBuff, wbSize, wBot, wTop, val);
		}
}

void zfont_atlas_sigdist_glyph_run_(glm::vec3 * vBuff, uint32_t * wBuff, uint32_t w, uint32_t h, uint32_t wbSize)
{
	uint32_t wTop = 0;
	uint32_t wBot = 0;
	for(uint32_t i = 0; i< wbSize; i++)
		wBuff[i] = 0;

	glm::vec3 tmp;
	for (uint32_t i = 0; i < h; i++)
		for(uint32_t j = 0; j < w; j++)
		{
			if (zfont_atlas_sigdist_glyph_run1_(vBuff, j, i, w, h, tmp))
			{
				vBuff[i*w + j] = tmp;
				zfont_atlas_sigdist_insert_(wBuff, wbSize, wBot, wTop, j, i, w, h);
			}
		}
	while (wBot != wTop)
	{
		uint32_t idx = wBuff[wBot++];
		if (wBot >= wbSize)
			wBot = 0;

		uint32_t x = idx % w;
		uint32_t y = idx / w;
		if (zfont_atlas_sigdist_glyph_run1_(vBuff, x, y, w, h, tmp))
		{
			vBuff[idx] = tmp;
			zfont_atlas_sigdist_insert_(wBuff, wbSize, wBot, wTop, x, y, w, h);
		}
	}
}

zFontDescription zenith::util::zfile_format::zfont_atlas_sigdist(const zFontDescription & src, uint32_t distExtension, uint32_t rW, uint32_t rH, uint32_t padding, float clampMin, float clampMax)
{
	if (src.fontType != FontType::ATLAS_ALPHA)
		throw ZFileException("zfont_atlas_sigdist: source font expected to be of ATLAS_ALPHA type!");
	if(src.intAtlasImage.imageFormat != ImageFormat::R8)
		throw ZFileException("zfont_atlas_sigdist: source font atlas format should be R8!");

	auto resFormat = ImageFormat::R8;

	std::vector<_zfu_glyph_loc> locs;
	locs.reserve(src.numGlyphs());
	for (uint32_t i = 0; i < src.numGlyphs(); i++)
	{
		uint32_t w = static_cast<uint32_t>(std::ceil(src.getGlyph(i).w * double(src.intAtlasImage.width)));
		uint32_t h = static_cast<uint32_t>(std::ceil(src.getGlyph(i).h * double(src.intAtlasImage.height)));

		_zfu_glyph_loc loc;
		loc.remove_location();
		loc.w = w + 2 * (distExtension + padding);
		loc.h = h + 2 * (distExtension + padding);
		loc.id = i;

		locs.push_back(loc);
	}
	
	double uRatio = zfont_atlas_try_pack_(locs, rW, rH);
	if (uRatio < 0.0)
		throw ZFileException("zfont_atlas_sigdist: unable to pack resulting atlas!");

	std::sort(locs.begin(), locs.end(), [](const _zfu_glyph_loc &lh, const _zfu_glyph_loc &rh) {
		return lh.id < rh.id;
	});

	zFontDescription res;
	uint32_t maxH = 0, maxW = 0;
	for (uint32_t i = 0; i < src.numGlyphs(); i++)
	{
		uint32_t w = static_cast<uint32_t>(std::ceil(src.getGlyph(i).w * double(src.intAtlasImage.width)));
		uint32_t h = static_cast<uint32_t>(std::ceil(src.getGlyph(i).h * double(src.intAtlasImage.height)));

		if (maxH < h)
			maxH = h;
		if (maxW < w)
			maxW = w;
	}
	uint32_t buffW = maxW + distExtension * 2;
	uint32_t buffH = maxH + distExtension * 2;
	uint32_t buffS = buffW * buffH;


	std::unique_ptr<glm::vec3[]> vBuff(new glm::vec3[buffS]);
	std::unique_ptr<uint32_t[]> wBuff(new uint32_t[buffS]);

	uint32_t elemSize = getPixelSize(src.intAtlasImage.imageFormat);
	uint32_t resSize = getPixelSize(resFormat);
	
	std::unique_ptr<uint8_t[]> wData(new uint8_t[resSize * rW * rH]);
	std::unique_ptr<zFontGlyphDescription[]> gData(new zFontGlyphDescription[src.numGlyphs()]);
	res.setEncoding(src.encodingName);
	res.setName(src.fontName);
	res.fontSize = src.fontSize;
	res.fontTraits = src.fontTraits;
	res.fontType = FontType::ATLAS_SIGDIST;
	res.setGlyphs(gData.get(), src.numGlyphs());

	//create glyphs
	double irW = 1 / double(rW);
	double irH = 1 / double(rH);

	for (uint32_t i = 0; i < src.numGlyphs(); i++)
	{
		uint32_t x = static_cast<uint32_t>(std::floor(src.getGlyph(i).x * double(src.intAtlasImage.width)));
		uint32_t y = static_cast<uint32_t>(std::floor(src.getGlyph(i).y * double(src.intAtlasImage.height)));

		uint32_t w = static_cast<uint32_t>(std::ceil(src.getGlyph(i).w * double(src.intAtlasImage.width)));
		uint32_t h = static_cast<uint32_t>(std::ceil(src.getGlyph(i).h * double(src.intAtlasImage.height)));

		uint32_t nw = locs[i].w - 2 * padding;
		uint32_t nh = locs[i].h - 2 * padding;
		uint32_t nx = locs[i].x + padding;
		uint32_t ny = locs[i].y + padding;

		zf_memaddr2d<> ma(elemSize, elemSize, src.intAtlasImage.width);
		const uint8_t * ptru = static_cast<const uint8_t *>(src.intAtlasImage.levelData[0])
			+ y * src.intAtlasImage.levelDescr[0]->getRowPitch()
			+ x * elemSize;
		void * ptr = static_cast<void *>(const_cast<uint8_t *>(ptru));
		zf_memregion2d_t mr(ma, w, h, ptr);

		zf_memaddr2d<> da(resSize, resSize, rW);
		uint8_t * dptr = static_cast<uint8_t *>(wData.get())
			+ ny * resSize * rW
			+ nx * resSize;
		zf_memregion2d_t dmr(da, nw, nh, dptr);

		zfont_atlas_sigdist_glyph_init_8b_(vBuff.get(), buffS, mr, distExtension);
		zfont_atlas_sigdist_glyph_run_(vBuff.get(), wBuff.get(), w + 2 * distExtension, h + 2 * distExtension, buffS);
		zfont_atlas_sigdist_glyph_save_8b_(vBuff.get(), buffS, dmr, clampMin, clampMax);

		res.getGlyph(i) = src.getGlyph(i);
		res.getGlyph(i).x = double(nx) * irW;
		res.getGlyph(i).y = double(ny) * irH;
		res.getGlyph(i).w = double(nw) * irW;
		res.getGlyph(i).h = double(nh) * irH;
	}
	auto pDsc = new zImgDataDescription;
	res.intAtlasImage.levelDescr[0] = pDsc;
	*pDsc = zImgDataDescription::create2D(rW, rH, resSize * rW);
	
	res.intAtlasImage.width = rW;
	res.intAtlasImage.height = rH;
	res.intAtlasImage.arraySize = 1;
	res.intAtlasImage.depth = 1;
	res.intAtlasImage.imageType = ImageType::IMG2D;
	res.intAtlasImage.mipLevels = 1;
	res.intAtlasImage.imageFormat = resFormat;
	res.intAtlasImage.levelData[0] = wData.release();
	gData.release();

	return res;
}
