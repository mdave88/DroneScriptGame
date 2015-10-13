#include "GameStdAfx.h"
#include "GameLogic/Drone.h"
#include "GameLogic/ComponentFactory.h"
#include "Common/LuaManager.h"
#include <boost/serialization/bitset.hpp>


Drone::Drone(const entityx::Entity& entity)
	: m_entity(entity)
	, m_id(0)
{
	for (uint8_t i = 0; i < (uint8_t)ComponentType::NUM; ++i)
	{
		m_components[i] = nullptr;
	}
}

void Drone::addComponent(const ComponentType componentType, PersistentComponent* componentPtr)
{
	m_components[(uint8_t)componentType] = componentPtr;

	uint8_t attribStartIndex = 1;
	for(uint8_t i = 0; i < (uint8_t)componentType; ++i)
	{
		attribStartIndex += componentAttribNums[i];
	}
	componentPtr->setAttribMask(&m_attribMask, &m_attribIndex, attribStartIndex);
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
	// reset a direction marking bit
	m_attribMask.reset(0);
	m_attribIndex = 0;

	// mark the direction on the first bit on the attrib mask: 0 - save, 1 - load
	boost::serialization::split_member(ar, *this, version);

	serializeComponents(ar, version);
}


template<class Archive>
void Drone::save(Archive& ar, const uint version) const
{
	uint64_t attribMaskInt = m_attribMask.to_ulong();
	ar << attribMaskInt;
}

template<class Archive>
void Drone::load(Archive& ar, const uint version)
{
	uint64_t attribMaskInt;
	ar >> attribMaskInt;
	m_attribMask = std::bitset<ATTRIB_NUM>(attribMaskInt);
	m_attribMask.set(0);
}


template<typename Archive>
void Drone::serializeComponents(Archive& ar, const uint version)
{
	for (uint8_t i = 0; i < (uint8_t)ComponentType::NUM; ++i)
	{
		const bool saving = m_attribMask[0] == false;
		if (saving)							// save
		{
			if (m_components[i] != nullptr)
			{
				ar& m_components[i];
			}
			else
			{
				m_attribIndex += componentAttribNums[i];
			}
		}
		else								// load
		{
			if (m_components[i] != nullptr)
			{
				ar& m_components[i];
			}
			else
			{
				PersistentComponent* componentPtr = ComponentFactory::getInstance()->assignComponent(m_entity, (ComponentType)i);
				addComponent((ComponentType)i, componentPtr);
				ar& m_components[i];
			}
		}
	}
}

SERIALIZABLE(Drone);
