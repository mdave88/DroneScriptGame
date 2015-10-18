#pragma once

#include "GameLogic/SerializationDefs.h"
#include "Common/LoggerSystem.h"

enum class ComponentType
{
	MOVEMENT,
	HEALTH,
	BATTERY,
	MOBYLITY,
	MEMORY,
	HDD,
	WELDER,
	JACKHAMMER,
	RADIO_TRANSMITTER,
	RADIO_RECEIVER,
	RADAR,
	LADAR,
	FUEL_CREATOR,
	NUM
};

class ComponentBase : public Serializable
{
public:
	ComponentBase(NetworkPriority networkPriority = NetworkPriority::MEDIUM) : Serializable(networkPriority)
	{
	}
};

class Movement : public ComponentBase
{
public:
	Movement(vec2 pos = vec2(0.0f), vec2 vel = vec2(0.0f))
		: ComponentBase(NetworkPriority::MEDIUM)
		, pos(pos), vel(vel)
	{
	}

private:
	vec2 pos;
	vec2 vel;

	SERIALIZABLE_CLASS;
	SERIALIZE2(pos, vel);
};

BOOST_CLASS_EXPORT_KEY(Movement);


class Health : public ComponentBase
{
public:
	Health(uint8_t maxHealth = 0, uint8_t health = 0)
		: ComponentBase(NetworkPriority::MEDIUM)
		, maxHealth(maxHealth), health(health)
	{
	}

private:
	uint8_t maxHealth;
	uint8_t health;

	// TODO: serialize consts only on creation
	SERIALIZABLE_CLASS;
	SERIALIZE2(maxHealth, health);
};

BOOST_CLASS_EXPORT_KEY(Health);
