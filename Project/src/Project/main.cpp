#include "GameStdAfx.h"
#ifdef CLIENT_SIDE
#include "Client/ClientMain.h"
#endif

#ifdef SERVER_SIDE
#include "Server/Server.h"
#include "Network/connection.h"
#include "Network/events/LuaCommand.h"
#endif


//#define ENABLE_VMEM
//#define ENABLE_MEMPRO

#if defined(ENABLE_VMEM)
	#ifdef ENABLE_MEMPRO
		#define MEMPRO
	#endif

	#define OVERRIDE_NEW_DELETE
	#include "Common/VMem.hpp"
#elif defined(ENABLE_MEMPRO)
	#define OVERRIDE_NEW_DELETE
	#define WAIT_FOR_CONNECT true
	#include "Common/MemPro.hpp"
#endif

#ifdef CLIENT_SIDE
ClientConfigs conf;
network::Client* client = nullptr;
#endif
#ifdef SERVER_SIDE
network::Server* server = nullptr;
#endif

// shutting down
void shutDown()
{
#ifdef CLIENT_SIDE
	SAFEDEL(client);
#endif

#ifdef SERVER_SIDE
	SAFEDEL(server);
#endif
}

/**
 * Prints the given std::string to the console. Can be called from lua scripts.
 *
 * logToConsole() is overloaded with different definitions on the server and client side (the server side can broadcasts the std::string).
 * @param str The std::string to be printed.
 */
void logToConsole(std::string str)
{
	bool handled = false;

#ifdef CLIENT_SIDE
	if (client)
	{
		client->getGameConsole()->print(str);
		handled = true;
	}
#endif // CLIENT_SIDE

#ifdef SERVER_SIDE
	if (!handled && server && server->isRunning())
	{
		TRACE_LUA("> " << str, 0);
		const std::string serialStr = network::marshal(network::events::LuaCommand(str));
		ENetPacket* packet = enet_packet_create((enet_uint8*) serialStr.c_str(), serialStr.length(), ENET_PACKET_FLAG_RELIABLE);
		enet_host_broadcast(server->getENetHost(), 0, packet);
		handled = true;
	}
#endif // SERVER_SIDE

	if(!handled)
	{
		TRACE_LUA("> " << str, 0);
	}
}

#define CLIENT_START_CODE "c"
#define CLIENT_THICK_START_CODE "b"
#define SERVER_START_CODE "s"

int serverMain(const int argc, char* argv[])
{
#ifdef SERVER_SIDE
	if (argc < 1)
	{
		return EXIT_FAILURE;
	}

	atexit(shutDown);

	ushort port = 0;
	short broadCastRate = 0;

	if (argc == 2)
	{
		const char* argumentsStr = utils::file::readFile(CONST_STR("resourcesDir") + "/serverArgs.txt");
		const std::vector<std::string> arguments = utils::tokenize(argumentsStr, " ");

		port			= atoi(arguments[0].c_str());
		broadCastRate	= atoi(arguments[1].c_str());

		delete argumentsStr;
	}
	else
	{
		port = atoi(argv[2]);
		broadCastRate = atoi(argv[3]);
	}

	server = new network::Server(port, broadCastRate);
	server->start();

	SAFEDEL(server);
#endif
	return EXIT_SUCCESS;
}

