#pragma once

#include <entityx/entityx.h>

#include "GameLogic/Modules.h"

/**
 * @brief Wraps an entityx::Entity.
 */
class Drone : public Serializable
{
public:
	Drone() {}
	Drone(const entityx::Entity& entity);

	void addComponent(const ComponentType componentType, PersistentComponent* componentsPtr);

	void removeModule();
	void activateModule(const ModuleType moduleType);
	
	void move(const vec2& vel);

	entityx::Entity& getEntity() { return m_entity; }

	// register to lua
	static void registerMethodsToLua();

private:
	friend class boost::serialization::access;

	// serialization
	template <typename Archive>
	void serialize(Archive& ar, const uint version);

	template <typename Archive>
	void serializeComponents(Archive& ar, const uint version);

protected:
	entityx::Entity				m_entity;
	uint8_t						m_id;
	std::string					m_name;

	PersistentComponent*		m_components[(uint8_t)ComponentType::NUM];
	//ModuleBase*				m_modules[(uint8_t)ModuleType::NUM];

	std::stringstream			m_log;

	uint8_t						m_inventorySize;
};

BOOST_CLASS_EXPORT_KEY(Drone);