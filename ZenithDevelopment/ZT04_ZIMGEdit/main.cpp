#include <Utils\IO\filesystem.h>
#include <Utils\FileFormat\ff_img.h>
#include <Utils\FileFormat\ff_img_utils.h>
#include <vector>
#include <iostream>

zenith::util::io::FileSystem * fs = nullptr;

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

std::vector<zenith::util::zfile_format::zImgDescription> loadSrcs(const std::vector<NamedParameter> &params)
{
	std::vector<std::pair<std::future<zenith::util::io::FileResult>, std::unique_ptr<unsigned char[]>>> srcs;
	std::vector<std::string> formats;
	for(const auto &p:params)
		if (p.paramName == "src")
		{
			auto f = fs->createFile();
			f.open(p.paramValue.c_str(), 'r', 1.0f);
			size_t sz = f.size_left(1.0f).get().size;
			unsigned char * data = new unsigned char[sz];
			std::unique_ptr<unsigned char[]> h(data);
			auto r = f.read(data, sz, 1.0f);
			f.close();

			srcs.emplace_back(std::move(r), std::move(h));
			std::string ext = p.paramValue.substr(p.paramValue.find_last_of('.')+1);
			formats.emplace_back(std::move(ext));
		}

	std::vector<zenith::util::zfile_format::zImgDescription> res;
	res.reserve(srcs.size());
	for (uint32_t i = 0; i < srcs.size(); i++)
	{
		auto r = srcs[i].first.get();
		res.push_back(zenith::util::zfile_format::zimg_from_img(r.data, r.size, formats[i].c_str()));
	}
	return res;
}


zenith::util::zfile_format::zImgDescription recombineImgs(const std::vector<NamedParameter> &params, zenith::util::zfile_format::zImgDescription * zimg, std::vector<zenith::util::zfile_format::zImgDescription> &imgs)
{
	if (zimg == nullptr && imgs.size() == 1)
	{
		zenith::util::zfile_format::zImgDescription res = imgs[0];
		imgs.clear();
		return res;
	}
	std::cout << "Unsupported images configuration.";
	throw std::runtime_error("Unsupported images configuration.");
}

zenith::util::zfile_format::zImgDescription performOperations(const std::vector<NamedParameter> &params, zenith::util::zfile_format::zImgDescription &zimg)
{
	zenith::util::zfile_format::zImgDescription tmp = zimg, res;
	for (const auto &p : params)
	{
		if (p.paramName == "dstFmt")
		{			
			zenith::util::zfile_format::ImageFormat dstFmt = zenith::util::zfile_format::str2ImageFormat(p.paramValue.c_str());

			res = zenith::util::zfile_format::zimg_change_format(tmp, dstFmt);

			zenith::util::zfile_format::zimg_free(tmp);
			tmp = res;
		}
	}
	return res;
}

void saveZIMG(const std::vector<NamedParameter> &params, const zenith::util::zfile_format::zImgDescription &zimg)
{
	std::vector<std::future<zenith::util::io::FileResult>> results;
	for (const auto &p : params)
		if (p.paramName == "out")
		{
			uint64_t fSize = zimg.getFullSize();
			uint8_t * data = new uint8_t[fSize];
			zenith::util::zfile_format::zimg_to_mem(zimg, data, fSize);

			auto f = fs->createFile();
			f.open(p.paramValue.c_str(), 'w', 1.0f);
			f.write(data, fSize);
			results.emplace_back(std::move(f.close()));
		}
	for (auto &p : results)
		p.get();
}

int main(int argc, const char * argv[])
{
	fs = new zenith::util::io::FileSystem(4);
	std::vector<NamedParameter> params = parseArguments(argc, argv);
	for (const auto &np : params)
		std::cout << np.paramName << " -> " << np.paramValue << "\n";

	auto src_imgs = loadSrcs(params);
	zenith::util::zfile_format::zImgDescription * src_zimg = nullptr;//loadZIMG(params);

	zenith::util::zfile_format::zImgDescription zimg = recombineImgs(params, src_zimg, src_imgs);
	zimg = performOperations(params, zimg);

	saveZIMG(params, zimg);

	zenith::util::zfile_format::zimg_free(zimg);

	return 0;
}