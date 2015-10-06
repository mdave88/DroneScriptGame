#include "GameStdAfx.h"
#include "GameLogic/Drone.h"
#include "Common/LuaManager.h"

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
		case ModuleType::Battery:
			break;
		case ModuleType::Mobylity:
			break;
		case ModuleType::Memory:
			break;
		case ModuleType::Hdd:
			break;
		case ModuleType::Welder:
			break;
		case ModuleType::Jackhammer:
			break;
		case ModuleType::RadioTransmitter:
			break;
		case ModuleType::RadioReceiver:
			break;
		case ModuleType::Radar:
			break;
		case ModuleType::Ladar:
			break;
		case ModuleType::FuelCreator:
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
