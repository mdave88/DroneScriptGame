#pragma once

#include "GameLogic/Components.h"


enum class ModuleType
{
	Battery,
	Mobylity,
	Memory,
	Hdd,
	Welder,
	Jackhammer,
	RadioTransmitter,
	RadioReceiver,
	Radar,
	Ladar,
	FuelCreator,
};

struct ModuleBase
{
	ModuleBase(int energyCostPerTurn = 0, int fuelCostPerTurn = 0, bool isActive = false)
		: energyCostPerTurn(energyCostPerTurn)
		, fuelCostPerTurn(fuelCostPerTurn)
		, isActive(isActive)
	{
	}

	int energyCostPerTurn;	// FuelModule
	int fuelCostPerTurn;	// EnergyModule
	bool isActive;
};

struct Battery : public ModuleBase
{
	int maxCapacity;
	int capacity;
};

struct Mobility : public ModuleBase
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
