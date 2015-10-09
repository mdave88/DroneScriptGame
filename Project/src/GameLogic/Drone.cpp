#include "GameStdAfx.h"
#include "GameLogic/Drone.h"
#include "Common/LuaManager.h"


SERIALIZABLE(Drone)

Drone::Drone(const entityx::Entity& entity)
	: m_entity(entity)
	, m_id(0)
{
	m_entity.assign<ModuleBase>();
	m_entity.assign<Movement>();
	m_entity.assign<Health>();
}

void Drone::addModule(const ModuleBase& module)
{
}

void Drone::removeModule()
{
}

void Drone::move(const vec2& vel)
{
	if(m_entity.has_component<Movement>())
	{
		Movement& movement = *m_entity.component<Movement>().get();
		movement.set_vel(vel);
	}
	else
	{
		m_log << "Error in move(): no Movement component" << std::endl;
	}
}

void Drone::activateModule(const ModuleType moduleType)
{
	switch(moduleType)
	{
		case ModuleType::BATTERY:
			break;
		case ModuleType::MOBYLITY:
			if(m_entity.has_component<Movement>())
			{
				//m_entity.component<Movement>()->
			}
			break;
		case ModuleType::MEMORY:
			break;
		case ModuleType::HDD:
			break;
		case ModuleType::WELDER:
			break;
		case ModuleType::JACKHAMMER:
			break;
		case ModuleType::RADIO_TRANSMITTER:
			break;
		case ModuleType::RADIO_RECEIVER:
			break;
		case ModuleType::RADAR:
			break;
		case ModuleType::LADAR:
			break;
		case ModuleType::FUELCREATOR:
			break;
	}
}


// register to lua
void Drone::registerMethodsToLua()
{
	using namespace luabind;

	class_<Drone, std::shared_ptr<Drone>> thisClass("Drone");

	REG_FUNC("move", &Drone::move);

	module(LuaManager::getInstance()->getState())[thisClass];
}

template <typename Archive>
void Drone::load(Archive& ar, const uint version)
{
	ar& attribIndex;
	ar& attribMask;

	serializeComponents(ar, version);
}

template <typename Archive>
void Drone::save(Archive& ar, const uint version)
{
	attribIndex = 0;
	attribMask.reset();

	serializeComponents(ar, version);
}

template <typename Archive>
void Drone::serializeComponents(Archive& ar, const uint version)
{
	// TODO: foreach(ComponentType)
	if(m_entity.has_component<Movement>())
	{
		m_entity.component<Movement>().get()->serialize(ar, version, attribMask, attribIndex);
	}
	else
	{
		attribIndex += Movement::numPersistentAttribs;
	}
}
