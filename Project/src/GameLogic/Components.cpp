#include "GameStdAfx.h"
#include "GameLogic/Components.h"


template <typename Archive>
void Movement::serialize(Archive& ar, const uint version)
{
	ar& boost::serialization::base_object<PersistentComponent>(*this);
	serializeFields(ar, pos, NetworkPriority::HIGH,
						vel, NetworkPriority::HIGH);
}

SERIALIZABLE(Movement);


template <typename Archive>
void Health::serialize(Archive& ar, const uint version)
{
	ar& boost::serialization::base_object<PersistentComponent>(*this);
	serializeFields(ar, maxHealth,
						health);
}

SERIALIZABLE(Health);
