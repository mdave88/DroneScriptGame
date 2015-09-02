#include "GameStdAfx.h"
#include "Player.h"


PlayerDirectory playerDirectory;


Player::Player(const vec3& pos, ushort race)
	: m_isLeftMB(false)
	, m_isRightMB(false)
{
	for (int i = 0; i < 256; i++)
	{
		m_keys[i] = false;
	}

	m_networkPriority = (uint8_t) NetworkPriority::TOP;
}

Player::~Player()
{
	//MQLOG("dest Player", 0)
}

void Player::animate(const float dt)
{
}

void Player::kill()
{
}


// input

void Player::setKeyState(uint8_t key, bool state)
{
	m_keys[key] = state;
}

void Player::setMouseState(const network::events::MouseEvent& mouseEvent)
{
	setRot(getRot() + vec3(mouseEvent.y, mouseEvent.x, 0) / 10.0);

	if (mouseEvent.type != network::events::MouseEvent::NETOBJ_MOUSE_ACTION)
	{
		return;
	}

	switch (mouseEvent.button)
	{
		case network::events::MouseEvent::BUTTON1_DOWN:
			m_isLeftMB = true;
			break;
		case network::events::MouseEvent::BUTTON1_UP:
			m_isLeftMB = false;
			break;
		case network::events::MouseEvent::BUTTON2_DOWN:
			m_isRightMB = true;
			break;
		case network::events::MouseEvent::BUTTON2_UP:
			m_isRightMB = false;
			break;
	}
}

void Player::applyInput(float dt)
{
}

void Player::registerInputHandlers()
{

}

void Player::pushActiveInputEvent(short eventType)
{
}


// register to lua
void Player::registerMethodsToLua()
{
	using namespace luabind;

	class_<Player, NodePtr, bases<Entity>> thisClass("Player");
	thisClass.def(constructor<>());
	thisClass.def(constructor<vec3>());
	thisClass.def(constructor<vec3, int>());

	module(LuaManager::getInstance()->getState()) [ thisClass ];
}
