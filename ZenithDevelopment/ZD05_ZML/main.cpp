#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <Utils\FileFormat\ff_zds.h>
#include <Utils\FileFormat\ff_zds_utils.h>
#include <cstdio>
#include <pugixml\pugixml.hpp>
#include <Utils\ioconv\input_xml.h>
#include <Utils\ioconv\input_zds.h>
#include <Utils\ioconv\output_xml.h>
#include <Utils\ioconv\output_zds.h>
#include <Utils\ioconv\io_alg.h>
#include <Utils\ioconv\input_alg.h>
#include <Utils\ioconv\output_alg.h>

using namespace zenith::util::zfile_format;

void test1()
{
	zDataStorage tst("root");

	auto it = tst.root();
	{
		auto it1 = it.append("test1", 4);
		auto it2 = it.append("test2", 8);
		auto it3 = it2.append("test1", 16);

		it1.write<int>(-7);
		it2.write<double>(3.0);
		memcpy_s(it3.data(), 16, "DATA", 5);

		int v1; it1.read(v1);
		double v2; it2.read(v2);
	}
	tst.optimize();
	it = tst.root();
	{
		for (auto i = it.child_begin(); i != it.child_end(); ++i)
		{
			std::cout << "tag=<" << i.tag_name() << "> size=" << i.size() << " value=";
			if (i.size() == 4)
				std::cout << i.value<int>();
			else
				std::cout << i.value<double>();
			std::cout << "\n";
		}

	}
	zDataStorageDescription descr = tst.descr();
	uint64_t fsz = descr.getFullSize();
	std::unique_ptr<uint8_t[]> data(new uint8_t[fsz]);

	zds_to_mem(descr, data.get(), fsz);

	FILE * f;
	fopen_s(&f, "test1.zds", "wb");
	fwrite(data.get(), 1, fsz, f);
	fclose(f);
}

void test2()
{
	FILE * f;
	fopen_s(&f, "test1.zds", "rb");
	fseek(f, 0, SEEK_END);
	uint64_t fsz = ftell(f);
	fseek(f, 0, SEEK_SET);
	std::unique_ptr<uint8_t[]> data(new uint8_t[fsz]);
	fread(data.get(), 1, fsz, f);
	fclose(f);

	zDataStorageDescription descr = zds_from_mem(data.get(), fsz);
	zDataStorage tst(descr, zDataStorage::DataOwnership::NO_OWNERSHIP);
	auto it = tst.root();
	{
		for (auto i = it.child_begin(); i != it.child_end(); ++i)
		{
			std::cout << "tag=<" << i.tag_name() << "> size=" << i.size() << " value=";
			if (i.size() == 4)
				std::cout << i.value<int>();
			else
				std::cout << i.value<double>();
			std::cout << "\n";
		}

	}
}

void test3()
{
	pugi::xml_document doc;
	doc.load_file("ZML_Test.xml");
	auto root_xml = *doc.root().children().begin();
	zDataStorage res(root_xml.name());
	auto root_zds = res.root();

	zenith::util::ioconv::io_transform(zenith::util::ioconv::input_xml_root(root_xml), zenith::util::ioconv::output_zds(root_zds));

	res.optimize();
	auto resd = res.descr();
	uint64_t fsz = resd.getFullSize();
	std::unique_ptr<uint8_t[]> data(new uint8_t[fsz]);
	zds_to_mem(resd, data.get(), fsz);

	FILE * f;
	fopen_s(&f, "ZML_Test.zds", "wb");
	fwrite(data.get(), 1, fsz, f);
	fclose(f);
}
void test4()
{
	FILE * f;
	fopen_s(&f, "ZML_Test.zds", "rb");
	fseek(f, 0, SEEK_END);
	uint64_t fsz = ftell(f);
	fseek(f, 0, SEEK_SET);
	std::unique_ptr<uint8_t[]> data(new uint8_t[fsz]);
	fread(data.get(), 1, fsz, f);
	fclose(f);

	zDataStorageDescription descr = zds_from_mem(data.get(), fsz);
	zDataStorage tst(descr, zDataStorage::DataOwnership::NO_OWNERSHIP);

	pugi::xml_document doc;
	auto xml_it = doc.root().append_child(tst.root().tag_name());
	zDataStorage::const_iterator root_iter = tst.root();
	zenith::util::ioconv::input_zds_root in_it(root_iter);
	zenith::util::ioconv::io_transform(in_it, zenith::util::ioconv::output_xml(xml_it));

	doc.save_file("ZML_Test_back.xml");
}

struct Test
{
	uint64_t value_uint;
	int64_t value_int;
	char value_char[16];
	double value_double;
};

struct Test2
{
	uint64_t value_uint;
	int64_t value_int;
	char value_char[16];
	double value_double;
};

template<class It, zenith::util::ioconv::InternalType intType>
class zenith::util::ioconv::io_handler_impl<Test, It, intType>
{
public:
	typedef Test value_type;
	static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
	inline static void input(Test &val, const It &it)
	{
		zenith::util::ioconv::input_named_required(val.value_char, it, "value_char");
		zenith::util::ioconv::input_named_required(val.value_int, it, "value_int");
		zenith::util::ioconv::input_named_required(val.value_uint, it, "value_uint");
		zenith::util::ioconv::input_named_required(val.value_double, it, "value_double");
	}
	inline static void output(const Test &val, It &it)
	{
		zenith::util::ioconv::output_single(val.value_char, it.append_value("value_char"));
		zenith::util::ioconv::output_single(val.value_int, it.append_value("value_int"));
		zenith::util::ioconv::output_single(val.value_uint, it.append_value("value_uint"));
		zenith::util::ioconv::output_single(val.value_double, it.append_value("value_double"));
	}
};

