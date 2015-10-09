#pragma once

#include "GameLogic/SerializationDefs.h"
#include <bitset>

#define CREATE_ACCESSOR(name, attribIndex)	public: \
											const decltype(name)& get_##name##() { return name; } \
											void set_##name##(const decltype(name)& newval) { name = newval; (*attribMaskPtr)[*attribIndexPtr] = true; }


#define CREATE_ACCESSORS1(_1)			public: \
										static const int numPersistentAttribs = 1; \
										CREATE_ACCESSOR(_1, 0)

#define CREATE_ACCESSORS2(_1, _2)		public: \
										static const int numPersistentAttribs = 2; \
										CREATE_ACCESSOR(_1, 0) \
										CREATE_ACCESSOR(_2, 1)

#define CREATE_ACCESSORS3(_1, _2, _3)	public: \
										static const int numPersistentAttribs = 3; \
										CREATE_ACCESSOR(_1, 0) \
										CREATE_ACCESSOR(_2, 1) \
										CREATE_ACCESSOR(_3, 2)

#define CREATE_ACCESSORS4(_1, _2, _3)	public: \
										static const int numPersistentAttribs = 4; \
										CREATE_ACCESSOR(_1, 0) \
										CREATE_ACCESSOR(_2, 1) \
										CREATE_ACCESSOR(_3, 2) \
										CREATE_ACCESSOR(_4, 3)

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


class Movement : public PersistentComponent
{
public:
	Movement(vec2 pos = vec2(0.0f), vec2 vel = vec2(0.0f))
		: PersistentComponent(NetworkPriority::MEDIUM)
		, pos(pos), vel(vel)
	{
	}

	template <typename Archive>
	void serialize(Archive& ar, const uint32_t version)
	{
		ar& boost::serialization::base_object<PersistentComponent>(*this);
		serializeVec2Fields(ar, pos, NetworkPriority::HIGH,
								vel, NetworkPriority::MEDIUM);
	}

private:
	vec2 pos;
	vec2 vel;

	CREATE_ACCESSORS2(pos, vel)
};

class Health : public PersistentComponent
{
public:
	Health(uint8_t maxHealth = 0, uint8_t health = 0)
		: PersistentComponent(NetworkPriority::MEDIUM)
		, maxHealth(maxHealth), health(health)
	{
	}

	template <typename Archive>
	void serialize(Archive& ar, const uint32_t version)
	{
		ar& boost::serialization::base_object<PersistentComponent>(*this);
		serializePrimitiveFields(ar, maxHealth, health);
	}

private:
	uint8_t maxHealth;
	uint8_t health;

	// TODO: serialize consts only on creation
	CREATE_ACCESSORS2(maxHealth, health)
};
