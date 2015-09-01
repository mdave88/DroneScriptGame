
#ifndef CONTACTREPORT_H
#define CONTACTREPORT_H

#include "GameLogic/Entity.h"
#include "Physics/PhysicsManager.h"

class ContactReport: public NxUserContactReport
{
public:
	virtual void onContactNotify(NxContactPair& pair, NxU32 events)
	{
		if (pair.isDeletedActor[0] || pair.isDeletedActor[1])
		{
			return;
		}

		vec3 contactPoint;
		vec3 contactNormal;

		NxContactStreamIterator it(pair.stream);
		while (it.goNextPair())
		{
			//user can also call getShape() and getNumPatches() here
			while (it.goNextPatch())
			{
				//user can also call getPatchNormal() and getNumPoints() here
				contactNormal = NXVEC3_TO_VEC3( it.getPatchNormal() );
				while (it.goNextPoint())
				{
					//user can also call getPoint() and getSeparation() here
					const NxVec3& contactPointNx = it.getPoint();
					contactPoint = NXVEC3_TO_VEC3( contactPointNx );

					//NxVec3 contactForce = (showPointContacts /*&& i.getShapeFlags()&NX_SF_POINT_CONTACT_FORCE*/) ?  contactNormal * it.getPointNormalForce() : pair.sumNormalForce;
					//NxVec3 contactArrowForceTip = contactPoint + contactForce * 0.1f;
					//NxVec3 contactArrowFrictionTip = contactPoint + pair.sumFrictionForce* 0.1f;
					//NxVec3 contactArrowPenetrationTip = contactPoint - contactNormal * i.getSeparation() * 20.0f;
				}
			}
		}

		Entity* c1 = nullptr;
		Entity* c2 = nullptr;

		if (pair.actors[0])
		{
			c1 = static_cast<Entity*>( pair.actors[0]->userData );
		}

		if (pair.actors[1])
		{
			c2 = static_cast<Entity*>( pair.actors[1]->userData );
		}

		if (c1)
		{
			c1->onHit(physics::ContactData(pair.actors[0], pair.actors[1], contactPoint, contactNormal));
		}
		if (c2)
		{
			c2->onHit(physics::ContactData(pair.actors[1], pair.actors[0], contactPoint, contactNormal));
		}
	}
};

#endif  // CONTACTREPORT_H
