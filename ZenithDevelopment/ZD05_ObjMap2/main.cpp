#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
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
#include <Utils\dtrie.h>
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

	//auto it = pd.begin();
	char buff[32];
	for (auto it = pd.begin(); it != pd.end(); ++it)
	{
		//auto len = it.key_fill(buff, 32); buff[len] = 0;
		std::cout << it.key_string() << " -> " << *it << "\n";
	}
	for (auto &x : pd)
	{
		std::cout << " -> " << x << "\n";
	}
	/*
	while (it.valid())
	{
		auto len = it.key_fill(buff, 32);
		buff[len] = 0;
		std::cout << buff << " -> " << *it << "\n";
		it = it.next();
	}
	*/
}

void test_dtrie_id()
{
	zenith::util::dtrie_id<char> pd;

	//simple add
	pd.add("ab");
	pd.add("cd");
	pd.add("ef");
	std::cout << "a: " << pd.exists("a") << "\n";
	std::cout << "ab: " << pd.exists("ab") << "\n";
	std::cout << "abc: " << pd.exists("abc") << "\n";
	std::cout << "cd: " << pd.exists("cd") << "\n";
	std::cout << "ef: " << pd.exists("ef") << "\n";
	std::cout << "gh: " << pd.exists("gh") << "\n";



	//first structure
	pd.add("abc");
	pd.add("cde");
	pd.add("efg");

	std::cout << "a: " << pd.exists("a") << "\n";
	std::cout << "ab: " << pd.exists("ab") << "\n";
	std::cout << "abc: " << pd.exists("abc") << "\n";
	std::cout << "cd: " << pd.exists("cd") << "\n";
	std::cout << "cde: " << pd.exists("cde") << "\n";
	std::cout << "ef: " << pd.exists("ef") << "\n";
	std::cout << "efg: " << pd.exists("efg") << "\n";
	std::cout << "gh: " << pd.exists("gh") << "\n";
	
	pd.add("xyzw");
	pd.add("xywz");
	pd.add("xyyy");

	//split structure
	pd.add("a");
	pd.add("b");
	pd.add("c");
	//	pd.add("d", 14);
	pd.add("e");
	pd.add("f");
	pd.add("g");
	pd.add("h");


//	pd.debug_output();

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

	//pd.debug_output();
	//auto it = pd.begin();
	for (auto it = pd.begin(); it != pd.end(); ++it)
		std::cout << it.key() << " -> " << it.value() << "\n";
	
	for (auto x : pd)
	{
		std::cout << " -> " << x << "\n";
	}
	std::cout << zenith::util::dtrie_id<char>::NodeSize;
	
}


void test_dtrie_vec()
{
	zenith::util::dtrie_vec<int, char> pd;

	//simple add
	pd.add("ab", 71);
	pd.add("cd", 72);
	pd.add("ef", 73);

	//first structure
	pd.add("abc", 91);
	pd.add("cde", 92);
	pd.add("efg", 93);

	pd.add("xyzw", -1);
	pd.add("xywz", -2);
	pd.add("xyyy", -3);

	//split structure
	pd.add("a", 1);
	pd.add("b", 2);
	pd.add("c", 3);
	//	pd.add("d", 14);
	pd.add("e", 3);
	pd.add("f", 2);
	pd.add("g", 1);
	pd.add("h", 0);

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

	
	for (auto it = pd.begin(); it != pd.end(); ++it)
		std::cout << it.key() << " -> " << it.value() << "\n";
	
	for (auto &x : pd)
	{
		std::cout << x.first << " => " << x.second << "\n";
		x.second++;
		std::cout << x.first << " => " << x.second << "\n";
	}	
	std::cout << pd.size() << " " << pd.dtrie_capacity();
}

inline std::string genRndString()
{
	uint32_t len = 3 + (std::rand() % 5);
	std::string rnd(len, 'a');
	for (uint32_t j = 0; j < len; j++)
	{
		char v = 'a' + (std::rand() % 26);
		rnd[j] = v;
	}
	return rnd;
}

