
#ifndef PLAYER_READY_EVENT_H
#define PLAYER_READY_EVENT_H

#include <iostream>

#include "Network/NetworkObject.h"

namespace network
{
namespace events
{

class PlayerReadyEvent : public NetworkObject
{
public:
	enum PlayerReadyType { NETOBJ_PLAYER_READY = NETOBJ_NONE + 1100 };

	std::string name;

	PlayerReadyEvent(const std::string& name = "johnDoe") : NetworkObject(NETOBJ_PLAYER_READY), name(name) {}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;

		ar& name;
	}
};

} // namespace events
} // namespace network

//BOOST_CLASS_EXPORT(network::events::PlayerReadyEvent);

#endif // PLAYER_READY_EVENT_H