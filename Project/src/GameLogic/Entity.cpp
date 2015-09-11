#include "GameStdAfx.h"
#include "GameLogic/Entity.h"

#include "Common/enginecore/EngineCore.h"


Entity::Entity(const vec3& pos)
	: Node(pos)
#ifdef CLIENT_SIDE
	, m_animationFrameTime(0)
	, m_animationFrame(0)
	, m_shadedMesh(graphics::ShadedMeshPtr())
#endif
{
}

Entity::Entity(const std::string& shadedMeshName, const vec3& pos)
#ifdef CLIENT_SIDE
	: m_animationFrameTime(0)
	, m_animationFrame(0)
	, m_shadedMeshName(shadedMeshName)
#endif
{
#ifdef CLIENT_SIDE
	setShadedMesh(EngineCore::getInstance()->getShadedMesh(shadedMeshName));
#endif
}

Entity::~Entity()
{
}

void Entity::animate(const float dt)
{
}

void Entity::render(const graphics::RenderContext& context)
{
#ifdef CLIENT_SIDE
	// render model
	if (m_shadedMesh)
	{
		m_shadedMesh->render(context, this);
	}
#endif
}

float Entity::getDistance(Entity* entity) const
{
	return (m_pos - entity->getPos()).length();
}

#ifdef CLIENT_SIDE
void Entity::setShadedMesh(graphics::ShadedMeshPtr shadedMesh)
{
	m_shadedMesh = shadedMesh;
}

void Entity::setShadedMeshByName(const std::string& shadedMeshName)
{
	m_shadedMeshName = shadedMeshName;
	setShadedMesh(EngineCore::getInstance()->getShadedMesh(shadedMeshName));
}

int Entity::getFrame() const
{
	return m_animationFrame;
}

void Entity::setFrame(float frame)
{
	m_animationFrame = frame;
}
#endif

// register to lua
void Entity::registerMethodsToLua()
{
	using namespace luabind;

#ifdef USE_LUABIND_DEBOOSTIFIED
	class_<Entity, Node> thisClass("Entity");
#else
	class_<Entity, NodePtr, bases<Node>> thisClass("Entity");
#endif
	thisClass.def(constructor<>());
	thisClass.def(constructor<vec3>());

	REG_FUNC("getDistance", &Entity::getDistance);

#ifdef CLIENT_SIDE
	REG_PROP("frameP", &Entity::getFrame, &Entity::setFrame);

	REG_FUNC("setShadedMesh", &Entity::setShadedMesh);
	REG_FUNC("setShadedMeshByName", &Entity::setShadedMeshByName);
#endif

	module(LuaManager::getInstance()->getState()) [ thisClass ];
}

// serialization
bool Entity::updateProperties(const Node& otherN, const bool serverSide)
{
	const Entity& other = (const Entity&) otherN;

	Node::updateProperties(other, serverSide);

	UP_P(m_animationFrame);
	UP_P(m_animationFrameTime);

	UP_P(m_modelMatrix);

	UP_P(m_shadedMeshName);

#ifdef CLIENT_SIDE
	if (!m_shadedMesh && !m_shadedMeshName.empty())  		// for the players it can be empty
	{
		setShadedMeshByName(m_shadedMeshName);
	}
#endif

	return m_attribmask;
}

template <typename Archive>
void Entity::serialize(Archive& ar, const uint version)
{
	ar& boost::serialization::base_object<Node>(*this);

	SER_P_F16(m_animationFrame, (uint8_t) NetworkPriority::HIGH);
	SER_P_F16(m_animationFrameTime, (uint8_t) NetworkPriority::HIGH);

	SER_P(m_modelMatrix);

	SER_P(m_shadedMeshName);
}

SERIALIZABLE(Entity);
