#pragma once

#include "GameLogic/Node.h"

#ifdef CLIENT_SIDE
#include "Graphics/ShadedMesh.h"
#include "Graphics/RenderContext.h"
#endif

class Entity : public Node
{
	SERIALIZABLE_CLASS

public:
	Entity(const vec3& pos = vec3(0.0f));
	Entity(const std::string& shadedMeshName, const vec3& pos = vec3(0.0f));

	virtual ~Entity();

	virtual void render(const graphics::RenderContext& context);
	virtual void animate(const float dt);

	virtual float getDistance(Entity* entity) const;

#ifdef CLIENT_SIDE
	virtual void setShadedMesh(graphics::ShadedMeshPtr shadedMesh);
	virtual void setShadedMeshByName(const std::string& shadedMeshName);

	int getFrame() const;
	void setFrame(float frame);
#endif


	// register to lua
	static void registerMethodsToLua();

protected:
	float					m_animationFrame;
	float					m_animationFrameTime;

	std::string				m_shadedMeshName;

#ifdef CLIENT_SIDE
	graphics::ShadedMeshPtr	m_shadedMesh;
#endif
};


BOOST_CLASS_EXPORT_KEY(Entity);
