
#define NOMINMAX

#ifndef MOUSE_EVENT_H
#define MOUSE_EVENT_H

#include "InputEvent.h"

namespace network
{
namespace events
{

class MouseEvent : public InputEvent
{
public:
	enum MouseEventType { NETOBJ_MOUSE_MOVE = NETOBJ_INPUT + 100, NETOBJ_MOUSE_ACTION, NETOBJ_MOUSE_DRAG };
	enum MouseButtonState { BUTTON1_DOWN, BUTTON1_UP, BUTTON2_DOWN, BUTTON2_UP };

	int x, y;
	int button;


public:
	MouseEvent(ushort type = NETOBJ_MOUSE_MOVE, int x = -1, int y = -1, int button = 0)
		: InputEvent(NETOBJ_MOUSE_MOVE)
		, x(x)
		, y(y)
		, button(button)
	{
	}

	/**
	* Set normalized mouse coordinates. (The server do not need to know about the resolution on the client side.)
	*/
	MouseEvent(ushort type, int x, int y, bool normalize = false, float m_mouseSensitivity = 1.0f, int width = 0, int height = 0, int button = 0)
		: InputEvent(type)
		, x(x)
		, y(y)
		, button(button)
	{
		if (normalize)
		{
			this->x = x * m_mouseSensitivity - (width  * m_mouseSensitivity) / 2.0f;
			this->y = y * m_mouseSensitivity - (height * m_mouseSensitivity) / 2.0f;
		}
	}

	virtual void set(const MouseEvent& otherEvent)
	{
		m_isUpdated = (x != otherEvent.x || y != otherEvent.y || button != otherEvent.button);
		*this = otherEvent;
	}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;

		ar& x;
		ar& y;
		ar& button;
	}
};

} // namespace events
} // namespace network

//BOOST_CLASS_EXPORT(network::events::MouseEvent);

#endif
