
#ifndef NODE_H
#define NODE_H

#include "Graphics/RenderContext.h"
#include "Common/luamanager/LuaManager.h"


/**
 * @brief Base class of the nodes of the game scene.
 */
class Node
{
	SERIALIZABLE_CLASS_SEPARATED

public:
	Node(const vec3& pos = vec3(0.0f), const vec3& rot = vec3(0.0f));
	virtual ~Node() {};

	virtual void render(const graphics::RenderContext& context) {}
	virtual void animate(const float dt) {}

	// getters-setters
	virtual bool isAnimated() const;
	virtual void setAnimated(bool isAnimated);

	virtual bool isVisible() const;
	virtual void setVisibility(bool isVisible);

	virtual const vec3 getPos() const;
	virtual void setPos(const vec3& pos);

	virtual const vec3& getRot() const;
	virtual void setRot(const vec3& rot);

	//virtual vec3 getInvRot() const;
	virtual void setInvRot(const vec3& invRot);

	virtual const vec3& getScale() const;
	virtual void setScale(const vec3& rot);

	virtual const Matrix& getModelMatrix() const;

	virtual ushort getId() const;
	virtual void setId(ushort id);

	virtual const std::string& getName() const;
	virtual void setName(const std::string& name);


	// register to lua
	static void registerMethodsToLua();

	// serialization
	void setAttribMask(int attribMask);
	void setAttribIndex(short attribIndex);

	void setNetworkPriority(int networkPriority);

	/**
	 * Updates a primitive attribute.
	 * Server side: if the attribute has changed during this method -> the bit bound to this attribute in the attribmask is set to 1.
	 * Client side: if the bit bound to this attribute from the attribmask is 1 -> the attribute has been updated on the server side -> update it.
	 *
	 * @params t			The primitive attribute of our object.
	 * @params o			The primitive attribute of the other object.
	 * @params serverSide	True if this method is called on the server side.
	 */
	template <typename T>
	bool updateProperty(T& t, const T& o, bool serverSide = true)
	{
		// client side
		if (!serverSide)
		{
			if (BIT_CHECK(m_attribmask, m_attribIndex++))
			{
				t = o;
				return true;
			}
			return false;
		}

		// server side
		if (t != o)
		{
			t = o;
			BIT_SET(m_attribmask, m_attribIndex++);
			return true;
		}

		BIT_CLEAR(m_attribmask, m_attribIndex++);
		return false;
	}

	/**
	 * Updates a complex attribute given in a boost shared pointer.
	 * Server side: if the attribute has changed during this method -> the bit bound to this attribute in the attribmask is set to 1.
	 * Client side: if the bit bound to this attribute from the attribmask is 1 -> the attribute has been updated on the server side -> update it.
	 *
	 * @params t			The complex attribute of our object.
	 * @params o			The complex attribute of the other object.
	 * @params serverSide	True if this method is called on the server side.
	 */
	template <typename T>
	bool updateComplexPropertySP(std::shared_ptr<T>& t, const std::shared_ptr<T>& o, bool serverSide = true)
	{
		// client side
		if (!serverSide)
		{
			if (BIT_CHECK(m_attribmask, m_attribIndex++))
			{
				//t = o;
				t->updateProperties(*o.get(), serverSide);
				return true;
			}
		}

		// server side
		if (t != o)
		{
			if (!t)
			{
				t = std::shared_ptr<T>(new T());
			}

			t->updateProperties(*o.get(), serverSide);
			BIT_SET(m_attribmask, m_attribIndex++);
			return true;
		}

		BIT_CLEAR(m_attribmask, m_attribIndex++);
		return false;
	}

protected:
	uint8_t		m_id;
	std::string	m_name;

	bool		m_isVisible;
	bool		m_isAnimated;

	vec3		m_pos;
	vec3		m_rot;
	vec3		m_scale;

	Matrix		m_modelMatrix;

	Node*		m_pParentNode;

	// serialization
	uint32_t	m_attribmask;
	uint32_t	m_attribIndex;

	uint8_t		m_networkPriority;
};


#endif // NODE_H
