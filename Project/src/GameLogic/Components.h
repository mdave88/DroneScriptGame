#pragma once

#include "GameLogic/SerializationDefs.h"
#include "Common/LoggerSystem.h"
#include <bitset>

#define CREATE_ACCESSOR(name, attribIndex)	public: \
											const decltype(name)& get_##name##() const { return name; } \
											void set_##name##(const decltype(name)& newval) { name = newval; attribMask[attribIndex] = true; }


#define CREATE_ACCESSORS1(_1)				public: \
											static const int numPersistentAttribs = 1; \
											CREATE_ACCESSOR(_1, 0) \
											SERIALIZABLE_CLASS

#define CREATE_ACCESSORS2(_1, _2)			public: \
											static const int numPersistentAttribs = 2; \
											CREATE_ACCESSOR(_1, 0) \
											CREATE_ACCESSOR(_2, 1) \
											SERIALIZABLE_CLASS

#define CREATE_ACCESSORS3(_1, _2, _3)		public: \
											static const int numPersistentAttribs = 3; \
											CREATE_ACCESSOR(_1, 0) \
											CREATE_ACCESSOR(_2, 1) \
											CREATE_ACCESSOR(_3, 2) \
											SERIALIZABLE_CLASS

#define CREATE_ACCESSORS4(_1, _2, _3)		public: \
											static const int numPersistentAttribs = 4; \
											CREATE_ACCESSOR(_1, 0) \
											CREATE_ACCESSOR(_2, 1) \
											CREATE_ACCESSOR(_3, 2) \
											CREATE_ACCESSOR(_4, 3) \
											SERIALIZABLE_CLASS


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

class PersistentComponent : public Serializable
{
public:
	PersistentComponent(NetworkPriority networkPriority = NetworkPriority::MEDIUM) : Serializable(networkPriority)
	{
	}
};

class Movement : public PersistentComponent
{
public:
	Movement(vec2 pos = vec2(0.0f), vec2 vel = vec2(0.0f))
		: PersistentComponent(NetworkPriority::MEDIUM)
		, pos(pos), vel(vel)
	{
	}

private:
	vec2 pos;
	vec2 vel;

	CREATE_ACCESSORS2(pos, vel)
};

BOOST_CLASS_EXPORT_KEY(Movement);


class Health : public PersistentComponent
{
public:
	Health(uint8_t maxHealth = 0, uint8_t health = 0)
		: PersistentComponent(NetworkPriority::MEDIUM)
		, maxHealth(maxHealth), health(health)
	{
	}

private:
	uint8_t maxHealth;
	uint8_t health;

	// TODO: serialize consts only on creation
	CREATE_ACCESSORS2(maxHealth, health)
};

BOOST_CLASS_EXPORT_KEY(Health);


static const uint8_t componentAttribNums[(uint8_t)ComponentType::NUM] =
{
	Movement::numPersistentAttribs,
	Health::numPersistentAttribs,
	0, //Battery,
	0, //Mobylity,
	0, //Memory,
	0, //Hdd,
	0, //Welder,
	0, //Jackhammer,
	0, //RadioTransmitter,
	0, //RadioReceiver,
	0, //Radar,
	0, //Ladar,
	0, //Fuel_creator,
};
