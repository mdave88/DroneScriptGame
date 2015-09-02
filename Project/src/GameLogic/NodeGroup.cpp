#include "GameStdAfx.h"
#include "NodeGroup.h"

#include "Common/enginecore/EngineCore.h"
#include "GameLogic/LightSource.h"

#include <typeinfo>


uint16_t NodeGroup::s_elementIndex = 0;

NodeGroup::NodeGroup(bool registerNodes) : m_registerNodes(registerNodes)
{
}

NodeGroup::~NodeGroup()
{
	// the lua side shared_ptr-s should be deleted last on the lua side
	// delete pRootNode is called before lua.close()
	NodeDirectory& nodeDirectory = EngineCore::getInstance()->getNodeDirectory();
	NodeIdDirectory& nodeIdDirectory = EngineCore::getInstance()->getNodeIdDirectory();
	for (const auto& entry : m_subnodes)
	{
		nodeDirectory.erase(entry.first);
		nodeIdDirectory.erase(entry.second->getId());
	}
}

void NodeGroup::add(Node* pElement)
{
	const std::string name = utils::getClassName(*pElement);
	addSPWithName(NodePtr(pElement), name, true);
}

void NodeGroup::addSP(NodePtr& element)
{
	const std::string name = utils::getClassName(*element);
	addSPWithName(element, name, true);
}

void NodeGroup::addNoName(Node* pElement)
{
	//std::string uniqueName = utils::conversion::intToStr(elementIndex);
	//m_subnodes[uniqueName] = NodePtr(pElement);
	//getNodeIdDirectory()[elementIndex++] = m_subnodes[uniqueName];
	EngineCore::getInstance()->getNodeIdDirectory()[s_elementIndex] = NodePtr(pElement);
	pElement->setId(s_elementIndex);
	s_elementIndex++;
}

void NodeGroup::addWithName(Node* pElement, const std::string& name, const bool calcIndex)
{
	addSPWithName(NodePtr(pElement), name, calcIndex);
}

void NodeGroup::addSPWithName(NodePtr& element, const std::string& name, const bool calcIndex)
{
	std::string nodeName = name;
	const uint16_t elementIndex = element->getId() != 0 ? element->getId() : s_elementIndex;

	if (calcIndex)
	{
		nodeName += utils::conversion::intToStr(elementIndex);
	}

	m_subnodes[nodeName] = element;

	if (name.find("light") != std::string::npos)
	{
		getLightSourceDirectory()[nodeName] = std::static_pointer_cast<LightSource>(m_subnodes[nodeName]);
	}

	if (!m_registerNodes)
	{
		return;
	}

	EngineCore::getInstance()->getNodeDirectory()[nodeName] = m_subnodes[nodeName];
	EngineCore::getInstance()->getNodeIdDirectory()[elementIndex] = m_subnodes[nodeName];

	element->setName(nodeName);
	element->setId(elementIndex);

	s_elementIndex = EngineCore::getInstance()->getNodeIdDirectory().size();
}

void NodeGroup::removeByName(const std::string& name)
{
	EngineCore::getInstance()->getNodeIdDirectory().erase(EngineCore::getInstance()->getNode(name)->getId());
	EngineCore::getInstance()->getNodeDirectory().erase(name);
	m_subnodes.erase(name);
}

void NodeGroup::removeSP(const NodePtr& element)
{
	const Node* pElement = element.get();

	const auto elementMatcher = [pElement](const std::pair<const std::string, std::shared_ptr<Node>>& entry)
	{
		return entry.second.get() == pElement;
	};
	const auto& it = std::find_if(m_subnodes.begin(), m_subnodes.end(), elementMatcher);
	if (it != m_subnodes.end())
	{
		EngineCore::getInstance()->getNodeIdDirectory().erase(it->second->getId());
		EngineCore::getInstance()->getNodeDirectory().erase(it->first);
		m_subnodes.erase(it);
	}
}

void NodeGroup::render(const graphics::RenderContext& context)
{
	if (!m_isVisible)
	{
		return;
	}

	if (context.m_pShader == nullptr)
	{
		graphics::getEffect("newone")->bind();
	}

	for (const auto& subNode : m_subnodes)
	{
		if (subNode.second->isVisible())
		{
			subNode.second->render(context);
		}
	}
}

void NodeGroup::animate(const float dt)
{
	if (!m_isAnimated)
	{
		return;
	}

	for (const auto& subNode : m_subnodes)
	{
		if (subNode.second->isAnimated())
		{
			subNode.second->animate(dt);
		}
	}
}

NodePtr NodeGroup::getSubNodeByName(const std::string& name) const
{
	const NodeDirectory::const_iterator it = m_subnodes.find(name);
	if(it != m_subnodes.end())
	{
		return it->second;
	}

	return NodePtr();
}

NodeDirectory& NodeGroup::getSubNodes()
{
	return m_subnodes;
}

void NodeGroup::setNodeRegistration(bool registerNodes)
{
	m_registerNodes = registerNodes;
}

// register to lua
void NodeGroup::registerMethodsToLua()
{
	using namespace luabind;

	class_<NodeGroup, NodePtr, bases<Node>> thisClass("NodeGroup");
	thisClass.def(constructor<>());
	REG_FUNC("add", &NodeGroup::add);
	REG_FUNC("addSP", &NodeGroup::addSP);
	REG_FUNC("addWithName", &NodeGroup::addWithName);
	REG_FUNC("addSPWithName", &NodeGroup::addSPWithName);
	REG_FUNC("removeByName", &NodeGroup::removeByName);
	REG_FUNC("removeSP", &NodeGroup::removeSP);

	module(LuaManager::getInstance()->getState()) [ thisClass ];
}
