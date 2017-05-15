#pragma once
#include <Utils\IO\filesystem.h>
#include <Utils\FileFormat\ff_img.h>
#include <string>


struct ProgramArguments
{
	std::string fontImage;
	std::string fontImageFormat;

	std::string fontDescription;
	std::string fontDescriptionFormat;
};

zenith::util::zfile_format::zImgDescription loadImage(const char * fname, const char * fformat);
void freeImage(zenith::util::zfile_format::zImgDescription &img);

zenith::util::zfile_format::zImgDescription loadDescription(const char * fname, const char * fformat);
void freeImage(zenith::util::zfile_format::zImgDescription &img);