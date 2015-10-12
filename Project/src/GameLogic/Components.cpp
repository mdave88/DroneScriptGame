#include "GameStdAfx.h"
#include "GameLogic/Components.h"


template <typename Archive>
void PersistentComponent::serialize(Archive& ar, const uint version)
{
	QLOG("PersistentComponent::serialize()");
	SER_P(m_networkPriority);	// network priority can change on the fly -> compressions can vary
}

SERIALIZABLE(PersistentComponent);


template <typename Archive>
void Movement::serialize(Archive& ar, const uint version)
{
	QLOG("Movement::serialize()");
	ar& boost::serialization::base_object<PersistentComponent>(*this);

	serializeFields(ar, pos, NetworkPriority::HIGH,
						vel, NetworkPriority::MEDIUM);
}

SERIALIZABLE(Movement);


//template <typename Archive>
//void Health::serialize(Archive& ar, const uint version)
//{
//	ar& boost::serialization::base_object<PersistentComponent>(*this);
//	serializeFields(ar, maxHealth,
//						health);
//}
//
//SERIALIZABLE(Health);
