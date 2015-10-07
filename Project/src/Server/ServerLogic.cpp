#include "GameStdAfx.h"
#include "Server/Server.h"
#include "Common/LuaManager.h"
#include "Common/LoggerSystem.h"


namespace network
{

Server::Server(ushort port, short broadcastRate)
	: m_port(port)
	, m_broadcastRate(broadcastRate)
	, m_pServerHost(0)
	, m_disconnectingClient(0)
	, m_pFrontBuffer(nullptr)
	, m_isServerRunning(false)

	, m_lastBroadcastTime(0.0f)
	, m_lastAnimationTime(0.0f)
	, m_dt(0.0f)

	, m_isGamePaused(true)
	, m_pEngineCore(nullptr)
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
	ConstantManager::getInstance()->loadConstants(CONST_STR("dataDir") + "/settings/constants.xml");

	// open network log file (tries to find a new name for it)
	int networkLogFileIndex = 2;
	std::string networkLogFileName = "networklog";
	do
	{
		m_networkLog = std::ofstream(networkLogFileName + ".csv");
		networkLogFileName = "networklog";
		networkLogFileName += utils::intToStr(networkLogFileIndex++);
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
			m_lastAnimationTime = m_pEngineCore->getElapsedTime();
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

	m_dt = std::min(0.1f, (m_pEngineCore->getElapsedTime() - m_lastAnimationTime) / 200.0f);
	m_lastAnimationTime = m_pEngineCore->getElapsedTime();

	m_luaProcessingMutex.lock();

	m_pEngineCore->animate(m_dt);

	broadcast();

	m_luaProcessingMutex.unlock();
}

bool Server::isRunning() const
{
	return !m_isGamePaused;
}

} // namespace network
