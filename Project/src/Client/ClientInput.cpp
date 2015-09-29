#include "GameStdAfx.h"
#include "Client/Client.h"

// OpenGL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "Common/enginecore/EngineCore.h"

#include "Graphics/Camera.h"
#include "Graphics/RenderContext.h"

#include "Console/CrimsonConsole.h"

#ifdef ENABLE_MYGUI
#include <MyGUI/MyGUI.h>
#include <MyGUI/MyGUI_OpenGLPlatform.h>

#include <MyGUI/Common/Input/InputConverter.h>
#endif



namespace network
{

bool isWireframeRenderEnabled = false;

void Client::registerActionKeys()
{
	// read from xml
	using namespace events;

	m_registeredActionKeys[CONST_INT("ControlsKeys::Use")]					= InputEvent::INPUTEVENT_USE;
	m_registeredActionKeys[CONST_INT("ControlsKeys::MoveForward")]			= InputEvent::INPUTEVENT_MOVE_FORWARD;
	m_registeredActionKeys[CONST_INT("ControlsKeys::MoveBackward")]			= InputEvent::INPUTEVENT_MOVE_BACKWARD;
	m_registeredActionKeys[CONST_INT("ControlsKeys::MoveRight")]			= InputEvent::INPUTEVENT_MOVE_RIGHT;
	m_registeredActionKeys[CONST_INT("ControlsKeys::MoveLeft")]				= InputEvent::INPUTEVENT_MOVE_LEFT;

	m_registeredActionKeys[CONST_INT("ControlsKeys::Rush")]					= InputEvent::INPUTEVENT_RUSH;
	m_registeredActionKeys[CONST_INT("ControlsKeys::Jump")]					= InputEvent::INPUTEVENT_JUMP;
	m_registeredActionKeys[CONST_INT("ControlsKeys::ToggleCrouch")]			= InputEvent::INPUTEVENT_TOGGLE_CROUCH;

	//m_registeredActionKeys[InputEvent::INPUTEVENT_USE]						= CONST_INT("ControlsKeys::Use");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_MOVE_FORWARD]				= CONST_INT("ControlsKeys::MoveForward");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_MOVE_BACKWARD]			= CONST_INT("ControlsKeys::MoveBackward");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_MOVE_RIGHT]				= CONST_INT("ControlsKeys::MoveRight");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_MOVE_LEFT]				= CONST_INT("ControlsKeys::MoveLeft");

	//m_registeredActionKeys[InputEvent::INPUTEVENT_RUSH]						= CONST_INT("ControlsKeys::Rush");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_JUMP]						= CONST_INT("ControlsKeys::Jump");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_TOGGLE_CROUCH]			= CONST_INT("ControlsKeys::ToggleCrouch");

	//m_registeredActionKeys[InputEvent::INPUTEVENT_ATTACK_PRIMARY]			= CONST_INT("ControlsKeys::AttackPrimary");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_ATTACK_SECONDARY]			= CONST_INT("ControlsKeys::AttackSecondary");

	//m_registeredActionKeys[InputEvent::INPUTEVENT_RELOAD]					= CONST_INT("ControlsKeys::Reload");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_THROW]					= CONST_INT("ControlsKeys::Throw");

	//m_registeredActionKeys[InputEvent::INPUTEVENT_FLASHLIGHT]				= CONST_INT("ControlsKeys::Flashlight");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_USE_ITEM_SLOT0]			= CONST_INT("ControlsKeys::UseItemSlot0");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_USE_ITEM_SLOT1]			= CONST_INT("ControlsKeys::UseItemSlot1");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_USE_ITEM_SLOT2]			= CONST_INT("ControlsKeys::UseItemSlot2");

	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_LAST]		= CONST_INT("ControlsKeys::ChangeWeaponLast");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_PREV]		= CONST_INT("ControlsKeys::ChangeWeaponPrev");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_NEXT]		= CONST_INT("ControlsKeys::ChangeWeaponNext");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_PRIMARY]	= CONST_INT("ControlsKeys::ChangeWeaponPrimary");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_SECONDARY]	= CONST_INT("ControlsKeys::ChangeWeaponSecondary");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_MELEE]		= CONST_INT("ControlsKeys::ChangeWeaponMelee");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_ITEM_SLOT0]	= CONST_INT("ControlsKeys::ChangeWeaponSlot0");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_ITEM_SLOT1]	= CONST_INT("ControlsKeys::ChangeWeaponSlot1");
	//m_registeredActionKeys[InputEvent::INPUTEVENT_CHANGE_WEAPON_ITEM_SLOT2]	= CONST_INT("ControlsKeys::ChangeWeaponSlot2");
}

#ifdef ENABLE_MYGUI
MyGUI::KeyCode::Enum keyToMyGuiKeyCode(const uint8_t key)
{
	MyGUI::KeyCode::Enum keyCode;

	switch (key)
	{
		case 13:
			keyCode = MyGUI::KeyCode::Return;
			break;
		case 127:
			keyCode = MyGUI::KeyCode::Delete;
			break;
		case 8:
			keyCode = MyGUI::KeyCode::Backspace;
			break;
		default:
			if (std::isdigit(key) && key != '0')
			{
				keyCode = MyGUI::KeyCode::Enum( MyGUI::KeyCode::Escape + (key - '0') );
			}
			else
			{
				keyCode = MyGUI::KeyCode::Enum(key);
			}
	};

	return keyCode;
}
#endif

