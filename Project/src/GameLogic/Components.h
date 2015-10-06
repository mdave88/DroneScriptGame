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
	void serialize(Archive& ar, const uint version)
	{
		BIT_CLEAR(m_attribMask, 7);

		// mark the direction on the most significant bit on the attrib mask: 0 - save, 1 - load
		boost::serialization::split_member(ar, *this, version);
		m_attribIndex = 0;

		// network priority can change on the fly -> compressions can vary
		SER_P(m_networkPriority);
	}

	template <typename Archive>
	void save(Archive& ar, const uint version) const
	{
		ar << m_attribMask;
	}

	void finalizeSave()
	{
		m_attribMask = 0;
		m_attribIndex = 0;
	}

	template <typename Archive>
	void load(Archive& ar, const uint version)
	{
		ar >> m_attribMask;

		// mark the direction -> you may need to reset it if the return value updateProperties() is significant on the client
		BIT_SET(m_attribMask, 7);
	}


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
};

class Movement : public PersistentComponent
{
public:
	Movement(vec2 pos = vec2(0.0f), vec2 vel = vec2(0.0f)) : pos(pos), vel(vel) {}

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
	Health(uint8_t maxHealth = 0, uint8_t health = 0) : maxHealth(maxHealth), health(health) {}

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

class Explosive : public PersistentComponent
{
public:
	Explosive(uint8_t damageBase = 0, uint8_t range = 0) : damageBase(damageBase), range(range) {}

	template <typename Archive>
	void serialize(Archive& ar, const uint32_t version)
	{
		ar& boost::serialization::base_object<PersistentComponent>(*this);
		serializePrimitiveFields(ar, damageBase, range);
	}

private:
	uint8_t damageBase;
	uint8_t range;			// damage = <damageBase> modified using <range> and <distance>

	CREATE_ACCESSORS2(damageBase, range)
};
