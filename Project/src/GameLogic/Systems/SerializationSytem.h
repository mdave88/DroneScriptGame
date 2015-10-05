#pragma once

#include <entityx/entityx.h>
#include "GameLogic/Components.h"

/**
 * @brief CollisionSystem
 */
class SerializationSytem : public entityx::System<SerializationSytem>
{
public:
	void update(entityx::EntityManager& es, entityx::EventManager& events, entityx::TimeDelta dt) override
	{
		entityx::ComponentHandle<Serializable> serializableComponent;
		for(entityx::Entity serializableEntity : es.entities_with_components(serializableComponent))
		{
		}
	};
};
