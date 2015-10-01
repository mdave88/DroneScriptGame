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

	template <typename Archive>
	void load(Archive& ar, const uint version) = 0;
	template <typename Archive>
	void save(Archive& ar, const uint version) const = 0;
};

struct Movement : public Serializable
{
	Movement(vec2 pos = vec2(0.0f), vec2 dir = vec2(0.0f)) : pos(pos), dir(dir) {}
	
	vec2 pos;
	vec2 dir;
	float speed;
	//vec2 vel;

	template <typename Archive>
	void load(Archive& ar, const uint version)
	{

	}

	template <typename Archive>
	void save(Archive& ar, const uint version) const
	{

	}
};

struct Health
{
	Health(int health = 0) : health(health) {}

	int health;
};

struct Explosive
{
	float damageBase;
	int range;			// damage = <damageBase> modified using <range> and <distance>
};
