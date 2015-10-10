#include "GameStdAfx.h"
#include "GameLogic/Drone.h"
#include "Common/LuaManager.h"
#include <boost/serialization/bitset.hpp>

Drone::Drone(const entityx::Entity& entity)
	: m_entity(entity)
	, m_id(0)
{
	for (PersistentComponent* component : m_components)
	{
		component = nullptr;
	}

	addComponent(ComponentType::MOVEMENT, m_entity.assign<Movement>().get());
	addComponent(ComponentType::HEALTH, m_entity.assign<Health>().get());
}

void Drone::addComponent(const ComponentType componentType, PersistentComponent* componentsPtr)
{
	m_components[(uint8_t)componentType] = componentsPtr;
	componentsPtr->setAttribMask(&m_attribMask, &m_attribIndex);
}

void Drone::removeModule()
{
}

void Drone::move(const vec2& vel)
{
	//m_modules[ModuleType::MOBYLITY]
	if (m_entity.has_component<Movement>())
	{
		Movement* movement = m_entity.component<Movement>().get();
		movement->set_vel(vel);
	}
	else
	{
		m_log << "Error in move(): no Movement component" << std::endl;
	}
}

void Drone::activateModule(const ModuleType moduleType)
{

}


// register to lua
void Drone::registerMethodsToLua()
{
	using namespace luabind;

	class_<Drone, std::shared_ptr<Drone>> thisClass("Drone");

	REG_FUNC("move", &Drone::move);

	module(LuaManager::getInstance()->getState())[thisClass];
}

// serialization
template <typename Archive>
void Drone::serialize(Archive& ar, const uint version)
{
	uint64_t attribMaskInt = m_attribMask.to_ulong();
	ar& attribMaskInt;

	serializeComponents(ar, version);
}

template<typename Archive>
void Drone::serializeComponents(Archive & ar, const uint version)
{
	for (uint8_t i = 0; i < (uint8_t)ComponentType::NUM; ++i)
	{
		if (m_components[i] != nullptr)
		{
			switch ((ComponentType)i)
			{
				case ComponentType::MOVEMENT:
					static_cast<Movement*>(m_components[i])->serialize(ar, version);
					break;
				default:
					break;
			}
		}
	}
}

SERIALIZABLE(Drone);
