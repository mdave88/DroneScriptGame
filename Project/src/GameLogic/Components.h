#pragma once

#include "GameLogic/SerializationDefs.h"
#include <bitset>

//struct Position
//{
//	Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
//
//	float x;
//	float y;
//};
//
//struct Direction
//{
//	Direction(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
//
//	float x;
//	float y;
//};


//#define CREATE_ACCESSOR(name, attribIndex)	const decltype(name)& get_##name##() { return name; } \
//											void set_##name##(const decltype(name)& newval) { name = newval; attribMask = (uint8_t)std::bitset<8>(attribMask).set(attribIndex).to_ulong(); }

#define CREATE_ACCESSOR(name, attribIndex)	public: \
											const decltype(name)& get_##name##() { return name; } \
											void set_##name##(const decltype(name)& newval) { name = newval; BIT_SET(attribMask, attribIndex); }

struct Serializable
{
	uint8_t id;
	std::bitset<64> attribMask;
	
	NetworkPriority networkPriority;

	//template <typename Archive>
	//void load(Archive& ar, const uint8_t version) = 0;
	//template <typename Archive>
	//void save(Archive& ar, const uint8_t version) const = 0;
};

struct PersistentComponent
{
	uint8_t attribMask;
};

class Movement : public PersistentComponent
{
public:
	static const int numPersistentAttribs = 3;

	Movement(vec2 pos = vec2(0.0f), vec2 dir = vec2(0.0f)) : pos(pos), dir(dir) {}

	template <typename Archive>
	void load(Archive& ar, const uint32_t version)
	{
		SER_P(pos);
		SER_P(dir);
		SER_P(speed);
	}

	template <typename Archive>
	void save(Archive& ar, const uint32_t version) const
	{
		SER_P(pos);
		SER_P(dir);
		SER_P(speed);
	}

private:
	vec2 pos;
	vec2 dir;
	float speed;
	//vec2 vel;

	CREATE_ACCESSOR(pos, 0);
	CREATE_ACCESSOR(dir, 1);
	CREATE_ACCESSOR(speed, 2);
};

class Health : public PersistentComponent
{
public:
	static const int numPersistentAttribs = 2;

	Health(uint8_t maxHealth = 0, uint8_t health = 0) : maxHealth(maxHealth), health(health) {}

	template <typename Archive>
	void load(Archive& ar, const uint32_t version)
	{
		SER_P(pos);
		SER_P(dir);
		SER_P(speed);
	}

	template <typename Archive>
	void save(Archive& ar, const uint32_t version) const
	{
		SER_P(pos);
		SER_P(dir);
		SER_P(speed);
	}

private:
	const uint8_t maxHealth;
	uint8_t health;

	//CREATE_ACCESSOR(maxHealth, 0);	// TODO: serialize consts only on creation
	CREATE_ACCESSOR(health, 1);
};

class Explosive
{
public:
	Explosive(uint8_t damageBase = 0, uint8_t range = 0) : damageBase(damageBase), range(range) {}

private:
	uint8_t damageBase;
	uint8_t range;			// damage = <damageBase> modified using <range> and <distance>
};
