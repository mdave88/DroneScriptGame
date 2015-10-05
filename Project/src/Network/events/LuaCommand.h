#define NOMINMAX

#pragma once

#include "Network/NetworkObject.h"
#include <string>

namespace network
{
namespace events
{

/**
 * @brief Contains a lua command.
 *
 * Contains a string that will be added to the lua interpreter on the server side.
 * The output is sent back to the client in another LuaCommand object.
 */
class LuaCommand : public NetworkObject
{
public:
	enum LuaCommandType { NETOBJ_LUACOMM = NETOBJ_NONE + 300 };

	std::string command;

public:
	LuaCommand(const std::string& command = "") : NetworkObject(NETOBJ_LUACOMM), command(command) {}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;
		ar& command;
	}
};

} // namespace events
} // namespace network

//BOOST_CLASS_EXPORT(network::events::LuaCommand);
