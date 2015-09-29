#include "GameStdAfx.h"

#ifdef ENABLE_MYGUI
#include "Client/Client.h"

#include "Graphics/RenderContext.h"

#include "Console/CrimsonConsole.h"

#include <MyGUI/MyGUI.h>
#include <MyGUI/MyGUI_OpenGLPlatform.h>
#include "OpenGLImageLoader_Devil.h"


namespace network
{

bool Client::initGui(bool instantStart)
{
	m_pGuiImageLoader = new OpenGLImageLoader_Devil();

	m_pGuiPlatform = new MyGUI::OpenGLPlatform();
	m_pGuiPlatform->initialise(m_pGuiImageLoader);

	MyGUI::LogManager::getInstance().setLoggingLevel(MyGUI::LogLevel::Error);


	// set mygui resource paths
	MyGUI::xml::Document doc;

	if (!doc.open(std::string(CONST_STR("dataDir") + "/MyGuiMedia/resources.xml")))
	{
		doc.getLastError();
		GX_ASSERT(0 && "Error: resources cannot be loaded from resources.xml\n %s", doc.getLastError());
	}

	MyGUI::xml::ElementPtr docRoot = doc.getRoot();
	if (docRoot == nullptr || docRoot->getName() != "Paths")
	{
		return false;
	}

	MyGUI::xml::ElementEnumerator node = docRoot->getElementEnumerator();
	while (node.next())
	{
		if (node->getName() == "Path")
		{
			m_pGuiPlatform->getDataManagerPtr()->addResourceLocation(node->getContent(), false);
		}
	}

	m_pGuiPlatform->getRenderManagerPtr()->setViewSize(m_configs.width0, m_configs.height0);

	// initialize pre-game gui
	m_pGui = new MyGUI::Gui();
	m_pGui->initialise();
	MyGUI::ResourceManager::getInstance().load("MyGUI_BlackBlueTheme.xml");

	if (!instantStart)
	{
		initPreGameGui();
	}

	return true;
}

void Client::initPreGameGui()
{
	// load starting layouts
	m_guiRoot = MyGUI::LayoutManager::getInstance().loadLayout( CONST_STR("GUI::ClientPreGameLayout") );

	MyGUI::WidgetPtr currentWidgetRoot = m_guiRoot.at(0);

	currentWidgetRoot->findWidget("addressEditBox")->castType<MyGUI::EditBox>()->setCaption(m_hostAddress);
	std::stringstream ss;
	ss << m_hostPort;
	currentWidgetRoot->findWidget("portEditBox")->castType<MyGUI::EditBox>()->setCaption(ss.str());

	currentWidgetRoot->findWidget("exitButton")->eventMouseButtonClick += MyGUI::newDelegate(this, &Client::button_handler);
	currentWidgetRoot->findWidget("connectButton")->eventMouseButtonClick += MyGUI::newDelegate(this, &Client::button_handler);

	m_guiRoot.at(1)->setVisible(false);
	m_guiRoot.at(1)->findWidget("progressBar")->castType<MyGUI::ProgressBar>()->setProgressPosition(1);

	m_isGuiOpened = true;
}

void Client::initEventHandlers(MyGUI::Widget* pRootWidget)
{
	MyGUI::EnumeratorWidgetPtr enumerator = pRootWidget->getEnumerator();

	while (enumerator.next())
	{
		// buttons
		MyGUI::Widget* pCurrentElement = enumerator.current();
		const std::string widgetName = enumerator.current()->getName();

		if (widgetName.find("Button") != std::string::npos)
		{
			pCurrentElement->eventMouseButtonClick += MyGUI::newDelegate(this, &Client::button_handler);
		}

		// editboxes
		else if (widgetName.find("EditBox") != std::string::npos)
		{
			pCurrentElement->castType<MyGUI::EditBox>()->eventKeyButtonPressed += MyGUI::newDelegate(this, &Client::editBox_handler);
		}

		// sliders
		else if (widgetName.find("Slider") != std::string::npos)
		{
			pCurrentElement->castType<MyGUI::ScrollBar>()->eventScrollChangePosition += MyGUI::newDelegate(this, &Client::scrollChangePosition_handler);
		}

		// comboboxes
		else if (widgetName.find("ComboBox") != std::string::npos)
		{
			pCurrentElement->castType<MyGUI::ComboBox>()->eventComboChangePosition += MyGUI::newDelegate(this, &Client::comboBox_handler);
		}

		// checkboxes
		else if (widgetName.find("CheckBox") != std::string::npos)
		{
			pCurrentElement->eventMouseButtonClick += MyGUI::newDelegate(this, &Client::button_handler);
		}

		else if (pCurrentElement->getChildCount() > 0)
		{
			initEventHandlers(pCurrentElement);
		}
	}
}

void Client::initGameGui()
{
	m_isGuiOpened = false;

	// release old layouts
	if (!m_guiRoot.empty())
	{
		MyGUI::LayoutManager::getInstance().unloadLayout(m_guiRoot);
	}

	// load in-game layout
	m_guiRoot = MyGUI::LayoutManager::getInstance().loadLayout( CONST_STR("GUI::ClientGameplayLayout") );

	m_guiGameWidgets["mainmenuWindow"]	= m_guiRoot.at(0);
	m_guiGameWidgets["chatPanel"]		= m_guiRoot.at(1);
	m_guiGameWidgets["optionsWindow"]	= m_guiRoot.at(2);

	ForEach(MyGUI::MapWidgetPtr, m_guiGameWidgets, it)
	{
		initEventHandlers(it->second);
	}

	m_guiGameWidgets["optionsWindow"]->findWidget("shadowsCheckBox")->castType<MyGUI::Button>()->setStateSelected(m_configs.m_shadowsOn);
	m_guiGameWidgets["optionsWindow"]->castType<MyGUI::Window>()->eventWindowButtonPressed += MyGUI::newDelegate(this, &Client::windowClose_handler);

	m_pGuiPlatform->getRenderManagerPtr()->setViewSize(m_configs.width, m_configs.height);
}

void Client::releaseGui()
{
	if (m_pGui)
	{
		m_pGui->shutdown();
		SAFEDEL(m_pGui);
	}

	SAFEDEL(m_pGuiImageLoader);

	if (m_pGuiPlatform)
	{
		m_pGuiPlatform->shutdown();
		SAFEDEL(m_pGuiPlatform);
	}
}

void Client::toggleMainMenuGui()
{
	m_isGuiOpened = !m_isGuiOpened;

	//m_guiGameWidgets["mainmenuWindow"]->setVisible(m_isGuiOpened);
	//m_guiGameWidgets["chatPanel"]->setVisible(m_isGuiOpened);
}

void Client::renderGui()
{
	if (!m_isGuiOpened)
	{
		return;
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);


	glLoadIdentity();

	if (m_pGuiPlatform)
	{
		m_pGuiPlatform->getRenderManagerPtr()->drawOneFrame();
	}
}

void Client::printToChatHistory(const std::string& message)
{
	MyGUI::EditBox* pChatHistory = m_guiGameWidgets["chatPanel"]->findWidget("chatHistory")->castType<MyGUI::EditBox>();

	pChatHistory->addText( message + "\n" );
}


std::pair<int, MyGUI::KeyCode::Enum> map_data[] =
{
	std::make_pair(GLUT_KEY_UP,			MyGUI::KeyCode::ArrowUp),
	std::make_pair(GLUT_KEY_DOWN,		MyGUI::KeyCode::ArrowDown),
	std::make_pair(GLUT_KEY_LEFT,		MyGUI::KeyCode::ArrowLeft),
	std::make_pair(GLUT_KEY_RIGHT,		MyGUI::KeyCode::ArrowRight),
	std::make_pair(GLUT_KEY_HOME,		MyGUI::KeyCode::Home),
	std::make_pair(GLUT_KEY_END,		MyGUI::KeyCode::End),
	std::make_pair(GLUT_KEY_PAGE_UP,	MyGUI::KeyCode::PageUp),
	std::make_pair(GLUT_KEY_PAGE_DOWN,	MyGUI::KeyCode::PageDown),
	std::make_pair(GLUT_KEY_INSERT,		MyGUI::KeyCode::Insert)
};

std::map<int, MyGUI::KeyCode::Enum> glutMyguiKeyMap(map_data, map_data + sizeof(map_data) / sizeof(map_data[0]));

void Client::button_handler(MyGUI::Widget* sender)
{
	const std::string& senderName = sender->getName();

	// connect button
	if (senderName == "connectButton")
	{
		MyGUI::WidgetPtr currentWidgetRoot = m_guiRoot.at(0);

		m_hostAddress	= currentWidgetRoot->findWidget("addressEditBox")->castType<MyGUI::EditBox>()->getCaption();
		m_hostPort		= atoi(currentWidgetRoot->findWidget("portEditBox")->castType<MyGUI::EditBox>()->getCaption().asUTF8_c_str());
		m_clientName	= currentWidgetRoot->findWidget("playernameEditBox")->castType<MyGUI::EditBox>()->getCaption();

		if (!initEngineCore())
		{
			currentWidgetRoot->findWidget("errorMsgTextBox")->castType<MyGUI::TextBox>()->setCaption("Error: could not connect to server.");
			currentWidgetRoot->findWidget("errorMsgTextBox")->castType<MyGUI::TextBox>()->setTextColour(MyGUI::Colour::Red);
			return;
		}

		initGameGui();
	}
	else if (senderName == "optionsButton")
	{
		m_guiGameWidgets["optionsWindow"]->castType<MyGUI::Window>()->setVisibleSmooth(!m_guiGameWidgets["optionsWindow"]->getVisible());
	}
	else if (senderName == "shadowsCheckBox")
	{
		bool shadowEnabled = m_pEngineCore->getRenderContext()->getEnableBit("shadow");
		m_pEngineCore->getRenderContext()->setEnableBit("shadow", !shadowEnabled);
		sender->castType<MyGUI::Button>()->setStateSelected(!shadowEnabled);
	}
	else // scripted handler
	{
		LuaManager::getInstance()->callFunction(senderName + "_onClick");
	}
}

void Client::editBox_handler(MyGUI::Widget* sender, MyGUI::KeyCode key, MyGUI::Char _char)
{
	const std::string& senderName = sender->getName();

	if ((key == MyGUI::KeyCode::Return))
	{
		if (senderName == "chatEditBox")
		{
			// chatEditBox enter
			MyGUI::EditBox* chatEditBox = sender->castType<MyGUI::EditBox>();
			if (chatEditBox->getCaption().length() > 0)
			{

				// send chat message
				std::string message(chatEditBox->getCaption());
				sendPacket(network::events::ChatMessage(message));

				chatEditBox->setCaption("");
			}
		}
		else if (senderName == "mouseSensitivityEditBox")
		{
			// mouseSensitivityEditBox enter
			float position = atof( sender->castType<MyGUI::EditBox>()->getCaption().asUTF8_c_str() );

			size_t scrollRange = m_guiGameWidgets["optionsWindow"]->findWidget("mouseSensitivitySlider")->castType<MyGUI::ScrollBar>()->getScrollRange();

			if (position == 0.0f)
			{
				position = 0.01f;
				std::stringstream ss;
				ss << 0.1f;
				sender->castType<MyGUI::EditBox>()->setCaption(ss.str());
			}
			else if (position * 10 >= scrollRange)
			{
				position = scrollRange / 10.0f - 0.1f;
				std::stringstream ss;
				ss << position;
				sender->castType<MyGUI::EditBox>()->setCaption(ss.str());
			}

			m_guiGameWidgets["optionsWindow"]->findWidget("mouseSensitivitySlider")->castType<MyGUI::ScrollBar>()->setScrollPosition((int)(position * 10));

			m_configs.m_mouseSensitivity = position;
		}
		else // scripted handler
		{
			LuaManager::getInstance()->callFunction(senderName + "_onButtonPressed", sender->castType<MyGUI::EditBox>()->getCaption());
		}
	}
}

void Client::scrollChangePosition_handler(MyGUI::ScrollBar* sender, size_t position)
{
	const std::string& senderName = sender->getName();

	if (senderName == "mouseSensitivityEditBox")
	{
		if (position == 0)
		{
			position = 0.1f;
			sender->setScrollPosition(position);
		}

		std::stringstream ss;
		ss << (float)(position / 10.0f);
		m_guiGameWidgets["optionsWindow"]->findWidget("mouseSensitivityEditBox")->castType<MyGUI::EditBox>()->setCaption(ss.str());

		m_configs.m_mouseSensitivity = position / 10.0f;
	}
	else // scripted handler
	{
		LuaManager::getInstance()->callFunction(senderName + "_onScrollChange", position);
	}
}

void Client::comboBox_handler(MyGUI::ComboBox* sender, size_t index)
{
	const std::string& senderName = sender->getName();

	if (senderName == "resolutionComboBox")
	{
		std::string itemData( sender->getItemNameAt(index).asUTF8_c_str() );
		m_configs.width  = atoi( itemData.substr(0, itemData.find("x")).c_str() );
		m_configs.height = atoi( itemData.substr(itemData.find("x") + 1, itemData.length()).c_str() );

		glutReshapeWindow(m_configs.width, m_configs.height);

		m_pEngineCore->onScreenResize(m_configs.width, m_configs.height);
	}
	else if (senderName == "effectsQComboBox")
	{
		graphics::Shader::ms_maxShaderQuality = index - 2;
	}
	else if (senderName == "mapTextureQComboBox")
	{
		// 0 - low, 1 - medium, 2 - high
		m_pEngineCore->reloadTextures(3.0f - index, true);
	}
	else if (senderName == "modelTextureQComboBox")
	{
		// 0 - low, 1 - medium, 2 - high
		m_pEngineCore->reloadTextures(3.0f - index, false);
	}
	else // scripted handler
	{
		std::string itemData( sender->getItemNameAt(index).asUTF8_c_str() );

		LuaManager::getInstance()->callFunction(senderName + "_onSelectionChange", index, itemData);
	}

}

void Client::windowClose_handler(MyGUI::Window* sender, const std::string& name)
{
	if (sender->getName() == "optionsWindow")
	{
		m_guiGameWidgets["optionsWindow"]->castType<MyGUI::Window>()->setVisibleSmooth(false);
	}
}

void Client::handleSpecialKeyDown(int key)
{
	int state = glutGetModifiers();
	if (state & GLUT_ACTIVE_SHIFT)
	{
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::RightShift);
	}

