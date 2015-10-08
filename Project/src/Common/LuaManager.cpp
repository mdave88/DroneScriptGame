#include "GameStdAfx.h"
#include "Common/LuaManager.h"
#include "Common/LoggerSystem.h"

LuaManager::LuaManager()
	: m_state(nullptr)
	, m_isOpened(false)
{
}

/**
 * Initializes the lua state and creates the global tables (entityTable, unitTable, itemTable, etc.).
 */
void LuaManager::init()
{
	//luabind::set_pcall_callback(LuaManager::handleError);

	if (m_isOpened)
	{
		close();
	}

	m_state = luaL_newstate();
	luaL_openlibs(m_state);
	luabind::open(m_state);

	createTable("entityTable");

	m_isOpened = true;
}

void LuaManager::close()
{
	lua_close(m_state);
	m_isOpened = false;
}

/**
 * Runs the given lua file.
 *
 * @param file	The file to be run.
 */
void LuaManager::doFile(const std::string& file)
{
	//int luaError = luaL_loadfile(m_state, file.c_str());
	luaL_loadfile(m_state, file.c_str());

	try
	{
		luabind::object compiledScript(luabind::from_stack(m_state, -1));
		luabind::call_function<void>(compiledScript);
	}
	catch (const luabind::error& e)
	{
		printError(e);
	}
}

/**
 * Adds the given command to the lua interpreter.
 *
 * @param command	The command to be run.
 */
void LuaManager::doString(const std::string& command)
{
	//int luaError = luaL_loadstring(m_state, command.c_str());
	luaL_loadstring(m_state, command.c_str());

	try
	{
		luabind::object compiledScript(luabind::from_stack(m_state, -1));
		//if (compiledScript.is_valid())
		luabind::call_function<void>(compiledScript);
	}
	catch (const luabind::error& e)
	{
		printError(e);
	}
}

void LuaManager::createTable(const std::string & tableName)
{
	luabind::object table = luabind::newtable(m_state);
	luabind::globals(m_state)[tableName] = table;
	luabind::globals(m_state)[tableName + "Size"] = 0;

#ifdef CLIENT_SIDE
	GameConsole::addKeywordToConsole(tableName);
#endif
}

int LuaManager::handleError(lua_State* state)
{
	lua_Debug d;
	lua_getstack(state, 1, &d);
	lua_getinfo(state, "Sln", &d);
	std::string err = lua_tostring(state, -1);
	lua_pop(state, 1);
	std::stringstream msg;
	msg << d.short_src << ":" << d.currentline;

	if (d.name != 0)
	{
		msg << "(" << d.namewhat << " " << d.name << ")";
	}
	msg << " " << err;
	lua_pushstring(state, msg.str().c_str());

	QLOG("Lua Error: " << msg.str().c_str());
	TRACE_ERROR("Lua Error: " << msg.str().c_str(), 0);

	return 1;
}

void LuaManager::printError(const luabind::error& e)
{
	const char* stateErrorString = lua_tostring(m_state, -1);

	GX_ASSERT(0 && "Lua Error: %s: %s", e.what(), stateErrorString);

	TRACE_ERROR("Lua Error: " << e.what() << ":", 0);
	TRACE_ERROR(stateErrorString, 0);
}

lua_State* LuaManager::getState()
{
	return m_state;
}

bool LuaManager::functionExist(const std::string& functionName)
{
	using namespace luabind;

	const object g = globals(m_state);
	const object func = g[ functionName.c_str() ];

	return (func && type(func) == LUA_TFUNCTION);
}


void addKeywordToConsole(const std::string& keyword)
{
#ifdef CLIENT_SIDE
	GameConsole::addKeywordToConsole(keyword);
#endif
}
