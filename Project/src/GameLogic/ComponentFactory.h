#pragma once

#include "Common/LoggerSystem.h"
#include "GameLogic/Modules.h"
#include <entityx/entityx.h>

class ComponentFactory : public Singleton<ComponentFactory>
{
public:
	template <typename C>
	C* assignComponentType(entityx::Entity& entity)
	{
		return entity.assign<C>().get();
	}

	PersistentComponent* assignComponent(entityx::Entity& entity, ComponentType componentType)
	{
		switch(componentType)
		{
			case ComponentType::MOVEMENT:
				return assignComponentType<Movement>(entity);
			case ComponentType::HEALTH:
				return assignComponentType<Health>(entity);
			case ComponentType::BATTERY:
				return assignComponentType<Battery>(entity);
			case ComponentType::MOBYLITY:
				return assignComponentType<Mobility>(entity);
			case ComponentType::MEMORY:
				return assignComponentType<Memory>(entity);
			case ComponentType::HDD:
				return assignComponentType<Hdd>(entity);
			case ComponentType::WELDER:
				return assignComponentType<Welder>(entity);
			case ComponentType::JACKHAMMER:
				return assignComponentType<Jackhammer>(entity);
			case ComponentType::RADIO_TRANSMITTER:
				return assignComponentType<RadioTransmitter>(entity);
			case ComponentType::RADIO_RECEIVER:
				return assignComponentType<RadioReceiver>(entity);
			case ComponentType::RADAR:
				return assignComponentType<Radar>(entity);
			case ComponentType::LADAR:
				return assignComponentType<Ladar>(entity);
			case ComponentType::FUEL_CREATOR:
				return assignComponentType<FuelCreator>(entity);
			default:
				GX_ASSERT("Error: ComponentFactory can't handle this ComponentType.");
		}

		return nullptr;
	}
};
