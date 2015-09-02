#include "GameStdAfx.h"
#include "GameState.h"

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/set.hpp>

#include "GameLogic/NodeGroup.h"

#include "Common/enginecore/EngineCore.h"
#include "console/CrimsonConsole.h"


namespace network
{

GameState::GameState(const ClientTable& clientTable, NodeIdDirectory& updatedEntities) : NetworkObject(NETOBJ_GAMESTATE)
{
	m_hasClientTableChanged = true;

	set(clientTable, updatedEntities);
}

GameState::GameState(const ClientTable& clientTable, NodeIdDirectory& updatedEntities, std::set<int>& deletedEntities) : NetworkObject(NETOBJ_GAMESTATE)
{
	m_hasClientTableChanged = true;

	set(clientTable, updatedEntities, deletedEntities);
}

void GameState::set(const ClientTable& clientTable, NodeIdDirectory& updatedEntities)
{
	setClientTable(clientTable);
	m_updatedEntities = updatedEntities;
}

void GameState::set(const ClientTable& clientTable, NodeIdDirectory& updatedEntities, std::set<int>& deletedEntitiesById)
{
	set(clientTable, updatedEntities);
	m_deletedEntitiesById = deletedEntitiesById;
}

/**
 * Filters out entities invisible to the client.
 * The filter collection is gathered from the BSP map.
 */
NodeIdDirectory GameState::applyFilter(const NodeIdDirectory& entityDirectory)
{
	NodeIdDirectory filteredEntities;
	for (const auto& entry : entityDirectory)
	{
		const uint16_t entryId = entry.first;
		const NodePtr& entryNode = entry.second;

		if (m_filter.find(entryId) != m_filter.end())
		{
			filteredEntities[entryId] = entryNode;
		}
	}

	return filteredEntities;
}

/**
 * Calculates the changes on the server side. The changes are filtered after the first update.
 * Compares the client and the server state and update the client state according to it:
 *	- adding new entities to the clientState
 *	- updating the entities that changed state
 *
 * @param clientState The state of the client.
 * @param serverState The state of the server.
 *
 * @return The number of updated, newly created and deleted entities.
 */
uint GameState::calculateChanges(NodeIdDirectory& clientState, const NodeIdDirectory& serverState)
{
	// find updated and new entities
	m_updatedEntities.clear();
	m_deletedEntitiesById.clear();

	uint changesNum = 0;

	// filter out entities invisible to the client if its not the first update
	NodeIdDirectory filteredState = (clientState.empty()) ? serverState : applyFilter(serverState);
	for (const auto& entry : filteredState)
	{
		const uint16_t entryId = entry.first;
		const NodePtr& entryNode = entry.second;

		if (clientState.find(entryId) != clientState.end())
		{
			// update properties
			if (clientState[entryId]->updateProperties(*entryNode.get()))
			{
				m_updatedEntities[entryId] = clientState[entryId];						// updated -> add to changed entities and send it
				changesNum++;
			}
		}
		else
		{
			// create new entity
			// TODO EntityPtr -> NodePtr
			clientState[entryId] = NodePtr(entryNode->clone());							// new entity	-> update client table
			m_updatedEntities[entryId] = clientState[entryId];							//				-> add to changed entities and send it
			changesNum++;
		}
	}


	// find deleted entities
	for (const auto& entry : clientState)
	{
		if (serverState.find(entry.first) == serverState.end())
		{
			//m_deletedEntities[entry.first] = EntityPtr();								// deleted entity -> add a dummy into the changed entities (only the name is important)
			m_deletedEntitiesById.insert(entry.first);
			changesNum++;
		}
	}

	return changesNum;
}

/**
 * Applies the changes to our node graph on the client side.
 * The new entities in the GameState object are added to the clientEntities directory.
 * The other entities are updated if needed.
 *
 * @param pRootNode The rote of our scene.
 * @param clientEntities The entities that are stored on the client side.
 */
void GameState::apply(NodeGroup* pRootNode, NodeIdDirectory& clientEntities)
{
	// update modified entities
	for (const auto& item : m_updatedEntities)
	{
		const NodePtr& entry = item.second;

		const auto& it = clientEntities.find(item.first);
		if(it != clientEntities.end())
		{
			it->second->updateProperties(*entry.get(), false);
		}
		else
		{
			const std::string& name = entry->getName();
			Node* pNodeClone = entry->clone();
			pNodeClone->setId(item.first);
			pRootNode->addWithName(pNodeClone, name);	// put the clone in the pRootNode (the clone() calls the updateProperties() that initializes the shadedMesh)

			// register its name to the console
			GameConsole::addKeywordToConsole(name);
		}
	}

	// delete entities
	for (const int entityId : m_deletedEntitiesById)
	{
		const auto& clientEntity = clientEntities.find(entityId);
		if (clientEntity != clientEntities.end())
		{
			pRootNode->removeByName(clientEntity->second->getName());
			clientEntities.erase(entityId);
		}
	}
}


// getters-setters
void GameState::setFilter(const std::set<ushort>& filter)
{
	m_filter = filter;
}

const ClientTable& GameState::getClientTable() const
{
	return m_clientTable;
}

/**
 * Sets the client table and test if its changed.
 */
void GameState::setClientTable(const ClientTable& clientTable)
{
	if (m_clientTable.size() != clientTable.size())
	{
		m_hasClientTableChanged = true;
	}

	m_clientTable = clientTable;
}

ClientData& GameState::getClient(const ushort id)
{
	return m_clientTable.at(id);
}


// serialization
template <typename Archive>
void GameState::serialize(Archive& ar, const uint version)
{
	ar& type;
	ar& m_hasClientTableChanged;

	ar& BOOST_SERIALIZATION_NVP(m_updatedEntities);
	ar& m_deletedEntitiesById;
	ar& BOOST_SERIALIZATION_NVP(m_clientTable);

	if (m_hasClientTableChanged)
	{
		//TRACE_NETWORK("clientTable Changed", 0);
		ar& BOOST_SERIALIZATION_NVP(m_clientTable);
		m_hasClientTableChanged = false;
	}
}

SERIALIZABLE_NOT_UPD(GameState);

} // namespace network

BOOST_CLASS_EXPORT(network::GameState);
