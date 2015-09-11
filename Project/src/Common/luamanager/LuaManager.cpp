#include "GameStdAfx.h"
#include "Common/luamanager/LuaManager.h"

#include "GameLogic/Entity.h"

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
	createTable("unitTable");
	createTable("playerTable");
	createTable("itemTable");

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


/**
 * Creates a lua table and fills it with the given array of entities.
 *
 * @param tableName		The name of the new table.
 * @param entities		The contents of the new table.
 * @param numEntities	The size of the array "entities".
 */
void LuaManager::createTable(const std::string& tableName, const Entity* entities[], const int numEntities)
{
	luabind::object table = luabind::newtable(m_state);
	luabind::globals(m_state)[tableName.c_str()] = table;
	luabind::globals(m_state)[tableName + "Size"] = numEntities;

	if (entities)
	{
		for (int i = 0; i < numEntities; i++)
		{
			table[i + 1] = entities[i];
			//setTableElement(tableName, i + 1, entities[i]);

			GameConsole::addKeywordToConsole(entities[i]->getName());
		}
	}

	GameConsole::addKeywordToConsole(tableName);
}

/**
 * Creates a lua table and fills it with the given collection of entities.
 *
 * @param tableName	The name of the new table.
 * @param entities	The contents of the new table.
 */
void LuaManager::createTable(const std::string& tableName, const std::set<Entity*>& entities)
{
	luabind::object table = luabind::newtable(m_state);
	luabind::globals(m_state)[tableName] = table;
	luabind::globals(m_state)[tableName + "Size"] = entities.size();

	int i = 1;
	for (const Entity* entity : entities)
	{
		table[i] = *entity;
		i++;

		GameConsole::addKeywordToConsole(entity->getName());
	}

	GameConsole::addKeywordToConsole(tableName);
}

/**
 * Registers the given entity on the lua side and puts it into the appropriate table.
 * The entityTable holds every entity. The unitTable hold only the units...
 *
 * @param entity	The entity to be registered on the lua side.
 */
int LuaManager::registerEntity(const Entity& entity, const std::string& tableName)
{
	const int id = luabind::object_cast<int>( luabind::globals(m_state)["entityTableSize"] );
	luabind::globals(m_state)["entityTableSize"] = id + 1;
	luabind::globals(m_state)["entityTable"] [ entity.getName() ] = entity;

	if (!tableName.empty())
	{
		registerActorToTable(tableName, entity, entity.getName());
	}


	GameConsole::addKeywordToConsole(entity.getName());

	return id;
}

/**
 * Registers the given entity on the lua side and puts it into the appropriate table.
 * The entityTable holds every entity. The unitTable hold only the units...
 *
 * @param entity	The entity to be registered on the lua side.
 */
int LuaManager::registerEntitySP(const EntityPtr& entity, const std::string& tableName)
{
	const int entityTableSize = luabind::object_cast<int>( luabind::globals(m_state)["entityTableSize"] );
	luabind::globals(m_state)["entityTableSize"] = entityTableSize + 1;
	luabind::globals(m_state)["entityTable"] [ entity->getName() ] = entity;

	if (!tableName.empty())
	{
		registerActorToTable(tableName, entity, entity->getName());
	}


	GameConsole::addKeywordToConsole(entity->getName());

	return entityTableSize + 1;
}

/**
 * Unregisters the given entity on the lua side and removes it from the appropriate table.
 * The entityTable holds every entity. The unitTable hold only the units...
 *
 * @param entity	The entity to be removed from the lua side.
 */
void LuaManager::unregisterEntity(const Entity& entity, const std::string& tableName)
{
	const int id = luabind::object_cast<int>( luabind::globals(m_state)["entityTableSize"] );
	luabind::globals(m_state)["entityTableSize"] = id - 1;
	luabind::globals(m_state)["entityTable"] [ entity.getName() ] = luabind::nil;

	if (!tableName.empty())
	{
		unregisterActorFromTable(tableName, entity.getName());
	}
}

/**
 * Unregisters the given entity on the lua side and removes it from the appropriate table.
 * The entityTable holds every entity. The unitTable hold only the units...
 *
 * @param entity	The entity to be removed from the lua side.
 */
void LuaManager::unregisterEntitySP(const EntityPtr& entity, const std::string& tableName)
{
	const int entityTableSize = luabind::object_cast<int>( luabind::globals(m_state)["entityTableSize"] );
	luabind::globals(m_state)["entityTableSize"] = entityTableSize - 1;
	luabind::globals(m_state)["entityTable"] [ entity->getName() ] = luabind::nil;

	if (!tableName.empty())
	{
		unregisterActorFromTable(tableName, entity->getName());
	}
}

bool LuaManager::functionExist(const std::string& functionName)
{
	using namespace luabind;

	const object g = globals(m_state);
	const object func = g[ functionName.c_str() ];

	return (func && type(func) == LUA_TFUNCTION);
}
