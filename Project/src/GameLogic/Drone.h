#pragma once

#include <entityx/entityx.h>

#include "GameLogic/Modules.h"

/**
 * @brief Wraps an entityx::Entity.
 */
class Drone : public Serializable
{
public:
	Drone(const entityx::Entity& entity);

	void addModule(const ModuleBase& module);
	void removeModule();
	void activateModule(const ModuleType moduleType);
	
	void move(const vec2& vel);

	entityx::Entity& getEntity() { return m_entity; }

	// serialization
	template <typename Archive>
	void serialize(Archive& ar, const uint version);

	// register to lua
	static void registerMethodsToLua();

protected:
	entityx::Entity		m_entity;
	uint8_t				m_id;
	std::string			m_name;

	std::stringstream	m_log;

	uint8_t				m_inventorySize;
};

BOOST_CLASS_EXPORT_KEY(Drone)