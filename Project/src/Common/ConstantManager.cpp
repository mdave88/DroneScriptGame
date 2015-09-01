#include "GameStdAfx.h"
#include "ConstantManager.h"

#include "Common/luamanager/LuaManager.h"


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
	GX_ASSERT(buffer && "Error: resources cannot be loaded from constants.json");

	rapidjson::Document d;
	d.ParseInsitu(buffer);

	const rapidjson::Value& value = d;
	parseObject(value);


	delete[] buffer;

	return true;
}

void ConstantManager::parseObject(const rapidjson::Value& object)
{
	for(rapidjson::Value::ConstMemberIterator m = object.MemberBegin(); m != object.MemberEnd(); ++m)
	{
		if(m->value.IsObject())
		{
			parseObject(m->value);
		}
		else if(m->value.IsBool())
		{
			m_boolDirectory[m->name.GetString()] = m->value.GetBool();
		}
		else if(m->value.IsInt())
		{
			m_intDirectory[m->name.GetString()] = m->value.GetInt();
		}
		else if(m->value.IsDouble())
		{
			m_floatDirectory[m->name.GetString()] = m->value.GetDouble();
		}
		else if(m->value.IsString())
		{
			m_stringDirectory[m->name.GetString()] = std::string(m->value.GetString(), m->value.GetStringLength());
		}
		else
		{
			// TODO: handle vectors
		}
	}
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