void Client::keyDown(uint8_t key, int x, int y)
{
	m_keyEvent.set(network::events::KeyEvent(network::events::KeyEvent::NETOBJ_KEY_DOWN, key));

#ifndef SERVER_SIDE
	m_pEngineCore->getPlayer()->setKeyState(key, true);
#endif

	// enter on console
	if (m_pGameConsole->isOpen() && key == 13)
	{
		std::string command(m_pGameConsole->getCommand());
		sendPacket(network::events::LuaCommand(command));

		// special commands
		if (command == "quit")
		{
			flushPackets();
			exit(EXIT_SUCCESS);
		}
		else if (command == "state")
		{
			TRACE_INFO("------------------------------------------------------------------", 0);
			///
			//for (const auto& entry : EngineCore::getInstance()->getNodeIdDirectory())
			//{
			//	TRACE_INFO(entry.first << "\t(" << entry.second->getId() << ")\t\t" << entry.second->getName(), 0);
			//}

			flushPackets();
		}
		else if (command.find("speed") != std::string::npos)
		{
			float speedMultiplier;
			sscanf(command.c_str(), "speed %f", &speedMultiplier);
			ConstantManager::getInstance()->setFloatConstant("Gameplay::GameSpeedMultiplier", speedMultiplier);

			flushPackets();
		}

		m_pGameConsole->keyDown(key, x, y);
	}
	else if (key == 27)
	{
		m_processInput = !m_processInput;
		toggleMainMenuGui();
	}
	else if (!m_isGuiOpened)
	{
		if (m_pGameConsole->keyDown(key, x, y) && !m_gamePaused)
		{
			sendPacket(m_keyEvent);
		}

		// render wireframes on/off
		if (!m_pGameConsole->isOpen() && (key == 'p' || key == 'P'))
		{
			isWireframeRenderEnabled = !isWireframeRenderEnabled;
			m_pEngineCore->getRenderContext()->setEnableBit("wireframe", isWireframeRenderEnabled);
		}

		// render shadow on/off
		if ((key == 'k' || key == 'K'))
		{
			bool shadowEnabled = m_pEngineCore->getRenderContext()->getEnableBit("shadow");
			m_pEngineCore->getRenderContext()->setEnableBit("shadow", !shadowEnabled);
		}

		// bloom+
		if ((key == 'v' || key == 'V') && m_pEngineCore->getRenderContext()->getContextFloatParam("bloomLimit") < 1.0f)
		{
			float bloomLimit = m_pEngineCore->getRenderContext()->getContextFloatParam("bloomLimit");
			m_pEngineCore->getRenderContext()->setContextFloatParam("bloomLimit", bloomLimit + 0.1f);
		}

		// bloom-
		if ((key == 'b' || key == 'B') && m_pEngineCore->getRenderContext()->getContextFloatParam("bloomLimit") > 0.0f)
		{
			float bloomLimit = m_pEngineCore->getRenderContext()->getContextFloatParam("bloomLimit");
			m_pEngineCore->getRenderContext()->setContextFloatParam("bloomLimit", bloomLimit - 0.1f);
		}

		// dof+
		if (key == 'm' || key == 'M')
		{
			float dofFadeDist = m_pEngineCore->getRenderContext()->getContextFloatParam("dofFadeDist");
			m_pEngineCore->getRenderContext()->setContextFloatParam("dofFadeDist", dofFadeDist + 10.0f);
		}

		// dof-
		if (key == 'n' || key == 'N')
		{
			float dofFadeDist = m_pEngineCore->getRenderContext()->getContextFloatParam("dofFadeDist");
			m_pEngineCore->getRenderContext()->setContextFloatParam("dofFadeDist", dofFadeDist - 10.0f);

			if (dofFadeDist - 10.0f < 0.0f)
			{
				m_pEngineCore->getRenderContext()->setContextFloatParam("dofFadeDist", 0.0f);
			}
		}
	}
#ifdef ENABLE_MYGUI
	else // Gui
	{
		//MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(key), (MyGUI::Char)key);
		MyGUI::InputManager::getInstance().injectKeyPress(keyToMyGuiKeyCode(key), (MyGUI::Char) key);
	}
#endif
}

void Client::keyUp(uint8_t key, int x, int y)
{
	m_keyEvent.set(network::events::KeyEvent(network::events::KeyEvent::NETOBJ_KEY_UP, key));


#ifndef SERVER_SIDE
	m_pEngineCore->getPlayer()->setKeyState(key, false);
#endif

	if (m_isGuiOpened)
	{
#ifdef ENABLE_MYGUI
		MyGUI::InputManager& guiInputManager = MyGUI::InputManager::getInstance();

		if (guiInputManager.isShiftPressed())
		{
			guiInputManager.injectKeyRelease(MyGUI::KeyCode::RightShift);
		}

		guiInputManager.injectKeyRelease(keyToMyGuiKeyCode(key));
#endif
	}
	else
	{
		if (m_pGameConsole->keyUp(key, x, y) && !m_gamePaused)
		{
			sendPacket(m_keyEvent);
		}
	}
}

