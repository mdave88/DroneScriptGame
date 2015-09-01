#ifndef DOOR_H
#define DOOR_H

#include "GameLogic/Entity.h"


class Door : public Entity
{
public:
	Door(const std::string& key = "masterKey", const vec3& pos = vec3(0.0f)) : m_key(key), Entity(pos)
	{
		m_status = DoorState::CLOSED;
		//MQLOG("is a new Door", 0);
	}

	virtual void open()
	{
		m_status = DoorState::OPENING;
	}

	virtual const std::string& getKey()
	{
		return m_key;
	}
	
	virtual void animate(const float dt)
	{
		LuaManager::getInstance()->callMethod(this, "animateL", dt);
	}


	// register to lua
	static void registerMethodsToLua()
	{
		using namespace luabind;

#ifdef USE_LUABIND_DEBOOSTIFIED
		class_<Door, Entity, EntityPtr> thisClass("Door");
#else
		class_<Door, NodePtr, bases<Entity>> thisClass("Door");
#endif
		//thisClass.def(constructor<std::string>());
		//thisClass.def(constructor<std::string, vec3>());

		thisClass.enum_("DoorStatus")
		[
		    value("OPEN", 0),
		    value("CLOSED", 1),
		    value("OPENING", 2),
		    value("CLOSING", 3)
		];

		//REG_ATTR("status", &Door::m_status);

		REG_FUNC("open", &Door::open);
		REG_FUNC("getKey", &Door::getKey);

		module(LuaManager::getInstance()->getState()) [ thisClass ];
	}

	// serialization
	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& boost::serialization::base_object<Entity>(*this);
	}


protected:
	enum class DoorState
	{
		OPEN,
		CLOSED,
		OPENING,
		CLOSING
	};

	DoorState	m_status;
	std::string	m_key;
};

#endif // DoorState::H
