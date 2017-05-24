#include "init.h"

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
			;//res.push_back(zenith::util::zfile_format::zfont_from_xml(r.data, r.size));
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
}

int main(int argc, const char *argv[])
{
	//0. initialize
	fs = new zenith::util::io::FileSystem(2); //two streams for loading
	//1. load font image & font description
	////a) get paths of image & font
	std::vector<NamedParameter> pArgs = parseArguments(argc, argv);

	std::vector<zenith::util::zfile_format::zFontDescription> srcFonts = loadFontData(pArgs);

	std::cout << sizeof(zenith::util::zfile_format::FontTraits) << " - " << alignof(zenith::util::zfile_format::FontTraits) << std::endl;
	std::cout << sizeof(zenith::util::zfile_format::ZChunk16B_FontHeader) << " - " << alignof(zenith::util::zfile_format::ZChunk16B_FontHeader) << std::endl;
	std::cout << sizeof(zenith::util::zfile_format::ZChunk64B_FontData_Glyph) << " - " << alignof(zenith::util::zfile_format::ZChunk64B_FontData_Glyph) << std::endl;
	char r[3], d[3];
	std::string q = "na";

	zenith::util::zstrcpy(r, q.c_str());
	
	return 0;
}