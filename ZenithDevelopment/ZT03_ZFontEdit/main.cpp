#include "init.h"
#include <Utils\str_cast.h>

zenith::util::io::FileSystem * fs;

inline std::pair<std::future<zenith::util::io::FileResult>, std::unique_ptr<unsigned char[]>> readFile(const char * fname, float priority = 1.0f)
{
	auto f = fs->createFile();
	f.open(fname, 'r', priority);
	size_t sz = f.size_left(priority).get().size;
	unsigned char * data = new unsigned char[sz];
	std::unique_ptr<unsigned char[]> h(data);
	auto r = f.read(data, sz, priority);
	f.close();
	return std::pair<std::future<zenith::util::io::FileResult>, std::unique_ptr<unsigned char[]>>(std::move(r), std::move(h));
}

inline std::future<zenith::util::io::FileResult> writeFile(const char * fname, uint8_t * data, size_t size, float priority = 1.0f)
{
	auto f = fs->createFile();
	f.open(fname, 'w', priority);
	auto r = f.write(data, size);
	f.close();
	return r;
}


struct NamedParameter
{
	std::string paramName;
	std::string paramValue;
};

NamedParameter parseArgument(const char * str)
{
	NamedParameter res;
	bool quoted = false;
	const char * str2 = str;
	uint32_t len1 = 0;
	while (*str2 != '\0')
	{
		if (*str2 == '\"')
			quoted = !quoted;
		if (!quoted && *str2 == '=')
		{
			str2++;
			break;
		}
		str2++;
		len1++;
	}
	if (!*str2)
	{
		res.paramName = "";
		res.paramValue = str;
		return res;
	}
	res.paramName = std::string(str, len1);
	res.paramValue = str2;
	return res;
}

std::vector<NamedParameter> parseArguments(int argc, const char *argv[])
{
	std::vector<NamedParameter> res;
	res.reserve(argc);
	for (uint32_t i = 1; i < argc; i++)
		res.push_back(parseArgument(argv[i]));
	return res;
}

std::vector<zenith::util::zfile_format::zFontDescription> loadFontData(const std::vector<NamedParameter> &params)
{
	std::vector<std::pair<std::future<zenith::util::io::FileResult>, std::unique_ptr<unsigned char[]>>> srcs;
	std::vector<std::pair<std::string, std::string>> formats;
	for (const auto &p : params)
		if (p.paramName == "atlas" || p.paramName == "font")
		{
			auto f = fs->createFile();
			f.open(p.paramValue.c_str(), 'r', 1.0f);
			size_t sz = f.size_left(1.0f).get().size;
			unsigned char * data = new unsigned char[sz];
			std::unique_ptr<unsigned char[]> h(data);
			auto r = f.read(data, sz, 1.0f);
			f.close();

			srcs.emplace_back(std::move(r), std::move(h));
			std::string ext = p.paramValue.substr(p.paramValue.find_last_of('.') + 1);
			formats.emplace_back(p.paramName, std::move(ext));
		}

	std::vector<zenith::util::zfile_format::zFontDescription> res;
	res.reserve(srcs.size());
	for (uint32_t i = 0; i < srcs.size(); i++)
	{
		auto r = srcs[i].first.get();
		std::transform(formats[i].second.begin(), formats[i].second.end(), formats[i].second.begin(), ::tolower);
		if (formats[i].first == "font" && formats[i].second == "zfnt")
			res.push_back(zenith::util::zfile_format::zfont_clone(zenith::util::zfile_format::zfont_from_mem(r.data, r.size)));
		else if (formats[i].first == "font" && formats[i].second != "zfnt")
			res.push_back(zenith::util::zfile_format::zfont_from_xml(r.data, r.size));
		else if (formats[i].first == "atlas" && formats[i].second == "zimg")
			res.push_back(zenith::util::zfile_format::zfont_from_zimg(zenith::util::zfile_format::zimg_clone(zenith::util::zfile_format::zimg_from_mem(r.data, r.size))));
		else if (formats[i].first == "atlas" && formats[i].second != "zimg")
			res.push_back(zenith::util::zfile_format::zfont_from_zimg(zenith::util::zfile_format::zimg_from_img(r.data, r.size, formats[i].second.c_str())));
	}
	return res;
}

zenith::util::zfile_format::zFontDescription combineFonts(const std::vector<NamedParameter> &params, std::vector<zenith::util::zfile_format::zFontDescription> &v)
{
	uint32_t numAtlases = 0;
	uint32_t numFontDs = 0;
	zenith::util::zfile_format::zFontDescription * fntAtl = nullptr;
	zenith::util::zfile_format::zFontDescription * fntDsc = nullptr;
	for (auto &ve : v)
	{
		if (ve.intAtlasImage.imageType != zenith::util::zfile_format::ImageType::UNDEF)
		{
			numAtlases++;
			fntAtl = &ve;
		}
		if (ve.numGlyphs() > 0)
		{
			numFontDs++;
			fntDsc = &ve;
		}
	}
	if (numAtlases != 1 || numFontDs != 1)
	{
		std::cout << "combineFonts error: either atlases, or font description is missing!";
		throw std::logic_error("combineFonts error: either atlases, or font description is missing!");
	}
	if (fntAtl == fntDsc)
		return *fntAtl;

	zenith::util::zfile_format::zFontDescription res = zenith::util::zfile_format::zFontDescription::empty();

	res.setGlyphs(*fntDsc);
	fntDsc->setGlyphs(nullptr, 0);

	res.setEncoding(fntDsc->encodingName);
	res.setName(fntDsc->fontName);
	res.fontSize = fntDsc->fontSize;
	res.fontTraits = fntDsc->fontTraits;
	res.fontType = fntDsc->fontType;

	res.intAtlasImage = fntAtl->intAtlasImage;
	fntAtl->intAtlasImage = zenith::util::zfile_format::zImgDescription::empty();

	return res;
}

