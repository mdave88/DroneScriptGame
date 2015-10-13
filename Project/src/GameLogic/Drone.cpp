#include "GameStdAfx.h"
#include "GameLogic/Drone.h"
#include "GameLogic/ComponentFactory.h"
#include "Common/LuaManager.h"

#include <boost/serialization/bitset.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>

Drone::Drone(const entityx::Entity& entity)
	: m_entity(entity)
	, m_id(0)
{
}

void Drone::addComponent(const ComponentType componentType, PersistentComponent* componentPtr)
{
	m_components[componentType] = componentPtr;
}

void Drone::removeModule()
{
}

void Drone::move(const vec2& vel)
{
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
	ar& boost::serialization::base_object<Serializable>(*this);
	ar& BOOST_SERIALIZATION_NVP(m_components);
	//serializeComponents(ar, version);
}

template<typename Archive>
void Drone::serializeComponents(Archive& ar, const uint version)
{
}

SERIALIZABLE(Drone);
