
#ifndef PLAYER_H
#define PLAYER_H

#include "GameLogic/Entity.h"

#include "Network/Events/MouseEvent.h"


class Player: public Entity
{
public:
	Player(const vec3& pos = vec3(0.0f), ushort race = 0);
	virtual ~Player();

	virtual bool isPlayer() const { return true; }

	virtual void animate(const float dt);

	virtual void kill();

	// input
	virtual void applyInput(float dt);

	virtual void registerInputHandlers();
	virtual void pushActiveInputEvent(short eventType);

	virtual void setKeyState(uint8_t key, bool state);
	virtual void setMouseState(const network::events::MouseEvent& mouseEvent);


	const vec3 getEyePos() { return vec3(0.0f); }

	// register to lua
	static void registerMethodsToLua();

protected:
	bool m_keys[256];
	bool m_isLeftMB, m_isRightMB;

	//std::set<uint16_t> m_activeInputEvents;
};

#endif
