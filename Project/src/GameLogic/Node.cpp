#include "GameStdAfx.h"
#include "Node.h"

Node::Node(const vec3& pos, const vec3& rot)
	:	m_id(0), 
		m_pos(pos),
		m_rot(rot),

		m_name(""),
		m_attribmask(0),
		m_attribIndex(0),
		m_isVisible(true),
		m_isAnimated(true),
		m_scale(vec3(1)),
		m_pParentNode(nullptr),
		m_networkPriority((uint8_t) NetworkPriority::HIGH)
{
}

bool Node::isAnimated() const
{
	return m_isAnimated;
}

void Node::setAnimated(bool isAnimated)
{
	m_isAnimated = isAnimated;
}

bool Node::isVisible() const
{
	return m_isVisible;
}

void Node::setVisibility(bool isVisible)
{
	m_isVisible = isVisible;
}

const vec3 Node::getPos() const
{
	return m_pos;
}

void Node::setPos(const vec3& pos)
{
	m_pos = pos;
	m_modelMatrix.setTranslation(pos);
}

const vec3& Node::getRot() const
{
	return m_rot;
}

void Node::setRot(const vec3& rot)
{
	m_rot = rot;
	m_modelMatrix.setRotation(rot);
}

//vec3 Node::getInvRot() const {
//	return rot;
//}

void Node::setInvRot(const vec3& invRot)
{
	//this->rot = rot;
	m_modelMatrix.setInverseRotation(invRot);
}

const vec3& Node::getScale() const
{
	return m_scale;
}

void Node::setScale(const vec3& scale)
{
	m_scale = scale;
}

const Matrix& Node::getModelMatrix() const
{
	return m_modelMatrix;
}

ushort Node::getId() const
{
	return m_id;
}
void Node::setId(ushort id)
{
	m_id = id;
}

const std::string& Node::getName() const
{
	return m_name;
}
void Node::setName(const std::string& name)
{
	m_name = name;
}

void Node::setAttribMask(int attribMask)
{
	m_attribmask = attribMask;
}

void Node::setAttribIndex(short attribIndex)
{
	m_attribIndex = attribIndex;
}


// register methods to lua
void Node::registerMethodsToLua()
{
	using namespace luabind;

	// register vec3
	{
		class_<vec3> thisClass("vec3");

		thisClass.def(constructor<float>());
		thisClass.def(constructor<float, float, float>());

		REG_ATTR("x", &vec3::x);
		REG_ATTR("y", &vec3::y);
		REG_ATTR("z", &vec3::z);

		REG_FUNC("abs", &vec3::abs);
		REG_FUNC("length", &vec3::length);
		REG_FUNC("setLength", &vec3::setLength);
		REG_FUNC("normalize", &vec3::normalize);
		REG_FUNC("isZeroVector", &vec3::isZeroVector);


		thisClass.def(self + vec3());
		thisClass.def(self - vec3());
		thisClass.def(self * vec3());
		thisClass.def(self / vec3());
		thisClass.def(self * float());
		thisClass.def(self / float());
		thisClass.def(self == vec3());

		module(LuaManager::getInstance()->getState()) [ thisClass ];
	}


	class_<Node, NodePtr> thisClass("Node");
	thisClass.def(constructor<vec3>());

	REG_FUNC("isAnimated", &Node::isAnimated);
	REG_FUNC("setAnimated", &Node::setAnimated);
	REG_FUNC("isVisible", &Node::isVisible);
	REG_FUNC("setVisibility", &Node::setVisibility);

	REG_PROP("posP", &Node::getPos, &Node::setPos);
	REG_PROP("rotP", &Node::getRot, &Node::setRot);
	REG_PROP("nameP", &Node::getName, &Node::setName);

	module(LuaManager::getInstance()->getState()) [ thisClass ];
}


// serialization
void Node::setNetworkPriority(int networkPriority)
{
	m_networkPriority = networkPriority;
}

/**
 * Updates the attributes of this object.
 * Server side: if the attribute has changed during this method -> the bit bound to this attribute in the attribmask is set to 1.
 * Client side: if the bit bound to this attribute from the attribmask is 1 -> the attribute has been updated on the server side -> update it.
 *
 * @params other		The other (updated) object.
 * @params serverSide	True if this method is called on the server side.
 */
bool Node::updateProperties(const Node& other, bool serverSide)
{
	if (serverSide)
	{
		m_attribmask = 0;
	}
	else
	{
		m_attribmask = other.m_attribmask;
	}
	m_attribIndex = 0;

	UP_P(m_networkPriority);

	UP_P(m_name);

	UP_P(m_pos);
	UP_P(m_rot);

	return m_attribmask;
}

template <typename Archive>
void Node::serialize(Archive& ar, const uint version)
{
	// reset a direction marking bit
	BIT_CLEAR(m_attribmask, 31);

	// mark the direction on the most significant bit on the attrib mask: 0 - save, 1 - load
	boost::serialization::split_member(ar, *this, version);
	m_attribIndex = 0;

	// network priority can change on the fly -> compressions can vary
	SER_P(m_networkPriority);

	SER_P(m_name);

	SER_P_VEC_F16(m_pos, (uint8_t) NetworkPriority::HIGH);

	// normalize the rotation angles to fit in the float decompression interval
	m_rot = normalizeOrientationVector(m_rot);
	SER_P_VEC_F16(m_rot, (uint8_t) NetworkPriority::HIGH);
}

template<class Archive>
void Node::save(Archive& ar, const uint version) const
{
	ar << m_attribmask;
}

template<class Archive>
void Node::load(Archive& ar, const uint version)
{
	ar >> m_attribmask;

	// mark the direction -> you may need to reset it if the return value updateProperties() is significant on the client
	BIT_SET(m_attribmask, 31);
}

SERIALIZABLE_SEPARATED(Node);
