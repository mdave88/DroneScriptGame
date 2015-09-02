
#ifndef NODEGROUP_H
#define NODEGROUP_H

#include "GameLogic/Node.h"


/**
 * @brief The nodes of the game scene can be grouped into nodegroups. The grouped nodes can share settings (visible, animated, etc.).
 */
class NodeGroup : public Node
{
public:
	NodeGroup(bool registerNodes = true);
	virtual ~NodeGroup();

	void add(Node* pElement);
	void addSP(NodePtr& element);

	void addNoName(Node* pElement);

	void addWithName(Node* pElement, const std::string& name = std::string("node"), const bool calcIndex = false);
	void addSPWithName(NodePtr& element, const std::string& name = std::string("node"), const bool calcIndex = false);

	void removeByName(const std::string& name);
	void removeSP(const NodePtr& e);

	virtual void render(const graphics::RenderContext& context);
	virtual void animate(const float dt);

	// getters-setters
	//virtual bool isAnimated() const;
	//virtual void setAnimated(bool isAnimated);
	//
	//virtual void setVisibility(bool isVisible);

	NodeDirectory& getSubNodes();
	NodePtr getSubNodeByName(const std::string& name) const;

	void setNodeRegistration(bool registerNodes);


	// register to lua
	static void registerMethodsToLua();

	// serialization
	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		ar& boost::serialization::base_object<Node>(*this);
		ar& m_subnodes;
	}

protected:
	static uint16_t	s_elementIndex;

	bool			m_registerNodes;
	NodeDirectory	m_subnodes;
};

#endif // NODEGROUP_H
