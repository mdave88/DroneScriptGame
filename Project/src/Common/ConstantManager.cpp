#include "GameStdAfx.h"
#include "ConstantManager.h"

#include "Common/luamanager/LuaManager.h"

#include <RapidXml/rapidxml_print.hpp>
#include <RapidXml/rapidxml_utils.hpp>


using namespace rapidxml;
typedef std::vector<xml_node<>*> XmlNodeVector;

XmlNodeVector ConstantManager::parseXmlSubnodes(xml_node<>* xmlNode)
{
	XmlNodeVector subXmlNodes;

	xmlNode = xmlNode->first_node();
	while (xmlNode)
	{
		subXmlNodes.push_back(xmlNode);
		xmlNode = xmlNode->next_sibling();
	}

	return subXmlNodes;
}

void ConstantManager::parseNamespace(const std::string& path, xml_node<>* xmlNode)
{
	std::string type, name, value;

	//XmlNodeVector subXmlNodes = parseXmlSubnodes(xmlNode);
	//ForEach (XmlNodeVector, subXmlNodes, it)
	//{
	//	const std::string nodeName = (*it)->name();
	//	if (nodeName == "namespace")
	//	{
	//		name = (*it)->first_attribute("name")->value();
	//		parseNamespace(path + name + "::", *it);

	//		continue;
	//	}

	//	type = (*it)->first_attribute("type")->value();
	//	name = path + (*it)->first_attribute("name")->value();
	//	value = (*it)->first_attribute("value")->value();

	//	if (type == "string")
	//	{
	//		setStringConstant(name, value);
	//	}
	//	else if (type == "int")
	//	{
	//		setIntConstant(name, atoi(value.c_str()));
	//	}
	//	else if (type == "float")
	//	{
	//		setFloatConstant(name, atof(value.c_str()));
	//	}
	//	else if (type == "bool")
	//	{
	//		setBoolConstant(name, value == "true");
	//	}
	//	else if (type == "vec3")
	//	{
	//		setVectorConstant(name, helperfuncs::conversion::strToVec3(value));
	//	}
	//}
}

ConstantManager::ConstantManager()
{
	setStringConstant("dataDir", getDataDir());
}

ConstantManager::~ConstantManager()
{
}

/**
 * Loading the constants from the settings file.
 */
bool ConstantManager::loadConstants(const std::string& filename)
{
	char* buffer = helperfuncs::file::readFile(filename.c_str());
	GX_ASSERT(buffer && "Error: resources cannot be loaded from constants.xml");

	//std::string xmlData(buffer);
	//delete[] buffer;

	//std::vector<char> xmlCopy(xmlData.begin(), xmlData.end());
	//xmlCopy.push_back('\0');

	//xml_document<> doc;
	//doc.parse < parse_declaration_node | parse_no_data_nodes > (&xmlCopy[0]);
	//xml_node<>* rootXmlNode = doc.first_node("constants");

	//// parse the namespaces
	//XmlNodeVector subXmlNodes = parseXmlSubnodes(rootXmlNode);
	//ForEach (XmlNodeVector, subXmlNodes, it)
	//{
	//	const std::string namepaceName = (*it)->first_attribute("name")->value();
	//	parseNamespace(namepaceName + "::", *it);
	//}

	return true;
}


// register methods to lua
void ConstantManager::registerMethodsToLua()
{
	using namespace luabind;

	luabind::module(LuaManager::getInstance()->getState()) [
	    luabind::def("CONST_INT",	&CONST_INT),
	    luabind::def("CONST_BOOL",	&CONST_BOOL),
	    luabind::def("CONST_FLOAT",	&CONST_FLOAT),
	    luabind::def("CONST_STR",	&CONST_STR),
	    luabind::def("CONST_VEC3",	&CONST_VEC3)
	];
}


//inline void ConstantManager::setIntConstant(const std::string& name, int value) {
//	m_intDirectory[name] = value;
//}
//
//inline void ConstantManager::setFloatConstant(const std::string& name, float value) {
//	m_floatDirectory[name] = value;
//}
//
//inline void ConstantManager::setBoolConstant(const std::string& name, bool value) {
//	m_boolDirectory[name] = value;
//}
//
//inline void ConstantManager::setStringConstant(const std::string& name, const std::string& value) {
//	m_stringDirectory[name] = value;
//}
//
//inline void ConstantManager::setVectorConstant(const std::string& name, const vec3& value) {
//	m_vectorDirectory[name] = value;
//}
//
//inline int ConstantManager::getIntConstant(const std::string& name) {
//	return m_intDirectory[name];
//}
//
//inline bool ConstantManager::getBoolConstant(const std::string& name) {
//	return m_boolDirectory[name];
//}
//
//inline float ConstantManager::getFloatConstant(const std::string& name) {
//	return m_floatDirectory[name];
//}
//
//inline std::string& ConstantManager::getStringConstant(const std::string& name) {
//	return m_stringDirectory[name];
//}
//
//inline vec3 ConstantManager::getVectorConstant(const std::string& name) {
//	return m_vectorDirectory[name];
//}
//
//// short helper methods
//inline int CONST_INT(const std::string& name) {
//	return ConstantManager::getInstance()->getIntConstant(name);
//}
//
//inline bool CONST_BOOL(const std::string& name) {
//	return ConstantManager::getInstance()->getBoolConstant(name);
//}
//
//inline float CONST_FLOAT(const std::string& name) {
//	return ConstantManager::getInstance()->getFloatConstant(name);
//}
//
//inline vec3 CONST_VEC3(const std::string& name) {
//	return ConstantManager::getInstance()->getVectorConstant(name);
//}
//
//inline const std::string& CONST_STR(const std::string& name) {
//	return ConstantManager::getInstance()->getStringConstant(name);
//}
