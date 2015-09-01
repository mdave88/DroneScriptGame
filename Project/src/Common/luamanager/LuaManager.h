#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

#include <set>
#include <string>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <luabind/luabind.hpp>
#include <luabind/class.hpp>
#include <luabind/function.hpp>
#include <luabind/object.hpp>
#include <luabind/operator.hpp>
#include <luabind/shared_ptr_converter.hpp>

#include <iostream>

// creates a boost reference from the pointer
#define REF_POINTER(T, p)	if (std::is_pointer<T>())	p = std::ref<T>(p);

class Entity;
typedef std::shared_ptr<Entity> EntityPtr;


class LuaManager : public Singleton<LuaManager>
{
public:
	LuaManager();

	void	init();
	void	close();

	void	doFile(const std::string& file);
	void	doString(const std::string& command);

	void	createTable(const std::string& tableName, const std::set<Entity*>& entities);
	void	createTable(const std::string& tableName, const Entity* entities[] = nullptr, const int numEntities = 0);

	void	printError(const luabind::error& e);
	int		handleError(lua_State* state);

	template <typename Type, typename Id>
	void setTableElement(const std::string& tableName, Id id, Type element)
	{
		luabind::globals(m_state)[tableName.c_str()] [id] = element;
	}

	/**
	 * Registers a non-entity object. Actor object are located in the entityTable.
	 */
	template <typename Type>
	void registerObject(const std::string& name, const Type element)
	{
		luabind::globals(m_state)[name] = element;
	}

	template <typename Type>
	int registerActor(const Type element, const std::string& name = "")
	{
		const int size = luabind::object_cast<int>( luabind::globals(m_state)["entityTableSize"] );
		luabind::globals(m_state)["entityTableSize"] = size + 1;
		luabind::globals(m_state)["entityTable"] [ name ] = element;

		if (name.find("Unit") != std::string::npos)
		{
			registerActorToTable("unitTable", element, name);
		}
		else if (name.find("Item") != std::string::npos)
		{
			registerActorToTable("itemTable", element, name);
		}
		else if (name.find("Player") != std::string::npos)
		{
			registerActorToTable("unitTable", element, name);
			registerActorToTable("playerTable", element, name);
		}

		addKeywordToConsole(name);

		return size + 1;
	}

	template <typename Type, typename Id>
	int registerActorToTable(const std::string& tableName, const Type& element, const Id id, const bool registerToActorTable = false)
	{
		const int entityTableSize = luabind::object_cast<int>( luabind::globals(m_state)["entityTableSize"] );

		if (registerToActorTable)
		{
			luabind::globals(m_state)["entityTableSize"] = entityTableSize + 1;
			luabind::globals(m_state)["entityTable"] [ id ] = element;
		}

		const int tableSize = luabind::object_cast<int>( luabind::globals(m_state)[tableName + "Size"] );
		luabind::globals(m_state)[tableName + "Size"] = tableSize + 1;
		luabind::globals(m_state)[tableName.c_str()] [ id ] = element;

		return entityTableSize + 1;
	}

	template <typename Id>
	int unregisterActorFromTable(const std::string& tableName, const Id id, const bool unregisterFromActorTable = false)
	{
		const int entityTableSize = luabind::object_cast<int>( luabind::globals(m_state)["entityTableSize"] );

		std::stringstream removeCommandSS;

		if (unregisterFromActorTable)
		{
			removeCommandSS << "table.remove(entityTable, " << id << ")";

			luabind::globals(m_state)["entityTableSize"] = entityTableSize - 1;
			doString(removeCommandSS.str());

			removeCommandSS.clear();
		}

		removeCommandSS << "table.remove(" << tableName << ", " << id << ")";

		const int tableSize = luabind::object_cast<int>( luabind::globals(m_state)[tableName + "Size"] );
		luabind::globals(m_state)[tableName + "Size"] = tableSize - 1;
		doString(removeCommandSS.str());

		return entityTableSize - 1;
	}

	int registerEntity(const Entity& entity, const std::string& tableName = "");
	int registerEntitySP(const EntityPtr& entity, const std::string& tableName = "");

	void unregisterEntity(const Entity& entity, const std::string& tableName = "");
	void unregisterEntitySP(const EntityPtr& entity, const std::string& tableName = "");

	template <typename Type>
	Type* getActor(const std::string& actorName)
	{
		return object_cast<Type*>(luabind::globals(m_state)[actorName]);
	}


