#pragma once

#include "GameLogic/SerializationDefs.h"
#include <bitset>

#define CREATE_ACCESSOR(name, attribIndex)	public: \
											const decltype(name)& get_##name##() { return name; } \
											void set_##name##(const decltype(name)& newval) { name = newval; BIT_SET(m_attribMask, attribIndex); }


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


struct Serializable
{
	uint8_t id;
	std::bitset<64> attribMask;
	
	NetworkPriority networkPriority;
};

// TODO: reset at the start of the main update()
struct PersistentComponent
{
	uint8_t m_attribMask;
	uint8_t m_attribIndex;
	NetworkPriority m_networkPriority;

	PersistentComponent(uint8_t attribMask = 0, uint8_t attribIndex = 0, NetworkPriority networkPriority = NetworkPriority::MEDIUM)
		: m_attribMask(attribMask)
		, m_attribIndex(attribIndex)
		, m_networkPriority(networkPriority)
	{}

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

	template <typename Archive>
	void serializeVec3Fields(Archive& ar) {}

	template <typename Archive, typename... Args>
	void serializeVec2Fields(Archive& ar, const vec2& field, Args... args)
	{
		SER_P_VEC2(field);
		serializeVec2Fields(ar, args...);
	}

	template <typename Archive, typename... Args>
	void serializeVec3Fields(Archive& ar, const vec3& field, Args... args)
	{
		SER_P_VEC3(field);
		serializeVec3Fields(ar, args...);
	}


	template <typename Archive>
	void serializeVec2FieldsWithPriority(Archive& ar) {}

	template <typename Archive, typename... Args>
	void serializeVec2FieldsWithPriority(Archive& ar, vec2& field, NetworkPriority priority, Args... args)
	{
		//SER_P_VEC2(field, priority);
		serializeVec2(ar, field, m_attribMask, m_attribIndex, m_networkPriority >= priority);
		serializeVec2FieldsWithPriority(ar, args...);
	}
};

class Movement : public PersistentComponent
{
public:
	Movement(vec2 pos = vec2(0.0f), vec2 vel = vec2(0.0f)) : pos(pos), vel(vel) {}

	template <typename Archive>
	void serialize(Archive& ar, const uint32_t version)
	{
		serializeVec2FieldsWithPriority(ar, pos, NetworkPriority::HIGH, vel, NetworkPriority::MEDIUM);
	}

private:
	vec2 pos;
	vec2 vel;

	CREATE_ACCESSORS2(pos, vel)
};

class Health : public PersistentComponent
{
public:
	Health(uint8_t maxHealth = 0, uint8_t health = 0) : maxHealth(maxHealth), health(health) {}

	template <typename Archive>
	void serialize(Archive& ar, const uint32_t version)
	{
		serializePrimitiveFields(ar, maxHealth, health);
	}

private:
	uint8_t maxHealth;
	uint8_t health;

	// TODO: serialize consts only on creation
	CREATE_ACCESSORS2(maxHealth, health)
};

class Explosive : public PersistentComponent
{
public:
	Explosive(uint8_t damageBase = 0, uint8_t range = 0) : damageBase(damageBase), range(range) {}

	template <typename Archive>
	void serialize(Archive& ar, const uint32_t version)
	{
		serializePrimitiveFields(ar, damageBase, range);
	}

private:
	uint8_t damageBase;
	uint8_t range;			// damage = <damageBase> modified using <range> and <distance>

	CREATE_ACCESSORS2(damageBase, range)
};
