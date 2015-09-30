#pragma once

#include <entityx/entityx.h>

#include "GameLogic/Modules.h"

/**
 * @brief Drones, you know this is what this game is about.
 */
class Drone : public entityx::Entity
{
public:
	Drone();

	void addModule(const ModuleBase& module);
	void removeModule();
	void activateModule(const ModuleType moduleType);
	
protected:
	uint8_t		m_id;
	std::string	m_name;

	uint8_t		m_inventorySize;
};