const NamedParameter * findParam(const std::vector<NamedParameter> &params, const std::string &paramName)
{
	for (auto i = 0; i < params.size(); i++)
		if (params[i].paramName == paramName)
			return &params[i];
	return nullptr;
}

void performOutAtlas(const zenith::util::zfile_format::zFontDescription &font, const std::string &fname)
{
	uint64_t fSize = font.intAtlasImage.getFullSize();
	uint8_t * fData = new uint8_t[fSize];
	zenith::util::zfile_format::zimg_to_mem(font.intAtlasImage, fData, fSize);

	auto futW = writeFile(fname.c_str(), fData, fSize);
	futW.get();

	delete[] fData;
}

void performOutFont(const zenith::util::zfile_format::zFontDescription &font, const std::string &fname)
{
	uint64_t fSize = font.getFullSize();
	uint8_t * fData = new uint8_t[fSize];
	zenith::util::zfile_format::zfont_to_mem(font, fData, fSize);

	auto futW = writeFile(fname.c_str(), fData, fSize);
	futW.get();

	delete[] fData;
}

void transformSigDist(const std::vector<NamedParameter> &params, zenith::util::zfile_format::zFontDescription &font)
{
	uint32_t pExtDist = 8;
	uint32_t pPadding = 1;

	const NamedParameter * np = nullptr;
	if (np = findParam(params, "extDist"))
		pExtDist = zenith::util::str_cast<uint32_t>(np->paramValue);
	if (np = findParam(params, "padding"))
		pPadding = zenith::util::str_cast<uint32_t>(np->paramValue);

	double guessNum = std::ceil(std::sqrt(font.numGlyphs())*1.05); //+5%
	uint32_t extraSize = guessNum * (pExtDist * 2 + pPadding * 2);
	uint32_t pW = font.intAtlasImage.width + extraSize;
	uint32_t pH = font.intAtlasImage.height + extraSize;
	float pClampMin = -float(pExtDist);
	float pClampMax = float(pExtDist);

	if (np = findParam(params, "width"))
		pW = zenith::util::str_cast<uint32_t>(np->paramValue);
	if (np = findParam(params, "height"))
		pH = zenith::util::str_cast<uint32_t>(np->paramValue);
	if (np = findParam(params, "clampMin"))
		pClampMin = zenith::util::str_cast<float>(np->paramValue);
	if (np = findParam(params, "clampMax"))
		pClampMax = zenith::util::str_cast<float>(np->paramValue);

	zenith::util::zfile_format::zFontDescription tmp = zenith::util::zfile_format::zfont_atlas_sigdist(font, pExtDist, pW, pH, pPadding, pClampMin, pClampMax);
	zenith::util::zfile_format::zfont_free(font);
	font = tmp;
}

void performOperations(const std::vector<NamedParameter> &params, zenith::util::zfile_format::zFontDescription &font)
{
	const NamedParameter * np = nullptr;

	if (np = findParam(params, "transform"))
	{
		if (np->paramValue == "sigdist")
			transformSigDist(params, font);
		else
			throw std::runtime_error("Unknown transform option!");
	}

	if (np = findParam(params, "out_atlas"))
		performOutAtlas(font, np->paramValue);
	if (np = findParam(params, "out"))
		performOutFont(font, np->paramValue);
}

int main(int argc, const char *argv[])
{
	/*
	command line argument:
	1) font=testFont.xml atlas=testFont.png transform=sigdist out_atlas=testAtlas.zimg out=test.zfnt
	2) font=test.zfnt out_atlas=testAtlasF.zimg
	3) font=testFont.xml atlas=testFont.png out=test_orig.zfnt
	4) font=test_orig.zfnt out_atlas=testAtlasFO.zimg
	*/
	//0. initialize
	fs = new zenith::util::io::FileSystem(2); //two streams for loading
	//1. load font image & font description
	////a) get paths of image & font
	std::vector<NamedParameter> pArgs = parseArguments(argc, argv);

	std::vector<zenith::util::zfile_format::zFontDescription> srcFonts = loadFontData(pArgs);
	zenith::util::zfile_format::zFontDescription srcFont = combineFonts(pArgs, srcFonts);

	performOperations(pArgs, srcFont);

	//std::cout << sizeof(zenith::util::zfile_format::FontTraits) << " - " << alignof(zenith::util::zfile_format::FontTraits) << std::endl;
	//std::cout << sizeof(zenith::util::zfile_format::ZChunk16B_FontHeader) << " - " << alignof(zenith::util::zfile_format::ZChunk16B_FontHeader) << std::endl;
	//std::cout << sizeof(zenith::util::zfile_format::ZChunk64B_FontData_Glyph) << " - " << alignof(zenith::util::zfile_format::ZChunk64B_FontData_Glyph) << std::endl;
	
	return 0;
}