void Client::specialDown(int key, int x, int y)
{
	m_pGameConsole->special(key, x, y);

	if (m_isGuiOpened)
	{
		handleSpecialKeyDown(key);
	}
}

void Client::specialUp(int key, int x, int y)
{
	m_keyEvent.set(network::events::KeyEvent(network::events::KeyEvent::NETOBJ_KEY_UP, key));

	//if (getApplicationSide() == APPSIDE_BOTH)
	//{
	//	m_pEngineCore->getPlayer()->setKeyState(key, false);
	//}

	switch (key)
	{
		case GLUT_KEY_F1:
			m_pGameConsole->specialUp(key, x, y, m_gamePaused);
			break;

		case GLUT_KEY_F2:
			if (!m_gamePaused)
			{
				sendPacket(m_keyEvent);
			}

			m_pEngineCore->reloadLuaScripts();
			break;

		case GLUT_KEY_F3:
			if (!m_gamePaused)
			{
				sendPacket(m_keyEvent);
			}

			m_pEngineCore->resetLuaScripts();
			break;

		case GLUT_KEY_F11:
			m_processInput = !m_processInput;
			toggleMainMenuGui();
			break;

		case GLUT_KEY_F12:
			m_gamePaused = !m_gamePaused;
			break;
	}

	if (m_isGuiOpened)
	{
		handleSpecialKeyUp(key);
	}
}

void Client::mouseMove(int x, int y)
{
	m_mouseEvent = network::events::MouseEvent(network::events::MouseEvent::NETOBJ_MOUSE_MOVE, x, y, true, m_configs.m_mouseSensitivity, m_configs.width, m_configs.height);

	if (m_gamePaused)
	{
		m_pGameConsole->mouseMove(x, y);
	}

#ifndef SERVER_SIDE
	if (!m_gamePaused && m_processInput)
	{
		network::send(m_mouseEvent, m_pPeer);
	}
#endif

	if (m_isGuiOpened)
	{
#ifdef ENABLE_MYGUI
		MyGUI::InputManager::getInstance().injectMouseMove(x, y, 0);
#endif
	}
}

void Client::mouseAction(int button, int state, int x, int y)
{
	if (m_gamePaused || !m_processInput)
	{
		m_pGameConsole->mouseAction(button, state, x, y);
	}

	int eventButton = 0;

	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			eventButton = network::events::MouseEvent::BUTTON1_DOWN;
		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			eventButton = network::events::MouseEvent::BUTTON2_DOWN;
		}

		if (m_isGuiOpened)
		{
#ifdef ENABLE_MYGUI
			MyGUI::InputManager::getInstance().injectMousePress(x, y, (button == GLUT_LEFT_BUTTON) ? MyGUI::MouseButton::Left : MyGUI::MouseButton::Right);
#endif
		}
	}

	if (state == GLUT_UP)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			eventButton = network::events::MouseEvent::BUTTON1_UP;
		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			eventButton = network::events::MouseEvent::BUTTON2_UP;
		}

		if (m_isGuiOpened)
		{
#ifdef ENABLE_MYGUI
			MyGUI::InputManager::getInstance().injectMouseRelease(x, y, (button == GLUT_LEFT_BUTTON) ? MyGUI::MouseButton::Left : MyGUI::MouseButton::Right);
#endif
		}
	}

	m_mouseEvent = network::events::MouseEvent(network::events::MouseEvent::NETOBJ_MOUSE_ACTION, x, y, true, m_configs.m_mouseSensitivity, m_configs.width, m_configs.height, eventButton);

#ifndef SERVER_SIDE
	if (!m_mouseEvent.m_isUpdated)
	{
		TRACE_INFO("melted", 0);
	}

	if (!m_gamePaused && m_processInput)
	{
		network::send(m_mouseEvent, m_pPeer);
	}
#endif
}

void Client::mouseDrag(int x, int y)
{
	m_mouseEvent = network::events::MouseEvent(network::events::MouseEvent::NETOBJ_MOUSE_DRAG, x, y, true, m_configs.m_mouseSensitivity, m_configs.width, m_configs.height);

	if (m_isGuiOpened)
	{
#ifdef ENABLE_MYGUI
		MyGUI::InputManager::getInstance().injectMouseMove(x, y, 0);
#endif
	}

	if (m_gamePaused || !m_processInput)
	{
		return;
	}

#ifndef SERVER_SIDE
	network::send(m_mouseEvent, m_pPeer);
#endif

	//sendPacket(m_mouseEvent);
}

void Client::reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (GLfloat) width / height, 1.0f, 300.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	glutSetCursor(GLUT_CURSOR_NONE);

#ifdef ENABLE_MYGUI
	updateGuiViewSize(width, height);
#endif
}

void Client::entryFunc(int state)
{
	m_processInput = state;
	//if (state == GLUT_LEFT)
	//	m_processInput = false;
}

} // namespace network
