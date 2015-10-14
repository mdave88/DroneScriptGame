#include "GameStdAfx.h"
#include "GameLogic/GameObject.h"
#include "GameLogic/ComponentFactory.h"
#include "Common/LuaManager.h"

#include <boost/serialization/bitset.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>

GameObject::GameObject(const entityx::Entity& entity)
	: m_entity(entity)
	, m_id(0)
{
}

void GameObject::addComponent(const ComponentType componentType, PersistentComponent* componentPtr)
{
	m_components[componentType] = componentPtr;
}

void GameObject::removeModule()
{
}

void GameObject::move(const vec2& vel)
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

void GameObject::activateModule(const ModuleType moduleType)
{

}


// register to lua
void GameObject::registerMethodsToLua()
{
	using namespace luabind;

	class_<GameObject, std::shared_ptr<GameObject>> thisClass("GameObject");

	REG_FUNC("move", &GameObject::move);

	module(LuaManager::getInstance()->getState())[thisClass];
}

// serialization
template <typename Archive>
void GameObject::serialize(Archive& ar, const uint version)
{
	ar& boost::serialization::base_object<Serializable>(*this);
	ar& BOOST_SERIALIZATION_NVP(m_components);
}

template<typename Archive>
void GameObject::serializeComponents(Archive& ar, const uint version)
{
}

SERIALIZABLE(GameObject);
