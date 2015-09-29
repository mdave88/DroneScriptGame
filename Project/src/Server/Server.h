#pragma once

#define NOMINMAX

#include <enet/enet.h>
#include <boost/thread/thread.hpp>

#include "Network/GameState.h"
#include "Common/enginecore/EngineCore.h"


typedef std::map<std::string, entityx::Entity> NodeDirectory;

namespace network
{

/**
 * @brief The Server of the game.
 *
 * Controls the game logic and physical simulations apart from the ragdoll physics.
 */
class Server
{
public:
	Server(ushort port, short broadcastRate);
	~Server();

	void start();
	void destroy();

	bool isRunning() const;
	ENetHost* getENetHost() const;


private:
	// Server logic
	void initEngineCore();

	void run();

	// networking
	void initNetwork(ushort port);

	void listen();
	void processEvents();
	void processEvent(const std::string& stringData);

	void broadcast();
	void calculateStatistics(uint numUpdatedPackages, const ClientData& clientData);


private:
	EngineCore*					m_pEngineCore;

	// Server logic attributes
	float						m_dt;

	int							m_broadcastRate;
	long						m_lastBroadcastTime;

	long						m_lastAnimationTime;
	bool						m_isGamePaused;

	ClientTable					m_clientTable;


	// network attributes
	ushort						m_port;
	ENetAddress					m_address;
	ENetHost*					m_pServerHost;
	ENetEvent					m_event;

	std::ofstream				m_networkLog;

	GameState					m_package;
	NodeDirectory				m_serverState;

	uint						m_disconnectingClient;


	// threads
	bool						m_isServerRunning;
	std::vector<ENetEvent>		m_eventBuffer1, m_eventBuffer2;
	std::vector<ENetEvent>*		m_pFrontBuffer;
	//std::vector<ENetEvent>*		m_pBackBuffer;
	//std::vector<ENetEvent>*		m_pSwapper;

	boost::thread				m_listenEventThread;
	boost::thread				m_processEventThread;

	boost::mutex				m_clientTableMutex;
	boost::mutex				m_eventBufferMutex;
	boost::mutex				m_luaProcessingMutex;
};
} // namespace network
