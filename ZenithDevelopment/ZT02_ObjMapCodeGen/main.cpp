#include <iostream>
#include <Utils/object_map.h>
#include <Utils/xml_tools.h>
#include <locale>
#include <list>
#include <map>

using namespace zenith::util;

std::map<std::string, std::string> params;


std::string strPreprocess(const std::string &str)
{
	std::string res = str;
	size_t pos = 0;
	while ((pos = res.find("\\n")) != std::string::npos)
		res.replace(pos, 2, "\n");
	return res;
}
std::string strSubstitute(const std::string &str, const ObjectMap<char, char> &df)
{
	std::string res = str;
	size_t posStart = 0;
	std::locale gl;
	while ((posStart = res.find('#')) != std::string::npos)
	{
		size_t posEnd = posStart + 1;
		while (posEnd < res.size() && (std::isalnum(res[posEnd], gl) || (res[posEnd] == '_')))
			posEnd++;
		std::string name = res.substr(posStart + 1, posEnd - posStart - 1);
		std::string val = df.getValues(name.c_str(), ObjectMapPresence::ONE).first->second.first.c_str();
		res.replace(posStart, posEnd - posStart, val.c_str());
	}
	return res;
}
std::list<std::string> strSplit(std::string str, char split)
{
	size_t off = 0;
	size_t pos = 0;
	std::list<std::string> strs;
	while ((pos = str.find(split, off)) != std::string::npos)
	{
		strs.push_back(str.substr(off, pos - off));
		off = pos + 1;
	}
	if (off < str.size())
		strs.push_back(str.substr(off));
	return strs;
}

std::string genCode_output(const std::string &output, const ObjectMap<char, char> &codeDef, const ObjectMap<char, char> &codeGen)
{
	return strSubstitute(output, codeDef);
}
std::string genCode_foreach()
{
	return "";
}

const ObjectMap<char, char> &findMatchingFunction(const ObjectMap<char, char> &codeDef, const ObjectMap<char, char> &codeGen, const std::string &type, const std::string &fun)
{
	auto def_fun_rng = codeGen.getObjects(type.c_str(), ObjectMapPresence::ONE).first->second.getObjects(fun.c_str(), ObjectMapPresence::ONE_PLUS);
	auto def_fun_it = def_fun_rng.first;
	for (; def_fun_it != def_fun_rng.second; def_fun_it++)
	{
		auto rng = def_fun_it->second.getValues("require", ObjectMapPresence::ONE_OR_ZERO);
		std::list<std::string> cg_require, cg_restrict;
		if (rng.first != rng.second)
			cg_require = strSplit(rng.first->second.first.c_str(), ',');

		rng = def_fun_it->second.getValues("restrict", ObjectMapPresence::ONE_OR_ZERO);
		if (rng.first != rng.second)
			cg_restrict = strSplit(rng.first->second.first.c_str(), ',');

		bool reqMatched = true;
		for (auto req : cg_require)
			if (codeDef.getNumValues(req.c_str()) != 1)
				reqMatched = false;
		for (auto req : cg_restrict)
			if (codeDef.getNumValues(req.c_str()) != 0)
				reqMatched = false;

		if (!reqMatched)
			continue;

		rng = def_fun_it->second.getValues();
		for (auto i = rng.first; i != rng.second; i++)
		{
			std::list<std::string> checks = strSplit(i->first.c_str(), '_');
			if (checks.size() != 2)
				continue;
			if (checks.front() != std::string("chk"))
				continue;
			auto rng2 = codeDef.getValues(checks.back().c_str(), ObjectMapPresence::ONE_OR_ZERO);
			if (rng2.first == rng2.second)
			{
				reqMatched = false;
				break;
			}
			std::string actValue = rng2.first->second.first.c_str();
			std::string chkValue = i->second.first.c_str();
			if (actValue != chkValue)
			{
				reqMatched = false;
				break;
			}
		}

		if (!reqMatched)
			continue;
		break;
	}

	return def_fun_it->second;
}

bool checkStatement(const ObjectMap<char, char> &codeDef, const ObjectMap<char, char> &codeGen)
{
	auto rng = codeGen.getValues("require", ObjectMapPresence::ONE_OR_ZERO);
	std::list<std::string> cg_require, cg_restrict;
	if (rng.first != rng.second)
		cg_require = strSplit(rng.first->second.first.c_str(), ',');

	rng = codeGen.getValues("restrict", ObjectMapPresence::ONE_OR_ZERO);
	if (rng.first != rng.second)
		cg_restrict = strSplit(rng.first->second.first.c_str(), ',');

	bool reqMatched = true;
	for (auto req : cg_require)
		if (codeDef.getNumValues(req.c_str()) != 1)
			reqMatched = false;
	for (auto req : cg_restrict)
		if (codeDef.getNumValues(req.c_str()) != 0)
			reqMatched = false;

	if (!reqMatched)
		return false;

	rng = codeGen.getValues();
	for (auto i = rng.first; i != rng.second; i++)
	{
		std::list<std::string> checks = strSplit(i->first.c_str(), '_');
		if (checks.size() != 2)
			continue;
		if (checks.front() != std::string("chk"))
			continue;
		auto rng2 = codeDef.getValues(checks.back().c_str(), ObjectMapPresence::ONE_OR_ZERO);
		if (rng2.first == rng2.second)
		{
			reqMatched = false;
			break;
		}
		std::string actValue = rng2.first->second.first.c_str();
		std::string chkValue = i->second.first.c_str();
		if (actValue != chkValue)
		{
			reqMatched = false;
			break;
		}
	}

	if (!reqMatched)
		return false;
	return true;
}

