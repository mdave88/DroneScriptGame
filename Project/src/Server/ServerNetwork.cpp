#include "GameStdAfx.h"
#include "Server/Server.h"

#include "Common/LuaManager.h"

#include "Graphics/Camera.h"

#include "Network/connection.h"
#include "Network/events/KeyEvent.h"
#include "Network/events/MouseEvent.h"
#include "Network/events/LuaCommand.h"
#include "Network/events/ChatMessage.h"
#include "Network/events/Killshot.h"
#include "Network/events/PlayerReadyEvent.h"
#include "Network/events/PlayerDisconnectingEvent.h"


// registering the serialized classes (needed for pointer types)
BOOST_CLASS_EXPORT(network::events::InputEvent);
BOOST_CLASS_EXPORT(network::events::KeyEvent);
BOOST_CLASS_EXPORT(network::events::MouseEvent);
BOOST_CLASS_EXPORT(network::events::LuaCommand);
BOOST_CLASS_EXPORT(network::events::ChatMessage);
BOOST_CLASS_EXPORT(network::events::PlayerReadyEvent);
BOOST_CLASS_EXPORT(network::events::PlayerDisconnectingEvent);


namespace network
{

/**
 * Initializes the server on the given port.
 * (Max 32 clients, 2 channels)
 *
 * @param port The port the server will listen to the clients.
 */
void Server::initNetwork(ushort port)
{
	TRACE_NETWORK("Starting server", 0);

	if (enet_initialize () != 0)
	{
		TRACE_ERROR("Error: Error initialising enet.", 0);
		exit(EXIT_FAILURE);
	}

	/* Bind the server to the default localhost.     */
	/* A specific host m_address can be specified by   */
	/* enet_address_set_host (& m_address, "x.x.x.x"); */
	m_address.host = ENET_HOST_ANY;
	m_address.port = port;

	m_pServerHost = enet_host_create (&m_address,
	                                  32,   /* number of clients */
	                                  2,    /* number of channels */
	                                  0,    /* Any incoming bandwidth */
	                                  0);   /* Any outgoing bandwidth */

	if (m_pServerHost == nullptr)
	{
		TRACE_ERROR("Error: Could not create server host.", 0);
		exit(EXIT_FAILURE);
	}

	//// initialize EventManager
	//network::events::EventManager::getInstance(m_serverHost);
}


/**
 * Listens to the clients (and put the received packages in the "back" buffer).
 * The connection and disconnection requests are also handled here.
 * (Running in listenEventThread)
 */
void Server::listen()
{
	std::string stringData;

	int startTime = 0;
	int processingTime = 0;

	int serviceResult;

	while (m_isServerRunning)
	{
		serviceResult = 1;

		startTime = m_pEngineCore->getElapsedTime();
		processingTime = 0;

		do
		{
			serviceResult = enet_host_service(m_pServerHost, &m_event, 1);

			if (serviceResult > 0)
			{
				switch (m_event.type)
				{
					case ENET_EVENT_TYPE_CONNECT:
						TRACE_NETWORK("A new client connected from " << m_event.peer->address.host << ":" << m_event.peer->address.port, 0);

						TRACE_NETWORK("Client information: " << m_event.peer->data, 0);
						m_event.peer->data = (void*) m_event.peer->connectID;

						TRACE_NETWORK("---------------------------------------------", 0);
						TRACE_NETWORK("Client connected with ID: " << m_event.peer->connectID, 0);
						TRACE_NETWORK("---------------------------------------------", 0);

						m_clientTable[m_event.peer->connectID].m_pPlayer = nullptr;
						m_clientTable[m_event.peer->connectID].m_pPeer = m_event.peer;

						break;

					case ENET_EVENT_TYPE_RECEIVE:
						stringData = std::string((char*) m_event.packet->data, m_event.packet->dataLength);

						processEvent(stringData);
						//pBackBuffer->push_back(m_event);

						//m_eventBufferMutex.lock();
						//if (m_pEngineCore->getElapsedTime() - startTime > 10) {
						//	pSwapper = pBackBuffer;
						//	pBackBuffer = m_pFrontBuffer;
						//	m_pFrontBuffer = pSwapper;

						//	//startTime = m_pEngineCore->getElapsedTime();
						//}
						//m_eventBufferMutex.unlock();

						break;

					case ENET_EVENT_TYPE_DISCONNECT:
						m_disconnectingClient = (enet_uint32) m_event.peer->data;
						TRACE_NETWORK("---------------------------------------------", 0);
						TRACE_NETWORK("Client disconnected: " << m_disconnectingClient, 0);
						TRACE_NETWORK("Peer data:  " << m_event.peer->data, 0);
						TRACE_NETWORK("---------------------------------------------", 0);

						TRACE_NETWORK(m_event.channelID, 0);

						if (m_disconnectingClient > 0)
						{
							// TODO: use std lock
							boost::mutex::scoped_lock lock(m_clientTableMutex);
							///m_pEngineCore->getRootNode()->removeByName(m_clientTable.at(m_disconnectingClient).clientName);
							m_clientTable.erase(m_disconnectingClient);
							TRACE_NETWORK("Client erased from client list.", 0);
						}

						if (m_clientTable.size() < 1)
						{
							m_isGamePaused = true;
						}

						/* Reset the peer's client information. */
						m_event.peer->data = nullptr;

						break;
				}
			}

			processingTime = m_pEngineCore->getElapsedTime() - startTime;
		}
		while (serviceResult > 0 && processingTime < 10);

		// sleep
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
}

/**
 * Processes the events from the "front" buffer.
 * (Running in processEventThread)
 */
void Server::processEvents()
{
	ENetEvent m_event;
	std::string stringData;

	while (m_isServerRunning)
	{
		m_clientTableMutex.lock();
		m_eventBufferMutex.lock();

		// iterate over the front buffer
		for (std::vector<ENetEvent>::iterator it = m_pFrontBuffer->begin(); it < m_pFrontBuffer->end(); ++it)
		{
			m_event = *it;
			stringData = std::string((char*) m_event.packet->data, m_event.packet->dataLength);

			processEvent(stringData);
		}
		m_pFrontBuffer->clear();
		m_eventBufferMutex.unlock();
		m_clientTableMutex.unlock();

		// sleep
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
}

/**
 * Processes a serialized NetworkObject.
 *
 * @param stringData The serialized form of the NetworkObject.
 */
void Server::processEvent(const std::string& stringData)
{
	NetworkObject netObject;
	if (unmarshal(netObject, stringData))
	{
		switch (netObject.type)
		{
			// player disconnecting
			case events::PlayerReadyEvent::NETOBJ_PLAYER_READY:
			{
				events::PlayerReadyEvent playerReadyEvent;
				if (unmarshal(playerReadyEvent, stringData))
				{
					TRACE_NETWORK("PlayerReadyEvent received.", 0);
					m_clientTable.at(m_event.peer->connectID).clientUsername = playerReadyEvent.name;
				}
			}
			break;

			// player disconnecting
			case events::PlayerDisconnectingEvent::NETOBJ_PLAYER_DC:
			{
				events::PlayerDisconnectingEvent disconnectingEvent;
				if (unmarshal(disconnectingEvent, stringData))
				{
					TRACE_NETWORK("DisconnectingEvent received.", 0);
					m_disconnectingClient = disconnectingEvent.connectionID;

					enet_host_broadcast(m_pServerHost, 0, m_event.packet);
				}
			}
			break;

			// keyEvent received
			case events::KeyEvent::NETOBJ_KEY_DOWN:
			case events::KeyEvent::NETOBJ_KEY_UP:
			{
				events::KeyEvent keyEvent;
				if (unmarshal(keyEvent, stringData))
				{
					///m_clientTable.at(m_event.peer->connectID).m_pPlayer->setKeyState(keyEvent.keyCode, keyEvent.type == events::KeyEvent::NETOBJ_KEY_DOWN);
				}
			}
			break;

			// mouseEvent received
			case events::MouseEvent::NETOBJ_MOUSE_MOVE:
			case events::MouseEvent::NETOBJ_MOUSE_ACTION:
			case events::MouseEvent::NETOBJ_MOUSE_DRAG:
			{
				events::MouseEvent mouseEvent;
				if (unmarshal(mouseEvent, stringData))
				{
					///m_clientTable.at(m_event.peer->connectID).m_pPlayer->setMouseState(mouseEvent);
				}
			}
			break;

			// lua command received
			case events::LuaCommand::NETOBJ_LUACOMM:
			{
				events::LuaCommand luaCommand;
				if (unmarshal(luaCommand, stringData))
				{
					if (luaCommand.command == "quit")
					{
						m_isServerRunning = false;
					}
					else if (luaCommand.command == "state")
					{
						TRACE_LUA("------------------------------------------------------------------", 0);
						///
						//for (const auto& entry : m_pEngineCore->getNodeIdDirectory())
						//{
						//	TRACE_LUA(entry.first << "\t(" << entry.second->getId() << ")\t\t" << entry.second->getName(), 0);
						//}
					}
					else if (luaCommand.command.find("speed") != std::string::npos)
					{
						float speedMultiplier;
						sscanf(luaCommand.command.c_str(), "speed %f", &speedMultiplier);
						ConstantManager::getInstance()->setFloatConstant("Gameplay::GameSpeedMultiplier", speedMultiplier);
					}
					else
					{
						boost::mutex::scoped_lock lock(m_luaProcessingMutex);
						LuaManager::getInstance()->doString(luaCommand.command);
					}
				}
			}
			break;

			// chat message received
			case events::ChatMessage::NETOBJ_CHATMSG:
			{
				events::ChatMessage chatMessage;
				if (unmarshal(chatMessage, stringData))
				{

					if (m_isServerRunning)
					{
						std::stringstream fullMessage;
						fullMessage << m_clientTable.at(m_event.peer->connectID).clientUsername;
						fullMessage << ": ";
						fullMessage << chatMessage.message;

						const std::string serialStr = marshal(network::events::ChatMessage(fullMessage.str()));

						ENetPacket* packet = enet_packet_create((enet_uint8*)serialStr.c_str(), serialStr.length(), ENET_PACKET_FLAG_RELIABLE);

						enet_host_broadcast(m_pServerHost, 0, packet);
					}
					else
					{
						TRACE_ERROR("Error: cannot send chat message - server is not running.", 0);
					}
				}
			}
			break;
		}
	}
}

/**
 * Broadcasts the changes to the clients.
 *
 * Calculate the changes on the server side, compress the data and send it to the clients.
 * We compare the client and the current state on the server and update the client state according to it:
 *	- adding new entities to the clientState
 *	- updating the entities that changed state
 */
void Server::broadcast()
{
	if (m_pEngineCore->getElapsedTime() - m_lastBroadcastTime < m_broadcastRate)
	{
		return;
	}

	///m_pEngineCore->getCamera()->setPerspective(45, 800, 600, 1.0f, 400.0f);
	//m_pEngineCore->getMap()->updateObservers(m_pEngineCore->getCamera());
	//m_serverState = getNodeDirectory();

	///
	//const NodeIdDirectory& serverState = m_pEngineCore->getNodeIdDirectory();

	//// for every client: calculate the changes and send it to client
	//for (auto& entry : m_clientTable)
	//{
	//	// set the filter for the clients observer
	//	//const std::set<ushort>& filter = m_pEngineCore->getMap()->getPerceptibleEntitiesByObserver(entry.first);
	//	//m_package.setFilter(filter);

	//	// TODO: Extend the filter with the priority filtering.
	//	// Priority could be circulated, like: prio-- -> set to max when reaches 0 : obj_prio > prio -> part of the collection

	//	const uint numUpdatedPackages = m_package.calculateChanges(entry.second.clientState, serverState);
	//	m_package.setClientTable(m_clientTable);

	//	//calculateStatistics(numUpdatedPackages, it->second);

	//	send(m_package, entry.second.m_pPeer);
	//}

	m_lastBroadcastTime = m_pEngineCore->getElapsedTime();
}

/**
 * Calculate the size of the packages made by different packaging mechanisms.
 * @param numUpdatedPackages	the number of packages updated for the client.
 * @param clientData			the data for the current client.
 */
void Server::calculateStatistics(uint numUpdatedPackages, const ClientData& clientData)
{
	// statistic data
	if (1 < numUpdatedPackages)
	{
		const std::string serialStrText = marshalText(m_package);
		const std::string serialStrBinary = marshalBinary(m_package);

		const std::string serialStrTextComp = marshalText(m_package, 1);
		const std::string serialStrBinaryComp = marshalBinary(m_package, 1);

		// marshal full state
		NodeIdDirectory changedEntities = clientData.clientState;
		{
			for (auto& entry : changedEntities)
			{
				auto& entity = entry.second;
				///
				//entity = EntityPtr((Entity*)entity->clone());
				//entity->setAttribIndex(0);
				//entity->setAttribMask(INT_MAX);
			}
		}

		GameState packageTmp;
		packageTmp.set(m_clientTable, changedEntities);
		const std::string serialStrTextFull = marshalText(packageTmp);
		const std::string serialStrBinaryFull = marshalBinary(packageTmp);

		const std::string serialStrTextCompFull = marshalText(packageTmp, 1);
		const std::string serialStrBinaryCompFull = marshalBinary(packageTmp, 1);

		// create csv file from the network usage
		m_networkLog << serialStrText.size() << "\t" << serialStrBinary.size() << "\t" << serialStrTextComp.size() << "\t" << serialStrBinaryComp.size() << "\t";
		m_networkLog << serialStrTextFull.size() << "\t" << serialStrBinaryFull.size() << "\t" << serialStrTextCompFull.size() << "\t" << serialStrBinaryCompFull.size() << "\t";
		m_networkLog << numUpdatedPackages << std::endl;
	}
}

/**
 * Destroys the server and deinitializes the ENet context.
 */
void Server::destroy()
{
	if (m_pServerHost)
	{
		enet_host_destroy(m_pServerHost);
		enet_deinitialize();
	}
}


ENetHost* Server::getENetHost() const
{
	return m_pServerHost;
}

} // namespace network
