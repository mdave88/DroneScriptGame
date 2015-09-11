#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include "connection.h"
#include "GameState.h"
#include "events/KeyEvent.h"
#include "events/MouseEvent.h"
#include "events/LuaCommand.h"
#include "events/ChatMessage.h"
#include "events/Killshot.h"
#include "events/PlayerReadyEvent.h"
#include "events/PlayerDisconnectingEvent.h"
