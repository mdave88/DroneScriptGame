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
	char* buffer = utils::file::readFile(filename.c_str());
	GX_ASSERT(buffer && "Error: resources cannot be loaded from constants.json");

	rapidjson::Document d;
	d.ParseInsitu(buffer);
	parseObject(d);

	delete[] buffer;

	return true;
}

void ConstantManager::parseObject(const rapidjson::Value& object, const std::string& path)
{
	for(rapidjson::Value::ConstMemberIterator m = object.MemberBegin(); m != object.MemberEnd(); ++m)
	{
		const char* name = path.empty() ? m->name.GetString() : utils::conversion::formatStr("%s::%s", path.c_str(), m->name.GetString());

		if(m->value.IsObject())
		{
			parseObject(m->value, name);
		}
		else if(m->value.IsBool())
		{
			m_boolDirectory[name] = m->value.GetBool();
		}
		else if(m->value.IsInt())
		{
			m_intDirectory[name] = m->value.GetInt();
		}
		else if(m->value.IsDouble())
		{
			m_floatDirectory[name] = m->value.GetDouble();
		}
		else if(m->value.IsString())
		{
			m_stringDirectory[name] = std::string(m->value.GetString(), m->value.GetStringLength());
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
