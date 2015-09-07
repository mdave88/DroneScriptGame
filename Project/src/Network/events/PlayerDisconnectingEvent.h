
#ifndef PLAYER_DISCONNECTING_EVENT_H
#define PLAYER_DISCONNECTING_EVENT_H

#include <iostream>

#include "Network/NetworkObject.h"

namespace network
{
namespace events
{

class PlayerDisconnectingEvent : public NetworkObject
{
public:
	enum PlayerDCType { NETOBJ_PLAYER_DC = NETOBJ_NONE + 1000 };

	std::string	data;
	uint connectionID;


	PlayerDisconnectingEvent(uint connectionID = 0, const std::string& data = "") : NetworkObject(NETOBJ_PLAYER_DC), connectionID(connectionID), data(data) {}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;

		ar& connectionID;
		ar& data;
	}
};

} // namespace events
} // namespace network

//BOOST_CLASS_EXPORT(network::events::PlayerDisconnectingEvent);

#endif // PLAYER_DISCONNECTING_EVENT_H
