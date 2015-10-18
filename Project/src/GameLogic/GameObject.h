#pragma once

#include "GameLogic/Modules.h"

#include <entityx/entityx.h>

typedef std::map<ComponentType, ComponentBase*> ComponentMap;


/**
 * @brief Wraps an entityx::Entity.
 */
class GameObject : public Serializable
{
	SERIALIZABLE_CLASS

public:
	GameObject() {}
	GameObject(const entityx::Entity& entity);

	void addComponent(const ComponentType componentType, ComponentBase* componentPtr);

	void removeModule();
	void activateModule(const ModuleType moduleType);
	
	void move(const vec2& vel);

	entityx::Entity& getEntity() { return m_entity; }

	// register to lua
	static void registerMethodsToLua();

private:
	template <typename Archive>
	void serializeComponents(Archive& ar, const uint version);

protected:
	entityx::Entity				m_entity;
	uint8_t						m_id;
	std::string					m_name;

	ComponentMap				m_components;

	std::stringstream			m_log;

	uint8_t						m_inventorySize;
};

BOOST_CLASS_EXPORT_KEY(GameObject);
