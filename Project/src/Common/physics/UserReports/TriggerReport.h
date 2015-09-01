
#ifndef TRIGGER_REPORT_H
#define TRIGGER_REPORT_H

#include "NxPhysics.h"

class TriggerReport : public NxUserTriggerReport
{
public:
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status);
};

#endif  // TRIGGER_REPORT_H
