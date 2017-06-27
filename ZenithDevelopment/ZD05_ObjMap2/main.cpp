#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <chrono>
#include <thread>
#include <pugixml\pugixml.hpp>

#include <Utils\object_map.h>
#include <Utils\xml_tools.h>

#include <Utils\ioconv\input_xml.h>
#include <Utils\ioconv\input_objmap.h>

#include <Utils\ioconv\output_objmap.h>
#include <Utils\ioconv\output_xml.h>

#include <Utils\ioconv\input_alg.h>
#include <Utils\ioconv\output_alg.h>

#include <Utils\ioconv\io_handlers.h>
#include <Utils\ioconv\io_alg.h>

#include <Utils\prefix_dag.h>
//using namespace zenith::util::ioconv;

struct TestPoint
{
	double x, y;
};

struct TestPolygon
{
	std::list<int> indices;
	std::map<std::string, TestPoint> points;
};

template<>
class zenith::util::ioconv::io_handler<TestPoint>
{
public:
	typedef TestPoint value_type;
	static const NodeType node_type = NodeType::COMPLEX; //value by default

	template<class It>
	inline static void input(TestPoint &val, const It &it)
	{
		zenith::util::ioconv::input_named_required(val.x, it, "x");
		zenith::util::ioconv::input_named_required(val.y, it, "y");
	}
	template<class It>
	inline static void output(const TestPoint &val, It &it)
	{
		zenith::util::ioconv::output_single(val.x, it.append_value("x"));
		zenith::util::ioconv::output_single(val.y, it.append_value("y"));
	}
};
template<>
class zenith::util::ioconv::io_handler<TestPolygon>
{
public:
	typedef TestPolygon value_type;
	static const NodeType node_type = NodeType::COMPLEX; //value by default

	template<class It>
	inline static void input(TestPolygon &val, const It &it)
	{
		zenith::util::ioconv::input_named_multiple(val.indices, it, "index");
		zenith::util::ioconv::input_named_multiple_map(val.points, it, "point", "name");
	}
	template<class It>
	inline static void output(const TestPolygon &val, It &it)
	{
		zenith::util::ioconv::output_multiple(val.indices, it, "index");
		zenith::util::ioconv::output_multiple_map(val.points, it, "point", "key");
	}
};
/*
template<class It>
void readTestPoint_SF(It &&it, TestPoint &obj)
{
	auto p = it.children();
	for (auto i = p.first; i != p.second; ++i)
	{
		if (strcmp(i.key(), "x") == 0)
			obj.x = zenith::util::str_cast<double>(i.value());
		else if (strcmp(i.key(), "y") == 0)
			obj.y = zenith::util::str_cast<double>(i.value());
	}
}
*/

void test_ioconv()
{
	pugi::xml_document doc;
	doc.load_file("test.xml");

	auto nTest = doc.root();

	zenith::util::ObjectMap<char, char> objMap;
	zenith::util::xml::xml2objmap(nTest, objMap);

	zenith::util::ioconv::input_objmap_unnamed om_it0(objMap);
	zenith::util::ioconv::input_xml_unnamed xml_it0(nTest);

	pugi::xml_document out_doc1, out_doc2, out_doc3, out_doc0;
	zenith::util::ObjectMap<char, char> outObjMap;
	zenith::util::ioconv::output_xml out_xml0(out_doc1.root());
	zenith::util::ioconv::output_xml out_xml_verb0(out_doc3.root());
	zenith::util::ioconv::output_objmap out_om0(outObjMap);

	zenith::util::ioconv::io_transform(om_it0, out_xml0.append_complex("FULL"));

	zenith::util::ioconv::io_transform(xml_it0, out_om0.append_complex("FULL"));

	zenith::util::ioconv::io_transform(xml_it0, out_xml_verb0.append_complex("FULL"));


	zenith::util::xml::objmap2xml(outObjMap, out_doc2.root());
	out_doc1.save_file("test_out1.xml");
	out_doc2.save_file("test_out2.xml");
	out_doc2.save_file("test_out3.xml");
	auto it0 = xml_it0;

	//std::map<std::string, TestPoint> mtp;
	TestPolygon poly;
	zenith::util::ioconv::input_named_required(poly, it0, "Poly");

	zenith::util::ioconv::output_xml out_xml_it(out_doc0.root());

	zenith::util::ioconv::output_single(poly, out_xml_it.append_complex("Poly"));
	//zenith::util::ioconv::input_named_multiple_map(mtp, it0, "Point", "name");

	out_doc0.save_file("test_out.xml");
	TestPoint pt;
	//readTestPoint_DF(input_named_single(it0, "Point"), pt);

	std::cout << "x";
	/*
	auto p0 = it0.children("a");

	auto p = input_named_multiple(std::move(it0), "a", PresenceType::ONE_PLUS);
	for (auto i = p.first; i != p.second; ++i)
	std::cout << i.key() << ": " << i.value() << "\n";
	*/
	/*
	auto it = named(it0);
	for(; !it.empty(); ++it)
	std::cout << it.key() << ": " << (it.type() == NodeType::VALUE ? it.value() : "<complex>") << "\n";
	*/
	//input_xml_wrap_named_ it(nTest.children("a").begin(), nTest.children("a").end());
	/*
	input_xml_wrap_unnamed_ it(nTest.begin(), nTest.end());
	while (1)
	{
	std::cout << it.key() << ": " << (it.is_node_value() ? it.value() : "<complex>") << "\n";
	it = it.next();
	if (it == input_xml_wrap_unnamed_::end())
	break;
	}
	*/
	/*
	pugi::xml_document out_doc1, out_doc2;
	zenith::util::ObjectMap<char, char> outObjMap;
	output_xml out_xml0(out_doc1.root());
	output_objmap out_om0(outObjMap);

	auto out_it0 = out_om0;
	auto out_it = out_it0.append_complex("OldPoint");
	out_it.append_value("x", zenith::util::str_cast<std::string>(pt.x).c_str());
	out_it.append_value("y", zenith::util::str_cast<std::string>(pt.y).c_str());
	out_it.append_value("name", pt.name.c_str(), NodeValueHint::ATTRIBUTE);

	zenith::util::xml::objmap2xml(outObjMap, out_doc2.root());
	//out_doc1.save_file("test_out1.xml");
	out_doc2.save_file("test_out2.xml");
	*/
}