	MyGUI::KeyCode::Enum scanCode = glutMyguiKeyMap[key];
	MyGUI::InputManager::getInstance().injectKeyPress(scanCode, 0);
}

void Client::handleSpecialKeyUp(int key)
{
	if (MyGUI::InputManager::getInstance().isShiftPressed())
	{
		MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::RightShift);
	}

	MyGUI::KeyCode::Enum scanCode = glutMyguiKeyMap[key];
	MyGUI::InputManager::getInstance().injectKeyRelease(scanCode);
}

void Client::frameEvent_handler(float time)
{
	//computeMouseMove();
}

void Client::setLoadingProgress(int progress, std::string message)
{
	if (!m_guiRoot.empty())
	{
		MyGUI::WidgetPtr currentWidgetRoot = m_guiRoot.at(1);

		currentWidgetRoot->setVisible(true);
		currentWidgetRoot->castType<MyGUI::Window>()->setCaption(message);
		currentWidgetRoot->findWidget("progressBar")->castType<MyGUI::ProgressBar>()->setProgressPosition(progress);

		renderGui();
		glutSwapBuffers();
	}
}


void Client::updateGuiViewSize(int width, int height)
{
	m_pGuiPlatform->getRenderManagerPtr()->setViewSize(width, height);
}

} // namespace network

#endif
