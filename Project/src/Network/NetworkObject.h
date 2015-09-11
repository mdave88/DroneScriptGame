#pragma once

namespace network
{

/**
 * @brief Base class of the sendable network objects in the game engine.
 *
 * Contains only information about the type of the object.
 * Some of the child classes can be 'melted': the similar events following a meltable event can be ignored.
 */
class NetworkObject
{
public:
	enum NetworkObjectType { NETOBJ_NONE };

	ushort	type;
	bool	m_isUpdated;

public:
	NetworkObject(ushort type = NETOBJ_NONE) : type(type), m_isUpdated(true) {}

	virtual void set(const NetworkObject& other) {}

	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;
	}
};

} // namespace network
