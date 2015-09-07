
#ifndef GAMESTATE_H
#define GAMESTATE_H

#define NOMINMAX

#include <enet/enet.h>

#include "NetworkObject.h"


class Player;
class NodeGroup;

namespace network
{

/**
 * @brief Contains the necessary informations about a client.
 *
 * Server side: contains the necessary informations about a client on the server.
 * Client side: contains the names of the clients/players in the game (only this field is serialized).
 *
 * Fields:
 *	- peer:			we can reach the client through this ENet object
 *	- player:		a pointer to the client's Player object in the scene
 *	- clientName:	the name of the client/player in the game
 *	- clientState:	the state of the client at the last broadcast/update
 */
struct ClientData : public NetworkObject
{
	enum ClientDataType { NETOBJ_CLIENTDATA = NETOBJ_NONE + 2 };

	ENetPeer*			m_pPeer;
	Player*				m_pPlayer;
	std::string			clientName;
	std::string			clientUsername;

	NodeIdDirectory		clientState;


	// serialization
	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;

		ar& clientName;
	}
};

// our clients in the game
typedef std::map<ushort, ClientData> ClientTable;


/**
 * @brief Handles the game state changes.
 *
 * The server fills it with the changes that occurred since the last broadcast/update and sends it to the client.
 * The client applies the changes to the game scene: creates, updates and destroys entities.
 * The changes can be filtered: different clients can see differently the BSP map.
 * The entities updated but invisible to the client can be filtered out.
 */
class GameState : public NetworkObject
{
public:
	enum GameStateType { NETOBJ_GAMESTATE = NETOBJ_NONE + 1 };

	GameState() : NetworkObject(NETOBJ_GAMESTATE), m_hasClientTableChanged(true) {}
	GameState(const ClientTable& clientTable, NodeIdDirectory& updatedEntities);
	GameState(const ClientTable& clientTable, NodeIdDirectory& updatedEntities, std::set<int>& deletedEntities);

	void				apply(NodeGroup* pRootNode, NodeIdDirectory& clientEntities);
	uint				calculateChanges(NodeIdDirectory& clientState, const NodeIdDirectory& serverState);

	NodeIdDirectory		applyFilter(const NodeIdDirectory& NodeDirectory);

	// getters-setters
	void				set(const ClientTable& clientTable, NodeIdDirectory& updatedEntities);
	void				set(const ClientTable& clientTable, NodeIdDirectory& updatedEntities, std::set<int>& deletedEntitiesById);

	void				setFilter(const std::set<ushort>& filter);

	const ClientTable&	getClientTable() const;
	void				setClientTable(const ClientTable& clientTable);

	ClientData&			getClient(const ushort id);


	// serialization
	template <typename Archive>
	void serialize(Archive& ar, const uint version);


private:
	ClientTable				m_clientTable;
	bool					m_hasClientTableChanged;

	NodeIdDirectory			m_updatedEntities;
	std::set<int>			m_deletedEntitiesById;

	std::set<ushort>		m_filter;				// filters the updateable entities
};

} // namespace network

//BOOST_CLASS_EXPORT_KEY(network::GameState);

#endif // GAMESTATE_HPP
