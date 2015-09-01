#include "GameStdAfx.h"
#include "Client/Client.h"

// OpenGL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "Console/CrimsonConsole.h"

#include "GameLogic/NodeGroup.h"


namespace network
{

const short Client::k_clientIdNone = -1;
const short Client::k_clientIdLocalFull = 1000;


Client::Client(const bool isThickClient)
	:	m_dt(0.0f),
		m_lastRenderTime(0.0f),
		m_gamePaused(true),
		m_processInput(true),
		m_pEngineCore(nullptr),

#ifdef ENABLE_MYGUI
		m_pGui(nullptr),
		m_pGuiPlatform(nullptr),
		m_pGuiImageLoader(nullptr),
#endif
		m_isGuiOpened(false),

		m_pPeer(nullptr),
		m_pClientHost(nullptr),
		m_serviceResult(1)
{
	if (!isThickClient)
	{
		m_clientId = k_clientIdNone;
	}
	else
	{
		m_clientId = k_clientIdLocalFull;
	}
}

Client::~Client()
{
	release();
}

void Client::release()
{
	releaseGui();

	m_pGameConsole->releaseKeys();
	delete m_pGameConsole;

	ConstantManager::destroyInstance();

	disconnect();

	m_pEngineCore->release();
	EngineCore::destroyInstance();
	LuaManager::destroyInstance();
}

// init
bool Client::init(Configs& configs, const std::string& hostAddress, uint port, bool instantStart)
{
	m_configs = configs;
	m_hostAddress = hostAddress;
	m_hostPort = port;

	new ConstantManager();
	ConstantManager::getInstance()->loadConstants(getDataDir() + "settings/constants.json");

	
	if (!initConsole())
	{
		return false;
	}

	// initialize mygui
	if (!initGui(instantStart))
	{
		return false;
	}

	// if -instant is added to the arguments -> start the game without the pre-game gui
	if (instantStart)
	{
		if (!initEngineCore())
		{
			return false;
		}

		// initialize the in-game gui
		glutReshapeWindow(m_configs.width, m_configs.height);

		initGameGui();
		return true;
	}

	return true;
}

bool Client::initConsole()
{
	m_pGameConsole = new GameConsole();
	m_pGameConsole->init(m_configs.width, m_configs.height);
	return true;
}

void Client::initGraphics()
{
	glutSetCursor(GLUT_CURSOR_NONE);

	srand(time(nullptr));

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

bool Client::initEngineCore()
{
	new EngineCore();

	// connect to server
	if (!connect(m_hostAddress, m_hostPort))
	{
		return false;
	}

	listen();

	// if the game is tested on the localhost, place it in different positions on the screen
	if (m_hostAddress == "localhost")
	{
#ifndef SERVER_SIDE
		glutPositionWindow((m_clientId % 2) * m_configs.width + 5, glutGet(GLUT_SCREEN_HEIGHT) - (m_clientId / 2 + 1) * m_configs.height - 50);
#else
		glutPositionWindow(glutGet(GLUT_SCREEN_WIDTH) - m_configs.width + 5, glutGet(GLUT_SCREEN_HEIGHT) - m_configs.height - 50);
#endif
	}

	setLoadingProgress(5, "Initializing physx..");

	m_pEngineCore = EngineCore::getInstance();

	// init engine
	setLoadingProgress(25, "Initializing sfx/gfx..");
	const bool initAudioVisualsSucceeded	= m_pEngineCore->initAudioVisuals(m_configs);

	setLoadingProgress(45, "Initializing game..");
	const bool initLogicSucceeded			= m_pEngineCore->initLogic();

	setLoadingProgress(65, "Finishing steps..");

	if (!initLogicSucceeded || !initAudioVisualsSucceeded)
	{
		TRACE_ERROR("Error: Cannot initialize world.", 0);
		release();
	}

	setLoadingProgress(95, "Initializing player..");

	// init player

	m_pEngineCore->getRenderContext()->setContextFloatParam("bloomLimit", CONST_FLOAT("Effects::Shaders::Bloom::Limit"));
	m_pEngineCore->getRenderContext()->setContextFloatParam("dofFadeDist", CONST_FLOAT("Effects::Shaders::DOF::FadeDist"));
	m_pEngineCore->getRenderContext()->setContextFloatParam("dofSaturation", CONST_FLOAT("Effects::Shaders::DOF::Saturation"));

	setLoadingProgress(100, "Done.");

	return true;
}

// render
void Client::render()
{
	if (m_clientId == k_clientIdNone)
	{
		renderGui();

		glutSwapBuffers();
		return;
	}

	if (m_gamePaused)
	{
		graphics::Shader::disableAll();
		m_pGameConsole->render();

		glutSwapBuffers();
		return;
	}

	if (m_dt == 0.0f)
	{
		m_dt = 0.01;
	}


	//physics::PhysicsManager::getInstance()->getPhysicsResults();

	m_debug_physxSimulationActive = false;

	//physics::PhysicsManager::getInstance()->releaseDeadActors();
	//CharacterController::releaseDeadControllers();


	//m_pEngineCore->getPlayer()->applyInput(m_dt);

	m_pEngineCore->animate(m_dt);
	m_pEngineCore->render(m_dt);

	graphics::Shader::disableAll();
	m_pGameConsole->render();

	//if (m_isGuiOpened)
	{
		renderGui();
	}

	//physics::PhysicsManager::getInstance()->startPhysics(m_dt);

	m_debug_physxSimulationActive = true;

	glutSwapBuffers();
}

void Client::idleFunc()
{
	listen();

	m_dt = (glutGet(GLUT_ELAPSED_TIME) - m_lastRenderTime) / 200.0f;
	m_lastRenderTime = glutGet(GLUT_ELAPSED_TIME);

	glutPostRedisplay();

	if (!m_gamePaused && m_processInput)
	{
		glutWarpPointer(m_configs.width / 2, m_configs.height / 2);
	}
}

GameConsole* Client::getGameConsole()
{
	return m_pGameConsole;
}

} // namespace network
