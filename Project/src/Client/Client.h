#pragma once

#define NOMINMAX

//#define ENABLE_MYGUI

#include <enet/enet.h>

#include "Network/CrimsonNetwork.h"
#include "GameLogic/EngineCore.h"

#ifdef ENABLE_MYGUI
#include <MyGUI/MyGUI.h>

namespace MyGUI
{
	class Gui;
	class OpenGLPlatform;

	class Widget;
	class Button;
	class EditBox;
	class Window;
}
#endif

class GameConsole;

class OpenGLImageLoader_Devil;


namespace network
{

class Client
{
public:
	static const short k_clientIdNone;
	static const short k_clientIdLocalFull;

	Client(const bool isThickClient = false);
	~Client();


	// init
	bool init(const ClientConfigs& configs, const std::string& hostAddress = "localhost", uint port = 80, bool instantStart = false);
	bool initConsole();
	void initGraphics();
	bool initEngineCore();
	void initLocalPlayer();

	void release();


	// render
	void render();
	void idleFunc();
	void reshape(int iwidth, int iheight);


	// input
	void registerActionKeys();
	void keyDown(uint8_t key, int x, int y);
	void keyUp(uint8_t key, int x, int y);
	void specialDown(int key, int x, int y);
	void specialUp(int key, int x, int y);

	void mouseMove(int x, int y);
	void mouseDrag(int x, int y);
	void mouseAction(int button, int state, int x, int y);

	void entryFunc(int state);


	// networking part
	bool connect(const std::string& host, uint port);
	void disconnect();

	void listen();

	template <class T>
	void sendPacket(T& packet)
	{
		if (m_clientId != k_clientIdNone && packet.m_isUpdated)
		{
			network::send(packet, m_pPeer);
		}
	}

	void flushPackets();


	// getters-setters
	ENetPeer*		getPeer() const;
	short			getClientId() const;
	GameConsole*	getGameConsole();

private:
#ifndef ENABLE_MYGUI
	// gui methods
	bool initGui(bool instantStart = true) { return true; }

	void initGameGui() {}
	void initPreGameGui() {}

	void releaseGui() {}

	void renderGui() {}

	void printToChatHistory(const std::string& message) {}

	void toggleMainMenuGui() {}

	void updateGuiViewSize(int width, int height) {}
	void setLoadingProgress(int progress, std::string message) {}

	void handleSpecialKeyDown(int key) {}
	void handleSpecialKeyUp(int key) {}
#else
	bool initGui(bool instantStart = true);
	void initGameGui();
	void initPreGameGui();

	void releaseGui();

	void renderGui();

	void printToChatHistory(const std::string& message);

	void toggleMainMenuGui();

	void updateGuiViewSize(int width, int height);
	void setLoadingProgress(int progress, std::string message);

	void handleSpecialKeyDown(int key);
	void handleSpecialKeyUp(int key);

	// gui event handlers
	void initEventHandlers(MyGUI::Widget* pRootWidget);

	void frameEvent_handler(float time);

	void button_handler(MyGUI::Widget* sender);
	void editBox_handler(MyGUI::Widget* sender, MyGUI::KeyCode _key, MyGUI::Char _char);
	void comboBox_handler(MyGUI::ComboBox* sender, size_t index);
	void scrollChangePosition_handler(MyGUI::ScrollBar* sender, size_t position);
	void windowClose_handler(MyGUI::Window* sender, const std::string& name);
#endif

private:
	// network attributes
	short								m_clientId;

	std::string							m_clientName;
	std::string							m_hostAddress;
	ushort								m_hostPort;

	NetworkObject						m_netObject;

	GameState							m_package;
	ClientTable							m_clientTable;

	// enet attributes
	ENetHost*							m_pClientHost;
	ENetPeer*							m_pPeer;
	ENetAddress							m_address;
	ENetEvent							m_event;
	int									m_serviceResult;

	events::KeyEvent					m_keyEvent;
	events::MouseEvent					m_mouseEvent;
	std::map<short, short>				m_registeredActionKeys;

	events::Killshot					m_killshot;
	events::LuaCommand					m_luaResponse;
	events::ChatMessage					m_chatMessage;
	events::PlayerDisconnectingEvent	m_disconnectingEvent;


	// game attributes
	EngineCore*							m_pEngineCore;
	float								m_dt;
	long								m_lastRenderTime;
	bool								m_gamePaused;
	bool								m_processInput;

	GameConsole*						m_pGameConsole;

	ClientConfigs						m_configs;

#ifdef ENABLE_MYGUI
	// gui attributes
	MyGUI::Gui*							m_pGui;
	MyGUI::OpenGLPlatform*				m_pGuiPlatform;
	OpenGLImageLoader_Devil*			m_pGuiImageLoader;

	MyGUI::VectorWidgetPtr				m_guiRoot;
	MyGUI::MapWidgetPtr					m_guiGameWidgets;
#endif

	bool								m_isGuiOpened;

	// debug
	bool								m_debug_physxSimulationActive;
};

} // namespace network

void shutDown();