	bool functionExist(const std::string& functionName);

	// Call function
	void callFunction(const std::string& methodName)
	{
		try
		{
			luabind::call_function<void>(m_state, methodName.c_str());
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}

	template <typename Ref1>
	void callFunction(const std::string& methodName, Ref1 ref1)
	{
		REF_POINTER(Ref1, ref1);

		try
		{
			luabind::call_function<void>(m_state, methodName.c_str(), ref1);
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}

	template <typename Ref1, typename Ref2>
	void callFunction(const std::string& methodName, Ref1 ref1, Ref2 ref2)
	{
		REF_POINTER(Ref1, ref1);
		REF_POINTER(Ref2, ref2);

		try
		{
			luabind::call_function<void>(m_state, methodName.c_str(), ref1, ref2);
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}

	template <typename Ref1, typename Ref2, typename Ref3>
	void callFunction(const std::string& methodName, Ref1 ref1, Ref2 ref2, Ref3 ref3)
	{
		REF_POINTER(Ref1, ref1);
		REF_POINTER(Ref2, ref2);
		REF_POINTER(Ref3, ref3);

		try
		{
			luabind::call_function<void>(m_state, methodName.c_str(), ref1, ref2, ref3);
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}

	template <typename Ref1, typename Ref2, typename Ref3, typename Ref4>
	void callFunction(const std::string& methodName, Ref1& ref1, Ref2& ref2, Ref3& ref3, Ref4& ref4)
	{
		REF_POINTER(Ref1, ref1);
		REF_POINTER(Ref2, ref2);
		REF_POINTER(Ref3, ref3);
		REF_POINTER(Ref4, ref4);

		try
		{
			luabind::call_function<void>(m_state, methodName.c_str(), ref1, ref2, ref3, ref4);
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}



	// Call member function
	template <typename Callee>
	void callMethod(Callee object, const std::string& methodName)
	{
		try
		{
			luabind::globals(m_state)["myObj"] = object;
			luabind::call_member<void>(luabind::globals(m_state)["myObj"], methodName.c_str());
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}

	template <typename Callee, typename Ref1>
	void callMethod(Callee object, const std::string& methodName, Ref1 ref1)
	{
		REF_POINTER(Ref1, ref1);

		try
		{
			luabind::globals(m_state)["myObj"] = object;
			luabind::call_member<void>(luabind::globals(m_state)["myObj"], methodName.c_str(), ref1);
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}

	template <typename Callee, typename Ref1, typename Ref2>
	void callMethod(Callee object, const std::string& methodName, Ref1 ref1, Ref2 ref2)
	{
		REF_POINTER(Ref1, ref1);
		REF_POINTER(Ref2, ref2);

		try
		{
			luabind::globals(m_state)["myObj"] = object;
			luabind::call_member<void>(luabind::globals(m_state)["myObj"], methodName.c_str(), ref1, ref2);
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}

	template <typename Callee, typename Ref1, typename Ref2, typename Ref3>
	void callMethod(Callee object, const std::string& methodName, Ref1 ref1, Ref2 ref2, Ref3 ref3)
	{
		REF_POINTER(Ref1, ref1);
		REF_POINTER(Ref2, ref2);
		REF_POINTER(Ref3, ref3);

		try
		{
			luabind::globals(m_state)["myObj"] = object;
			luabind::call_member<void>(luabind::globals(m_state)["myObj"], methodName.c_str(), ref1, ref2, ref3);
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}

	template <typename Callee, typename Ref1, typename Ref2, typename Ref3, typename Ref4>
	void callMethod(Callee object, const std::string& methodName, Ref1 ref1, Ref2 ref2, Ref3 ref3, Ref4 ref4)
	{
		REF_POINTER(Ref1, ref1);
		REF_POINTER(Ref2, ref2);
		REF_POINTER(Ref3, ref3);
		REF_POINTER(Ref4, ref4);

		try
		{
			luabind::globals(m_state)["myObj"] = object;
			luabind::call_member<void>(luabind::globals(m_state)["myObj"], methodName.c_str(), ref1, ref2, ref3, ref4);
		}
		catch (luabind::error e)
		{
			printError(e);
		}
	}


	// getters-setters
	lua_State* getState();


private:
	lua_State*	m_state;
	bool		m_isOpened;
};

#endif // LUA_MANAGER_H
