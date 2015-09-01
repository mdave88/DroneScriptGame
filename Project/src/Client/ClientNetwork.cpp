#include "GameStdAfx.h"
#include "Client/Client.h"

#include "GameLogic/Player.h"

// registering the serialized classes (needed for pointer types)
//BOOST_CLASS_EXPORT_IMPLEMENT(Node);
//BOOST_CLASS_EXPORT_IMPLEMENT(NodeGroup);
//BOOST_CLASS_EXPORT_IMPLEMENT(Entity);
//BOOST_CLASS_EXPORT_IMPLEMENT(Door);
//BOOST_CLASS_EXPORT_IMPLEMENT(LightSource);
//


namespace network
{

/**
 * Initializes the client and tries to connect to the server on the given m_address and port.
 *
 * @param host The m_address of the server.
 * @param port The port the server will listen to the clients.
 */
bool Client::connect(const std::string& host, uint port)
{
	TRACE_NETWORK("Starting client.", 0);

	if (enet_initialize () != 0)
	{
		TRACE_ERROR("Error: Error initializing enet.", 0);
		exit(EXIT_SUCCESS);
	}

	m_pClientHost = enet_host_create (nullptr,	/* create a client host */
	                                  1,									/* number of clients */
	                                  2,									/* number of channels */
	                                  0,//57600 / 8,							/* incoming bandwidth */
	                                  0);//14400 / 8);							/* outgoing bandwidth */

	if (m_pClientHost == nullptr)
	{
		TRACE_ERROR("Error: Could not create client host.", 0);
		return false;
	}


	enet_address_set_host(&m_address, host.c_str());
	m_address.port = port;

	m_pPeer = enet_host_connect(m_pClientHost,
	                            &m_address,							/* address to connect to */
	                            2,									/* number of channels */
	                            10);								/* user data supplied to the receiving host */

	if (m_pPeer == nullptr)
	{
		TRACE_ERROR("Error: No available peers for initiating an ENet connection.", 0);
		return false;
	}

	TRACE_NETWORK("---------------------------------------------", 0);

	int connectionAttempts = 5;
	while (connectionAttempts > 0)
	{
		// Try to connect to server within 1000 ms
		if (enet_host_service(m_pClientHost, &m_event, 1000) > 0 && m_event.type == ENET_EVENT_TYPE_CONNECT)
		{
			TRACE_NETWORK("Connection to server succeeded.", 0);
			TRACE_NETWORK("clientID (peer): " << m_pPeer->connectID, 0);

			m_clientId = (short) m_pPeer->outgoingPeerID;
			TRACE_NETWORK("client num: " << m_clientId << std::endl, 0);

			connectionAttempts = 0;
		}
		connectionAttempts--;
	}

	// could not connect to server
	if (m_event.type != ENET_EVENT_TYPE_CONNECT)
	{
		/* Either the 5 seconds are up or a disconnect m_event was */
		/* received. Reset the peer in the m_event the 5 seconds   */
		/* had run out without any significant m_event.            */
		enet_peer_reset(m_pPeer);
		enet_host_destroy(m_pClientHost);
		enet_deinitialize();

		TRACE_ERROR("Error: Connection to server failed.", 0);
		TRACE_NETWORK("---------------------------------------------", 0);

		return false;
	}

	TRACE_NETWORK("---------------------------------------------", 0);

	events::PlayerReadyEvent playerReadyEvent(m_clientName);
	network::send(playerReadyEvent, m_pPeer);

	return true;
}

/**
 * Listens to the packages sent by the server.
 *	- if the m_package is a GameState object: applies the changes to the scene.
 *	- if the m_clientTable in the m_package has changed: stores the updated one
 *	- if its the first run: replaces the dummy player with the real one created on the server
 *	- if its a lua command: it is a response to a lua command sent to the server earlier
 */
void Client::listen()
{
	if (m_clientId == k_clientIdNone)
	{
		return;
	}

	m_serviceResult = 1;

	do
	{
		m_serviceResult = enet_host_service(m_pClientHost, &m_event, 1);

		if (m_serviceResult > 0)
		{
			switch (m_event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
					TRACE_NETWORK("A new client connected from " << m_event.peer->address.host << ":" << m_event.peer->address.port, 0);
					m_event.peer->data = (void*)"New User";

					break;

				case ENET_EVENT_TYPE_RECEIVE:
					if (unmarshal(m_netObject, std::string((char*) m_event.packet->data, m_event.packet->dataLength)))
					{
						switch (m_netObject.type)
						{
							// getting back escape char
							case events::KeyEvent::NETOBJ_KEY_DOWN:
								//disconnect();

								break;

							case GameState::NETOBJ_GAMESTATE:
								if (unmarshal(m_package, std::string((char*) m_event.packet->data, m_event.packet->dataLength)))
								{
									// apply the changes to our entities
									m_package.apply(m_pEngineCore->getRootNode(), EngineCore::getInstance()->getNodeIdDirectory());

									// if the clientList changed -> save it to the local variable
									if (!m_package.getClientTable().empty())
									{
										m_clientTable = m_package.getClientTable();
									}

									if (m_pEngineCore->getPlayer()->getName().empty())
									{
										// replace the dummy player with the real one
										if (m_pEngineCore->getNodeDirectory().find(m_clientTable[m_pPeer->connectID].clientName) != m_pEngineCore->getNodeDirectory().end())
										{
											// remove the dummy player from the bsp map
											//m_pEngineCore->getMap()->clearObservers();
											// delete dummy player
											delete m_pEngineCore->getPlayer();
											m_pEngineCore->setPlayer(std::static_pointer_cast<Player>(m_pEngineCore->getNodeDirectory().at(m_clientTable[m_pPeer->connectID].clientName)).get());

											// add the player to the bsp map
											//m_pEngineCore->getMap()->addObserver(m_clientId, m_pEngineCore->getPlayer());
										}
									}
								}
								m_gamePaused = false;

								break;

							case events::LuaCommand::NETOBJ_LUACOMM:
								if (unmarshal(m_luaResponse, std::string((char*) m_event.packet->data, m_event.packet->dataLength)))
								{
									logToConsole(m_luaResponse.command);
								}

								break;

							case events::ChatMessage::NETOBJ_CHATMSG:
								if (unmarshal(m_chatMessage, std::string((char*) m_event.packet->data, m_event.packet->dataLength)))
								{
									printToChatHistory(m_chatMessage.message);
								}

								break;
						}
					}

					enet_packet_destroy(m_event.packet);

					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					TRACE_NETWORK(m_event.peer->data << " disconnected.", 0);

					break;

				default:
					GX_ASSERT(0 && "Error: unknown event type received.");
					//TRACE_ERROR("Error: unknown event type received.", 0);
			}
		}
	}
	while (m_serviceResult > 0);
}

/**
 * Disconnects the client from the server, deinitializes ENet and releases the engine core.
 * Allow up to 3 seconds for the disconnect to succeed and drop any packets received packets.
 */
void Client::disconnect()
{
	// Send disconnecting request
	m_disconnectingEvent.connectionID = m_pPeer->connectID;
	network::send(m_disconnectingEvent, m_pPeer);

	enet_host_flush(m_pClientHost);


	//// Waiting for ACK
	//bool acked = false;
	//int startTime = glutGet(GLUT_ELAPSED_TIME);

	//while (glutGet(GLUT_ELAPSED_TIME) - startTime < 3000 && enet_host_service(m_clientHost, &m_event, 3000) > 0) {
	//	switch (m_event.type) {
	//		case ENET_EVENT_TYPE_RECEIVE:

	//			if (unmarshal(m_netObject, std::string((char*) m_event.packet->data, m_event.packet->dataLength))) {
	//				if (m_netObject.type == events::PlayerDisconnectingEvent::NETOBJ_PLAYER_DC) {
	//					if (unmarshal(m_disconnectingEvent, std::string((char*) m_event.packet->data, m_event.packet->dataLength))) {
	//						if (m_disconnectingEvent.connectionID == m_peer->connectID) {
	//							TRACE_NETWORK("Disconnection ACK-ed.", 0);
	//							acked = true;
	//						}
	//					}
	//				}
	//			}

	//			enet_packet_destroy(m_event.packet);

	//			break;
	//	}
	//}

	enet_peer_disconnect(m_pPeer, 0);

	// Allow up to 3 seconds for the disconnect to succeed and drop any packets received packets
	while (enet_host_service(m_pClientHost, &m_event, 3000) > 0)
	{
		switch (m_event.type)
		{
			case ENET_EVENT_TYPE_RECEIVE:
				if (unmarshal(m_disconnectingEvent, std::string((char*) m_event.packet->data, m_event.packet->dataLength)))
					if (m_disconnectingEvent.connectionID == m_pPeer->connectID)
					{
						TRACE_NETWORK("Disconnection ACK-ed.", 0);
					}

				enet_packet_destroy(m_event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				TRACE_NETWORK("Disconnection succeeded.", 0);
				break;
		}
	}


	enet_host_destroy(m_pClientHost);
	enet_deinitialize();
}

void Client::flushPackets()
{
	//if (getApplicationSide() != APPSIDE_CLIENT)
	//{
	//	return;
	//}

	enet_host_flush(m_pClientHost);
	enet_host_service(m_pClientHost, &m_event, 100);
}

ENetPeer* Client::getPeer() const
{
	return m_pPeer;
}

short Client::getClientId() const
{
	return m_clientId;
}

} // namespace network