template<class It>
class zenith::util::ioconv::io_handler_impl<Test2, It, zenith::util::ioconv::InternalType::STRING>
{
public:
	typedef Test2 value_type;
	static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
	inline static void input(Test2 &val, const It &it)
	{
		zenith::util::ioconv::input_named_required(val.value_char, it, "value_char");
		zenith::util::ioconv::input_named_required(val.value_int, it, "value_int");
		zenith::util::ioconv::input_named_required(val.value_uint, it, "value_uint");
		zenith::util::ioconv::input_named_required(val.value_double, it, "value_double");
	}
	inline static void output(const Test2 &val, It &it)
	{
		zenith::util::ioconv::output_single(val.value_char, it.append_value("value_char"));
		zenith::util::ioconv::output_single(val.value_int, it.append_value("value_int"));
		zenith::util::ioconv::output_single(val.value_uint, it.append_value("value_uint"));
		zenith::util::ioconv::output_single(val.value_double, it.append_value("value_double"));
	}
};


template<class It>
class zenith::util::ioconv::io_handler_impl<Test2, It, zenith::util::ioconv::InternalType::BINARY>
{
public:
	typedef Test2 value_type;
	static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::VALUE;
	inline static void input(Test2 &val, const It &it)
	{
		it.read(val);
	}
	inline static void output(const Test2 &val, It &it)
	{
		it.as_value().reset_size(sizeof(Test2));
		it.as_value().write(val);
	}
};

template<class T>
void test_output_xml(const T &t, const char * fname)
{
	pugi::xml_document doc;
	auto xml_out = zenith::util::ioconv::output_xml(doc.root().append_child("File"));
	zenith::util::ioconv::output_single(t, xml_out);
	doc.save_file(fname);
}
template<class T>
void test_input_xml(T &t, const char * fname)
{
	pugi::xml_document doc;
	doc.load_file(fname);
	auto n = *doc.root().children("File").begin();
	zenith::util::ioconv::input(t, zenith::util::ioconv::input_xml_root(n));
}
template<class T>
void test_output_zds(const T &t, const char * fname)
{
	zDataStorage zds("File");
	auto zds_out = zenith::util::ioconv::output_zds(zds.root());
	zenith::util::ioconv::output_single(t, zds_out);
	zds.optimize();
	auto resd = zds.descr();
	uint64_t fsz = resd.getFullSize();
	std::unique_ptr<uint8_t[]> data(new uint8_t[fsz]);
	zds_to_mem(resd, data.get(), fsz);
	FILE * f;
	fopen_s(&f, fname, "wb");
	fwrite(data.get(), 1, fsz, f);
	fclose(f);
}

template<class T>
void test_input_zds(T &t, const char * fname)
{
	FILE * f;
	fopen_s(&f, fname, "rb");
	fseek(f, 0, SEEK_END);
	uint64_t fsz = ftell(f);
	fseek(f, 0, SEEK_SET);
	std::unique_ptr<uint8_t[]> data(new uint8_t[fsz]);
	fread(data.get(), 1, fsz, f);
	fclose(f);

	zDataStorageDescription descr = zds_from_mem(data.get(), fsz);
	zDataStorage tst(descr, zDataStorage::DataOwnership::NO_OWNERSHIP);

	
	zenith::util::ioconv::input(t, zenith::util::ioconv::input_zds_root(tst.root()));
}

void test_t1()
{
	Test t1;
	strcpy_s(t1.value_char, "TEST-CHAR");
	t1.value_int = -128;
	t1.value_uint = 1024;
	t1.value_double = 2.718281828459045;

	Test2 t2;
	memcpy_s(&t2, sizeof(t2), &t1, sizeof(t1));

	test_output_xml(t1, "t1.xml");
	test_output_xml(t2, "t2.xml");

	test_output_zds(t1, "t1.zds");
	test_output_zds(t2, "t2.zds");

}

int main()
{
	//test1();
	//test2();
	/*
	FILE * f;
	fopen_s(&f, "test1.zds", "rb");
	fseek(f, 0, SEEK_END);
	uint64_t fsz = ftell(f);
	fseek(f, 0, SEEK_SET);
	std::unique_ptr<uint8_t[]> data(new uint8_t[fsz]);
	fread(data.get(), 1, fsz, f);
	fclose(f);

	zDataStorageDescription descr = zds_from_mem(data.get(), fsz);
	zDataStorage tst(descr, zDataStorage::DataOwnership::NO_OWNERSHIP);
	pugi::xml_document doc;
	copy_all(doc, tst);
	doc.save_file("test1.xml");
	*/
	
	Test t1, t1x;
	Test2 t2, t2x;

	test_input_zds(t1, "t1.zds");
	test_input_zds(t2, "t2.zds");

	test_input_xml(t1x, "t1.xml");
	test_input_xml(t2x, "t2.xml");

	test_output_xml(t1, "t1_zds.xml");
	test_output_xml(t2, "t2_zds.xml");
	
	std::cout << "Resources freed.\n";
	std::cout << "Closing console in 0.5 second.\n";

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	return 0;
}