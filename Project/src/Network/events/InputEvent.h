
#define NOMINMAX

#ifndef INPUT_EVENT_H
#define INPUT_EVENT_H

#include "Network/NetworkObject.h"


namespace network
{

/**
 * @brief Namespace of the different sendable events.
 */
namespace events
{

/**
 * @brief Base class of the input events.
 */
class InputEvent : public NetworkObject
{
public:
	enum InputEventType { NETOBJ_INPUT = NETOBJ_NONE + 100 };

	enum InputEventAction
	{
		INPUTEVENT_USE = 0,

		INPUTEVENT_MOVE_FORWARD,
		INPUTEVENT_MOVE_BACKWARD,
		INPUTEVENT_MOVE_RIGHT,
		INPUTEVENT_MOVE_LEFT,

		INPUTEVENT_RUSH,
		INPUTEVENT_JUMP,
		INPUTEVENT_TOGGLE_CROUCH,

		INPUTEVENT_ATTACK_PRIMARY,
		INPUTEVENT_ATTACK_SECONDARY,

		INPUTEVENT_RELOAD,
		INPUTEVENT_THROW,

		INPUTEVENT_FLASHLIGHT,
		INPUTEVENT_USE_ITEM_SLOT0,
		INPUTEVENT_USE_ITEM_SLOT1,
		INPUTEVENT_USE_ITEM_SLOT2,

		INPUTEVENT_CHANGE_WEAPON_LAST,
		INPUTEVENT_CHANGE_WEAPON_PREV,
		INPUTEVENT_CHANGE_WEAPON_NEXT,
		INPUTEVENT_CHANGE_WEAPON_PRIMARY,	// SLOT 0
		INPUTEVENT_CHANGE_WEAPON_SECONDARY,	// SLOT 1
		INPUTEVENT_CHANGE_WEAPON_MELEE,		// SLOT 2
		INPUTEVENT_CHANGE_WEAPON_ITEM_SLOT0,
		INPUTEVENT_CHANGE_WEAPON_ITEM_SLOT1,
		INPUTEVENT_CHANGE_WEAPON_ITEM_SLOT2,

		INPUTEVENT_NUM
	};


public:
	InputEvent(ushort type = NETOBJ_INPUT) : NetworkObject(type) {}

	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& type;
	}
};

} // namespace events
} // namespace network

//BOOST_CLASS_EXPORT_KEY(network::events::InputEvent);

#endif	// INPUT_EVENT_H