int main()
{
	//test_ioconv();
	//test_prefix_dag();
	//test_dtrie_vec();
	
	uint32_t NumInserted = 30000;
	uint32_t NumOut = 30000;

	std::unordered_map<std::string, int> map_vals;
	zenith::util::dtrie_vec<int, char> dtrie_vals(NumInserted, NumInserted * 20);

	uint32_t TestSize = 1000000;
	double pOut = 0.9;
	/*
	dtrie v map
	0.0: 347 v 540 - 35% speedup
	0.1: 326 v 582 | 323 v 535 - 40% speedup
	0.5: 327 v 440 | 249 v 484 - 25%-50% speedup
	0.9: 208 v 365 | 185 v 329 - 50% speedup

	dtrie v hash_map
	0.0: 349 v 108 | 3.2x slower
	0.1: 302 v 119 | 2.5x slower
	0.5: 237 v 84  | 2.8x slower
	0.9: 205 v 60  | 3.4x slower
	*/

	std::vector<std::string> inNames;
	std::vector<std::string> outNames;

	std::vector<std::string> testNames;
	
	std::cout << "Generating dictionaries..\n";
	inNames.reserve(NumInserted);
	uint32_t numCycles = 0;
	for (uint32_t i = 0; i < NumInserted; numCycles++)
	{
		auto rnd = genRndString();
		bool me = (map_vals.find(rnd) != map_vals.end());
		bool de = dtrie_vals.exists(rnd.c_str());
		if (me != de)
			std::cout << "ERR\n";
		if (de)
			continue;
		int r = (std::rand() % 101) - 50;
		dtrie_vals.add(rnd.c_str(), r);
		map_vals[rnd] = r;
		inNames.push_back(rnd);
		i++;
	}
	std::cout << numCycles << " cycles needed.\n";
	std::cout << "Generating external keys..\n";
	outNames.reserve(NumOut);
	numCycles = 0;
	for (uint32_t i = 0; i < NumOut; numCycles++)
	{
		auto rnd = genRndString();
		bool me = (map_vals.find(rnd) != map_vals.end());
		bool de = dtrie_vals.exists(rnd.c_str());
		if (me != de)
			std::cout << "ERR\n";
		if (de)
			continue;
		outNames.push_back(rnd);
		i++;
	}
	std::cout << numCycles << " cycles needed.\n";
	std::cout << "Combining test sample..\n";
	testNames.reserve(TestSize);
	uint32_t realIn = 0, realOut = 0;
	for (uint32_t i = 0; i < TestSize; i++)
	{
		double p = double(std::rand() % 10000) * 0.0001;
		if (p < pOut)
		{
			uint32_t idx = std::rand() % NumOut;
			testNames.push_back(outNames[idx]);
			realOut++;
		}
		else
		{
			uint32_t idx = std::rand() % NumInserted;
			testNames.push_back(inNames[idx]);
			realIn++;
		}
	}
	std::cout << "In: " << realIn << ", out: " << realOut << "\n";
	std::cout << "Preparation finished.\n\n";

	int sum1 = 0, sum2 = 0;
	std::cout << "Testing dtrie_vec..\n";

	auto dtrie_start = std::chrono::high_resolution_clock::now();
	for (uint32_t i = 0; i < TestSize; i++)
	{		
		if (!dtrie_vals.exists(testNames[i].c_str()))
			continue;
		sum1 += dtrie_vals.get(testNames[i].c_str());
	}
	auto dtrie_end = std::chrono::high_resolution_clock::now();


	std::cout << "Testing map..\n";

	auto map_start = std::chrono::high_resolution_clock::now();
	for (uint32_t i = 0; i < TestSize; i++)
	{
		if (map_vals.find(testNames[i]) == map_vals.end())
			continue;
		sum2 += map_vals.at(testNames[i]);
	}
	auto map_end = std::chrono::high_resolution_clock::now();

	std::cout << "Finished testing.\n";
	std::cout << "Sum by dtrie_vec = " << sum1 << ", by map = " << sum2 << ".\n";
	double dtrie_time = std::chrono::duration_cast<std::chrono::milliseconds>(dtrie_end - dtrie_start).count();
	double map_time = std::chrono::duration_cast<std::chrono::milliseconds>(map_end - map_start).count();
	std::cout << "Time by dtrie_vec = " << dtrie_time << "\n";
	std::cout << "Time by map = " << map_time << "\n";

	std::cout << "\nClosing...";
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	return 0;
}