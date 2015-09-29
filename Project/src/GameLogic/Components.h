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

struct Movement
{
	Movement(float x = 0.0f, float y = 0.0f, float dirX = 0.0f, float dirY = 0.0f) : x(x), y(y), dirX(x), dirY(y) {}
	
	float x, y;
	float dirX, dirY;
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

struct Serializable
{
	uint8_t id;
	uint32_t attribmask;
	uint32_t attribIndex;

	NetworkPriority networkPriority;
};