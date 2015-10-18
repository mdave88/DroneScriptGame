#pragma once

#include "GameLogic/Components.h"


enum class ModuleType
{
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

struct ModuleBase : public ComponentBase
{
	ModuleBase(uint8_t energyCostPerTurn = 0, uint8_t fuelCostPerTurn = 0, bool isActive = false)
		: energyCostPerTurn(energyCostPerTurn)
		, fuelCostPerTurn(fuelCostPerTurn)
		, isActive(isActive)
	{
	}

	uint8_t energyCostPerTurn;	// FuelModule
	uint8_t fuelCostPerTurn;	// EnergyModule
	bool isActive;


	SERIALIZABLE_CLASS;
	SERIALIZE3(energyCostPerTurn, fuelCostPerTurn, isActive);
};

struct Battery : public ModuleBase
{
	uint8_t maxCapacity;
	uint8_t capacity;


	SERIALIZABLE_CLASS;
	SERIALIZE2(maxCapacity, capacity);
};

struct Mobility : public ModuleBase
{
	float maxSpeed;
	float speed;	// set by the drone


	SERIALIZABLE_CLASS;
	SERIALIZE2(maxSpeed, speed);
};

struct Memory : public ModuleBase
{
	uint8_t capacity;
	std::string contents;
};

struct Hdd : public ModuleBase
{
	uint8_t capacity;
	std::string contents;
};

struct Welder : public ModuleBase
{
	uint8_t maxStrength;
	uint8_t strength;
};

struct Jackhammer : public ModuleBase
{
	uint8_t maxStrength;
	uint8_t strength;
	uint8_t durability;
};

struct RadioTransmitter : public ModuleBase
{
	uint8_t maxStrength;
	uint8_t strength;
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
