#pragma once

#include <string>

struct Position
{
	Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

	float x;
	float y;
};

struct Direction
{
	Direction(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

	float x;
	float y;
};

struct Health
{
	Health(int health = 0) : health(health) {}

	int health;
};

struct ModuleBase
{
	int energyCostPerTurn;	// FuelModule
	int fuelCostPerTurn;	// EnergyModule
	bool isActive;
};

struct Battery : public ModuleBase
{
	int maxCapacity;
	int capacity;
};

struct Mobylity : public ModuleBase
{
	float maxSpeed;
	float speed;	// set by the drone
};

struct Memory : public ModuleBase
{
	int capacity;
	std::string contents;
};

struct Hdd : public ModuleBase
{
	int capacity;
	std::string contents;
};

struct Welder : public ModuleBase
{
	int maxStrength;
	int strength;
};

struct Jackhammer : public ModuleBase
{
	int maxStrength;
	int strength;
	int durability;
};

struct RadioTransmitter : public ModuleBase
{
	int maxStrength;
	int strength;
};

struct RadioReceiver : public ModuleBase
{

};

struct Sensor : public ModuleBase
{

};

struct Radar : public Sensor
{

};

struct Ladar : public Sensor
{

};

struct FuelCreator : public ModuleBase
{

};

struct Explosive
{
	float damageBase;
	int range;			// damage = <damageBase> modified using <range> and <distance>
};

class HandheldModule
{
public:

};
