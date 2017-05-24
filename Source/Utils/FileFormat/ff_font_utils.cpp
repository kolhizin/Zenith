#include "ff_font_utils.h"
#include "ff_img_utils.h"
#include <Utils\xml_tools.h>

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