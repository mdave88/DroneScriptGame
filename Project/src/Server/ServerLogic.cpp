#include "GameStdAfx.h"
#include "Server/Server.h"

#include "GameLogic/NodeGroup.h"


namespace network
{

Server::Server(ushort port, short broadcastRate)
	:	m_port(port),
	    m_broadcastRate(broadcastRate),
	    m_pServerHost(0),
	    m_disconnectingClient(0),
	    m_pFrontBuffer(nullptr),
	    m_isServerRunning(false),

	    m_lastBroadcastTime(0.0f),
	    m_lastAnimationTime(0.0f),
	    m_dt(0.0f),

	    m_isGamePaused(true),
	    m_pEngineCore(nullptr)
{
	//pBackBuffer = &eventBuffer1;
	//pFrontBuffer = &eventBuffer2;
}

Server::~Server()
{
	m_networkLog.close();

	destroy();

	m_pEngineCore->release();

	EngineCore::destroyInstance();
	LuaManager::destroyInstance();
	ConstantManager::destroyInstance();
}

/**
 * Starts the server:
 *	- opens the network log file
 *	- initializes the engine core
 *	- initializes the ENet networking
 *	- starts the listening thread
 *	- starts the game loop
 */
void Server::start()
{
	new ConstantManager();
	ConstantManager::getInstance()->loadConstants(getDataDir() + "settings/constants.xml");

	// open network log file (tries to find a new name for it)
	int networkLogFileIndex = 2;
	std::string networkLogFileName = "networklog";
	do
	{
		m_networkLog = std::ofstream(networkLogFileName + ".csv");
		networkLogFileName = "networklog";
		networkLogFileName += utils::conversion::intToStr(networkLogFileIndex++);
	}
	while (!m_networkLog.is_open());

	m_networkLog << "text" << "\t" << "binary" << "\t" << "text comp" << "\t" << "binary comp" << "\t";
	m_networkLog << "text full" << "\t" << "binary full" << "\t" << "text full comp" << "\t" << "binary full comp" << "\t";
	m_networkLog << "changes" << std::endl;

	TRACE_NETWORK("Initializing game server.", 0);

	// initialize server
	initEngineCore();
	initNetwork(m_port);

	m_listenEventThread = boost::thread(boost::bind(&Server::listen, this));
	//processEventThread = boost::thread(boost::bind(&Server::processEvents, this));

	m_isServerRunning = true;
	while (m_isServerRunning)
	{
		// broadcast and run
		if (m_isGamePaused && m_clientTable.size() > 0)
		{
			m_lastAnimationTime = glutGet(GLUT_ELAPSED_TIME);
			m_isGamePaused = false;
		}

		if (!m_isGamePaused)
		{
			run();
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}

	m_listenEventThread.join();
	//processEventThread.join();
}

void Server::initEngineCore()
{
	srand(time(nullptr));

	new EngineCore();
	m_pEngineCore = EngineCore::getInstance();

	if (!m_pEngineCore->initLogic())
	{
		TRACE_ERROR("Error: Cannot initialize world.", 0);
		exit(EXIT_FAILURE);
	}
}

void Server::run()
{
	boost::mutex::scoped_lock lock(m_clientTableMutex);

	m_dt = (glutGet(GLUT_ELAPSED_TIME) - m_lastAnimationTime) / 200.0f;
	m_lastAnimationTime = glutGet(GLUT_ELAPSED_TIME);

	if (m_dt < 0.1f)
	{
		m_dt = 0.1;
	}

	// engineCore animate
	//physics::PhysicsManager::getInstance()->getPhysicsResults();

	//physics::PhysicsManager::getInstance()->releaseDeadActors();
	//CharacterController::releaseDeadControllers();

	m_luaProcessingMutex.lock();

	m_pEngineCore->animate(m_dt);
	//m_pEngineCore->getMap()->animate(m_dt);
	//m_pEngineCore->getRootNode()->animate(m_dt);

	broadcast();

	m_luaProcessingMutex.unlock();

	//physics::PhysicsManager::getInstance()->startPhysics(m_dt);
}

bool Server::isRunning() const
{
	return !m_isGamePaused;
}

} // namespace network





//network::Server* server = nullptr;

//void shutDown() {
//	//TRACE_NETWORK("Shutting down server.", 0);
//	SAFEDEL(server);
//}

///**
// * Prints the given std::string to the console. Can be called from lua scripts.
// * If the server is initialized and running the std::string is broadcasted to the clients (overloaded with different definitions on the server and client side).
// *
// * @param str The std::string to be printed/broadcasted.
// */
//void logToConsole(std::string str) {
//	if (server && server->isRunning()) {
//		TRACE_LUA("> " << str, 0);
//		std::string serialStr = marshal(network::events::LuaCommand(str));
//		ENetPacket* packet = enet_packet_create((enet_uint8*) serialStr.c_str(), serialStr.length(), ENET_PACKET_FLAG_RELIABLE);
//
//		enet_host_broadcast(server->getENetHost(), 0, packet);
//	} else {
//		TRACE_LUA("> " << str, 0);
//	}
//}

//int serverMain(int argc, char* argv[]) {
//	if (argc < 1)
//		return EXIT_FAILURE;
//
//	atexit(shutDown);
//
//	server = new network::Server(atoi(argv[2]), atoi(argv[3]));
//	server->start();
//
//	SAFEDEL(server);
//
//	return EXIT_SUCCESS;
//}
