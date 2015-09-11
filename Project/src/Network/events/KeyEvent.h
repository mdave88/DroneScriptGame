#pragma once

#include "InputEvent.h"

namespace network
{
namespace events
{

class KeyEvent : public InputEvent
{
public:
	enum KeyEventType { NETOBJ_KEY_DOWN = NETOBJ_INPUT + 1, NETOBJ_KEY_UP };

	char keyCode;
	char isAltDown, isCtrlDown, isShiftDown;

public:
	KeyEvent(ushort type = NETOBJ_KEY_DOWN, uint8_t keyCode = 0, bool isAltDown = 0, bool isCtrlDown = 0, bool isShiftDown = 0)
		: InputEvent(type)
		, keyCode(keyCode)
		, isAltDown(isAltDown)
		, isCtrlDown(isCtrlDown)
		, isShiftDown(isShiftDown)
	{
	}

	virtual void set(const KeyEvent& otherEvent)
	{
		m_isUpdated = (type != otherEvent.type || keyCode != otherEvent.keyCode || isAltDown != otherEvent.isAltDown || isCtrlDown != otherEvent.isCtrlDown || isShiftDown != otherEvent.isShiftDown);
		*this = otherEvent;
	}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;

		ar& keyCode;
		ar& isAltDown;
		ar& isCtrlDown;
		ar& isShiftDown;
	}
};


} // namespace events
} // namespace network

//BOOST_CLASS_EXPORT(network::events::KeyEvent);
