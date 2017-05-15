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

ProgramArguments parseArguments(int argc, char *argv[])
{
	/*hard-coded for debug purposes*/
	ProgramArguments pa;
	pa.fontImage = "testFont.png";
	pa.fontImageFormat = "PNG";

	pa.fontDescription = "testFont.xml";
	pa.fontDescriptionFormat = "XML";
}

int main(int argc, char *argv[])
{
	//0. initialize
	fs = new zenith::util::io::FileSystem(2); //two streams for loading
	//1. load font image & font description
	////a) get paths of image & font
	ProgramArguments pArgs = parseArguments(argc, argv);

	if (pArgs.fontImageFormat == "ZFNT")
	{
		auto fntData = readFile(pArgs.fontImage.c_str());
	}
	else
	{
		auto imgData = readFile(pArgs.fontImage.c_str());
		auto dscData = readFile(pArgs.fontDescription.c_str());
	}


}