#pragma once

#include "GameLogic/SerializationDefs.h"

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

struct Serializable
{
	uint8_t id;
	uint32_t attribmask;
	uint32_t attribIndex;

	NetworkPriority networkPriority;

	//template <typename Archive>
	//void load(Archive& ar, const uint8_t version) = 0;
	//template <typename Archive>
	//void save(Archive& ar, const uint8_t version) const = 0;
};

struct Movement : public Serializable
{
	Movement(vec2 pos = vec2(0.0f), vec2 dir = vec2(0.0f)) : pos(pos), dir(dir) {}
	
	vec2 pos;
	vec2 dir;
	float speed;
	//vec2 vel;

	template <typename Archive>
	void load(Archive& ar, const uint32_t version)
	{

	}

	template <typename Archive>
	void save(Archive& ar, const uint32_t version) const
	{

	}
};

struct Health
{
	Health(uint8_t maxHealth = 0, uint8_t health = 0) : maxHealth(maxHealth), health(health) {}

	const uint8_t maxHealth;
	uint8_t health;
};

struct Explosive
{
	float damageBase;
	uint8_t range;			// damage = <damageBase> modified using <range> and <distance>
};
