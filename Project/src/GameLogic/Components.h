#pragma once

#include "GameLogic/SerializationDefs.h"
#include <bitset>

#define CREATE_ACCESSOR(name, attribIndex)	public: \
											const decltype(name)& get_##name##() { return name; } \
											void set_##name##(const decltype(name)& newval) { name = newval; (*attribMaskPtr)[*attribIndexPtr] = true; }


#define CREATE_ACCESSORS1(_1)				public: \
											static const int numPersistentAttribs = 1; \
											CREATE_ACCESSOR(_1, 0)

#define CREATE_ACCESSORS2(_1, _2)			public: \
											static const int numPersistentAttribs = 2; \
											CREATE_ACCESSOR(_1, 0) \
											CREATE_ACCESSOR(_2, 1)

#define CREATE_ACCESSORS3(_1, _2, _3)		public: \
											static const int numPersistentAttribs = 3; \
											CREATE_ACCESSOR(_1, 0) \
											CREATE_ACCESSOR(_2, 1) \
											CREATE_ACCESSOR(_3, 2)

#define CREATE_ACCESSORS4(_1, _2, _3)		public: \
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

class PersistentComponent
{
public:
	PersistentComponent(ComponentType componentType = ComponentType::NUM, NetworkPriority networkPriority = NetworkPriority::MEDIUM)
		: componentType(componentType)
		, attribMaskPtr(nullptr)
		, attribIndexPtr(nullptr)
		, m_networkPriority(networkPriority)
	{
	}

	void setAttribMask(std::bitset<64>* _attribMask, uint8_t* _attribIndex)
	{
		attribMaskPtr = _attribMask;
		attribIndexPtr = _attribIndex;
	}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		SER_P(m_networkPriority);	// network priority can change on the fly -> compressions can vary
	}

protected:
	template <typename Archive>
	void serializePrimitiveFields(Archive& ar) {}

	template <typename Archive, typename T, typename... Args>
	void serializePrimitiveFields(Archive& ar, T& field, Args... args)
	{
		SER_P(field);
		serializePrimitiveFields(ar, args...);
	}

	template <typename Archive>
	void serializeVec2Fields(Archive& ar) {}

	template <typename Archive, typename... Args>
	void serializeVec2Fields(Archive& ar, vec2& field, NetworkPriority priority, Args... args)
	{
		SER_P_VEC2(field, priority);
		serializeVec2Fields(ar, args...);
	}

protected:
	const ComponentType componentType;
	uint8_t* attribIndexPtr;
	std::bitset<ATTRIB_NUM>* attribMaskPtr;
	NetworkPriority m_networkPriority;
};


class Movement : public PersistentComponent
{
public:
	Movement(vec2 pos = vec2(0.0f), vec2 vel = vec2(0.0f))
		: PersistentComponent(ComponentType::MOVEMENT, NetworkPriority::MEDIUM)
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
		: PersistentComponent(ComponentType::HEALTH, NetworkPriority::MEDIUM)
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
	0, //RadioTransmitter::,
	0, //Radio_receiver,
	0, //Radar,
	0, //Ladar,
	0, //Fuel_creator,
};