void test_prefix_dag()
{
	zenith::util::prefix_dag<char, int, 64> pd;

	//simple add
	pd.add("ab", 7);
	pd.add("cd", 8);
	pd.add("ef", 9);
	std::cout << "a: " << pd.exists("a") << "\n";
	std::cout << "ab: " << pd.exists("ab") << "\n";
	std::cout << "abc: " << pd.exists("abc") << "\n";
	std::cout << "cd: " << pd.exists("cd") << "\n";
	std::cout << "ef: " << pd.exists("ef") << "\n";
	std::cout << "gh: " << pd.exists("gh") << "\n";

	//first structure
	pd.add("abc", 72);
	pd.add("cde", 82);
	pd.add("efg", 92);

	std::cout << "a: " << pd.exists("a") << "\n";
	std::cout << "ab: " << pd.exists("ab") << "\n";
	std::cout << "abc: " << pd.exists("abc") << "\n";
	std::cout << "cd: " << pd.exists("cd") << "\n";
	std::cout << "cde: " << pd.exists("cde") << "\n";
	std::cout << "ef: " << pd.exists("ef") << "\n";
	std::cout << "efg: " << pd.exists("efg") << "\n";
	std::cout << "gh: " << pd.exists("gh") << "\n";

	//split structure
	pd.add("a", 11);
	pd.add("b", 12);
	pd.add("c", 13);
//	pd.add("d", 14);
	pd.add("e", 15);
	pd.add("f", 16);
	pd.add("g", 17);
	pd.add("h", 18);

	std::cout << "a: " << pd.exists("a") << "\n";
	std::cout << "ab: " << pd.exists("ab") << "\n";
	std::cout << "abc: " << pd.exists("abc") << "\n";
	std::cout << "cd: " << pd.exists("cd") << "\n";
	std::cout << "cde: " << pd.exists("cde") << "\n";
	std::cout << "ef: " << pd.exists("ef") << "\n";
	std::cout << "efg: " << pd.exists("efg") << "\n";
	std::cout << "gh: " << pd.exists("gh") << "\n";
	std::cout << "b: " << pd.exists("b") << "\n";
	std::cout << "c: " << pd.exists("c") << "\n";
	std::cout << "d: " << pd.exists("d") << "\n";
	std::cout << "e: " << pd.exists("e") << "\n";
	std::cout << "f: " << pd.exists("f") << "\n";
	std::cout << "g: " << pd.exists("g") << "\n";
	std::cout << "h: " << pd.exists("h") << "\n";


	std::cout << "a: " << pd.get("a") << "\n";
	std::cout << "ab: " << pd.get("ab") << "\n";
	std::cout << "abc: " << pd.get("abc") << "\n";
	std::cout << "cd: " << pd.get("cd") << "\n";
	std::cout << "cde: " << pd.get("cde") << "\n";
	std::cout << "ef: " << pd.get("ef") << "\n";
	std::cout << "efg: " << pd.get("efg") << "\n";
//	std::cout << "gh: " << pd.get("gh") << "\n";
	std::cout << "b: " << pd.get("b") << "\n";
	std::cout << "c: " << pd.get("c") << "\n";
//	std::cout << "d: " << pd.get("d") << "\n";
	std::cout << "e: " << pd.get("e") << "\n";
	std::cout << "f: " << pd.get("f") << "\n";
	std::cout << "g: " << pd.get("g") << "\n";
	std::cout << "h: " << pd.get("h") << "\n";
}

int main()
{
	//test_ioconv();
	test_prefix_dag();
	
	std::cout << "\nClosing...";
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	return 0;
}