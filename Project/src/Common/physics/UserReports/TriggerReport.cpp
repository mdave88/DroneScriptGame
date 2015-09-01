#include "GameStdAfx.h"
#include "Common/physics/UserReports/TriggerReport.h"

#include "GameLogic/Entity.h"


void TriggerReport::onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
{
	const NxActor& actor = otherShape.getActor();
	const NxActor& triggerActor = triggerShape.getActor();

	const char* triggerName = triggerActor.getName();
	const int triggerData = (int) triggerActor.userData;
	const Entity* entity = (Entity*) actor.userData;
	if (entity && !entity->getName().empty())
	{
		LuaManager::getInstance()->callFunction(triggerName, status, entity->getName(), triggerData);
	}

	//if (status & NX_TRIGGER_ON_ENTER) {
	//	LuaManager::getInstance()->callFunction(triggerName + "_Enter", entity->getName(), triggerData);
	//}
	//if (status & NX_TRIGGER_ON_LEAVE) {
	//	LuaManager::getInstance()->callFunction(triggerName + "_Leave", entity->getName(), triggerData);
	//}
	//if (status & NX_TRIGGER_ON_STAY) {
	//	LuaManager::getInstance()->callFunction(triggerName + "_Stay", entity->getName(), triggerData);
	//}
}
