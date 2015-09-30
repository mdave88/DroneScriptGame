#include "GameStdAfx.h"
#include "GameLogic/Drone.h"
#include "Common/LuaManager.h"

Drone::Drone()
	: m_id(0)
{
}

void Drone::addModule(const ModuleBase& module)
{
	assign<ModuleBase>();
}

void Drone::removeModule()
{
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

// register methods to lua
//void Drone::registerMethodsToLua()
//{
//	using namespace luabind;
//
//	class_<Drone, DronePtr> thisClass("Drone");
//	thisClass.def(constructor<vec3>());
//
//	REG_PROP("posP", &Drone::getPos, &Drone::setPos);
//	REG_PROP("rotP", &Drone::getRot, &Drone::setRot);
//	REG_PROP("nameP", &Drone::getName, &Drone::setName);
//
//	module(LuaManager::getInstance()->getState()) [ thisClass ];
//}
