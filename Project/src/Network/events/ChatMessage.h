
#define NOMINMAX

#ifndef CHAT_EVENT_H
#define CHAT_EVENT_H

#include "Network/NetworkObject.h"

namespace network
{
namespace events
{

/**
 * @brief Contains a chat message.
 *
 * Contains a string that will be sent to every client excluding the sender.
 */
class ChatMessage : public NetworkObject
{
public:
	enum ChatMessageType { NETOBJ_CHATMSG = (int)NETOBJ_NONE + 400 };

	std::string message;

public:
	ChatMessage(const std::string& message = "") : NetworkObject(NETOBJ_CHATMSG), message(message) {}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;
		ar& message;
	}
};

} // namespace events
} // namespace network

//BOOST_CLASS_EXPORT_KEY(network::events::ChatMessage);

#endif // CHAT_EVENT_H
