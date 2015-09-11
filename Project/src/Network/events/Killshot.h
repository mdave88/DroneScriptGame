#define NOMINMAX

#pragma once

#include "Network/NetworkObject.h"


namespace network
{
namespace events
{

/**
 * @brief Holds the information about the shot that killed the unit.
 *
 * The effect of impact can be reproduced from this information on the client side (the client side does not simulate movement of the projectiles).
 */
class Killshot : public NetworkObject
{
public:
	enum InputEventType { NETOBJ_KILLSHOT = NETOBJ_NONE + 200 };

	std::string target;
	std::string actorName;
	vec3 pos, force;


public:
	Killshot(ushort type = NETOBJ_KILLSHOT) : NetworkObject(type) {}

	Killshot(const std::string& target, const std::string& actorName, const vec3& pos, const vec3& force, ushort type = NETOBJ_KILLSHOT)
		: NetworkObject(type)
		, target(target)
		, actorName(actorName)
		, pos(pos)
		, force(force)
	{
	}


	// needed for updateProperties
	bool operator!=(const Killshot& other) const
	{
		return (target != other.target || actorName != other.actorName || pos != other.pos || force != other.force);
	}

	Killshot& operator=(const Killshot& other)
	{
		if (this != &other)
		{
			target = other.target;
			actorName = other.actorName;
			pos = other.pos;
			force = other.force;
		}

		return *this;
	}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;

		ar& target;
		ar& actorName;
		ar& pos;
		ar& force;
	}
};


} // namespace events
} // namespace network

//BOOST_CLASS_EXPORT(network::events::Killshot);