int clientMain(int argc, char* argv[], const bool isThickClient)
{
#ifdef CLIENT_SIDE
	//freopen("lualog.txt", "w", stdout);
	atexit(shutDown);						// called only if the code is EXIT_SUCCESS!

	// read command line arguments
	bool		instantStart = false;	// without starting gui
	std::string	settingsFile;
	std::string	hostAddress;
	int			hostPort = 0;

	int			numDbgBreaks = 0;
	int*		debugBreakArray = new int[20];

	// read the arguments from clientArgs.txt
	if (argc == 2)
	{
		new ConstantManager();
		const char* argumentsStr = utils::file::readFile(CONST_STR("resourcesDir") + "/clientArgs.txt");
		const std::vector<std::string> arguments = utils::tokenize(argumentsStr, " ");

		for (uint i = 0; i < arguments.size(); i++)
		{
			if (arguments[i] == "-s" || arguments[i] == "-settings")
			{
				settingsFile = arguments[i + 1];
			}

			if (arguments[i] == "-h" || arguments[i] == "-host")
			{
				hostAddress = arguments[i + 1];
			}

			if (arguments[i] == "-p" || arguments[i] == "-port")
			{
				hostPort = atoi(arguments[i + 1].c_str());
			}

			if (arguments[i] == "-db" || arguments[i] == "-dbgBrk")
			{
				debugBreakArray[numDbgBreaks++] = atoi(arguments[i + 1].c_str());
			}

			if (arguments[i] == "-instant")
			{
				instantStart = true;
			}
		}

		delete argumentsStr;
	}
	else
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-settings") == 0)
			{
				settingsFile = argv[i + 1];
			}

			if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-host") == 0)
			{
				hostAddress = argv[i + 1];
			}

			if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-port") == 0)
			{
				hostPort = atoi( argv[i + 1] );
			}

			if (strcmp(argv[i], "-db") == 0 || strcmp(argv[i], "-dbgBrk") == 0)
			{
				debugBreakArray[numDbgBreaks++] = atoi(argv[i + 1]);
			}

			if (strcmp(argv[i], "-instant") == 0)
			{
				instantStart = true;
			}
		}
	}

	// for debugging
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	for (int i = 0; i < numDbgBreaks; i++)
	{
		_CrtSetBreakAlloc( debugBreakArray[i] );
	}
#endif
	delete[] debugBreakArray;


	if (!settingsFile.empty())
	{
		conf.loadConfigs(settingsFile);
	}
	else
	{
		conf.loadConfigs();
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	//glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - conf.width) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - conf.height) / 2);
	glutInitWindowPosition(0, glutGet(GLUT_SCREEN_HEIGHT) - conf.height - 30);
	glutInitWindowSize(conf.width0, conf.height0);
	glutCreateWindow("Survive");

	client = new network::Client(isThickClient);
	if (!client->init(conf, hostAddress, hostPort, instantStart))
	{
		exit(EXIT_SUCCESS);
	}

	client->initGraphics();


	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutIdleFunc(idleFunc);

	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	glutMouseFunc(mouseAction);
	glutMotionFunc(mouseDrag);
	glutPassiveMotionFunc(mouseMove);

	//glutEntryFunc(entryFunc);

	if (conf.m_fullscreen)
	{
		glutFullScreen();
	}


	glutMainLoop();
#endif

	return 0;
}

#include "GameLogic/Drone.h"

int main(int argc, char* argv[])
{
	entityx::EntityX ex;

	Drone drone(ex.entities.create());
	drone.move(vec2(1, 0));
	const std::string serialStr = network::marshal(drone, 0);

#if defined(CLIENT_SIDE) && defined(SERVER_SIDE)
	if (strcmp(argv[1], CLIENT_START_CODE) == 0)
	{
		return clientMain(argc, argv);
	}
	else if (strcmp(argv[1], CLIENT_THICK_START_CODE) == 0)
	{
		return clientMain(argc, argv, true);
	}
	else if (strcmp(argv[1], SERVER_START_CODE) == 0)
	{
		return serverMain(argc, argv);
	}
	std::cout << "Bad argument. Possible ones are: " << CLIENT_START_CODE << " " << CLIENT_THICK_START_CODE << " " << SERVER_START_CODE;
#elif defined(CLIENT_SIDE)
	return clientMain(argc, argv);
#elif defined(SERVER_SIDE)
	return serverMain(argc, argv);
#endif
}



#ifdef CLIENT_SIDE
// binding glut functions to client methods

// glutReshapeFunc
void reshape(int width, int height)
{
	client->reshape(width, height);
}

// glutDisplayFunc
void render()
{
	client->render();
}

// inputs
void keyDown(uint8_t key, int x, int y)
{
	client->keyDown(key, x, y);
}

void keyUp(uint8_t key, int x, int y)
{
	client->keyUp(key, x, y);
}

void special(int key, int x, int y)
{
	client->specialDown(key, x, y);
}

void specialUp(int key, int x, int y)
{
	client->specialUp(key, x, y);
}

void mouseMove(int x, int y)
{
	client->mouseMove(x, y);
}

void mouseAction(int button, int state, int x, int y)
{
	client->mouseAction(button, state, x, y);
}

void mouseDrag(int x, int y)
{
	client->mouseDrag(x, y);
}

void idleFunc()
{
	client->idleFunc();
}

void entryFunc(int state)
{
	client->entryFunc(state);
}
#endif // CLIENT_SIDE