std::string genCode(const ObjectMap<char, char> &codeDef, const ObjectMap<char, char> &codeGen, const std::string &type, const std::string &fun)
{
	std::string result = "";


	std::string name = codeDef.getValues("name", ObjectMapPresence::ONE).first->second.first.c_str();

	auto &def_fun = findMatchingFunction(codeDef, codeGen, type, fun);


	auto def_rng = def_fun.getObjects("action", ObjectMapPresence::ONE_PLUS);

	for (auto it = def_rng.first; it != def_rng.second; it++)
	{
		std::string cg_type = it->second.getValues("type", ObjectMapPresence::ONE).first->second.first.c_str();
		std::string cg_code = strPreprocess(it->second.getValues("code", ObjectMapPresence::ONE).first->second.first.c_str());

		if (!checkStatement(codeDef, it->second))
			continue;

		if (cg_type == "output")
		{
			result += genCode_output(cg_code, codeDef, codeGen);
		}
		else if (cg_type == "foreach")
		{
			std::string cg_node = strPreprocess(it->second.getValues("node", ObjectMapPresence::ONE).first->second.first.c_str());
			std::string cg_join = strPreprocess(it->second.getValues("join", ObjectMapPresence::ONE).first->second.first.c_str());
			std::string cg_fun = strPreprocess(it->second.getValues("fun", ObjectMapPresence::ONE).first->second.first.c_str());

			auto rng = codeDef.getObjects();
			if (cg_node != std::string("any"))
				rng = codeDef.getObjects(cg_node.c_str());

			for (auto i = rng.first; i != rng.second; i++)
			{
				if (i != rng.first)
					result += cg_join;
				result += genCode(i->second, codeGen, i->first.c_str(), cg_fun);
			}
		}

	}

	return result;
}

void init()
{
	params["PATH"] = "D:/Programming/Zenith/";
	params["RPATH_GEN"] = "Source/Utils/CodeGeneration/";
	params["RPATH_DEF"] = "Source/Utils/CodeGeneration/";
	params["CODE_GEN"] = "ObjectMap_CodeGen.xml";
}

void update_params(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "Expecting at least 1 argument: CODE-DEF filename!";
		throw std::runtime_error("Expecting at least 1 argument: CODE-DEF filename!");
	}
	params["CODE_DEF"] = argv[1];

	params["FILE:GEN"] = params["PATH"] + params["RPATH_GEN"] + params["CODE_GEN"];
	params["FILE:DEF"] = params["PATH"] + params["RPATH_DEF"] + params["CODE_DEF"];
}

int main(int argc, char **argv)
{
	init();
	update_params(argc, argv);

	pugi::xml_document codeGenXML, codeDefXML;
	codeGenXML.load_file(params["FILE:GEN"].c_str());
	codeDefXML.load_file(params["FILE:DEF"].c_str());

	ObjectMap<char, char> omCodeGen;

	xml::xml2objmap(codeGenXML.root().child("functions"), omCodeGen);

	std::string gOptions;
	auto gOptionsAttr = codeDefXML.root().child("root").attribute("astyle_options");
	if (!gOptionsAttr.empty())
		gOptions = gOptionsAttr.as_string();

	for (auto ch : codeDefXML.root().child("root").children("file"))
	{
		std::string fName = ch.attribute("name").as_string();

		auto chPrefix = ch.child("prefix");
		std::string prefix = "";
		if (!chPrefix.empty())
			prefix = chPrefix.child_value();

		auto chPostfix = ch.child("postfix");
		std::string postfix = "";
		if (!chPostfix.empty())
			postfix = chPostfix.child_value();

		std::string locOptions;
		auto locOptionsAttr = ch.attribute("astyle_options");
		if (!locOptionsAttr.empty())
			locOptions = locOptionsAttr.as_string();

		std::string options = (locOptions.empty() ? gOptions : locOptions);


		std::cout << "generating file: " << fName << std::endl;

		std::remove(fName.c_str());
		std::ofstream f(fName.c_str());
		f << prefix << "\n";
		for (auto chn : ch.children("namespace"))
		{
			ObjectMap<char, char> omCodeDef;
			xml::xml2objmap(ch.child("namespace"), omCodeDef);

			f << genCode(omCodeDef, omCodeGen, "namespace", "define") << "\n";
		}
		
		f << postfix;
		f.close();

		std::cout << "file (" << fName << ") generated." << std::endl;

		std::string cmd = std::string("astyle -v ") + options + " " + fName;
		std::system(cmd.c_str());
	}
	std::cout << "finished processing successfully!";
	return 0;
}