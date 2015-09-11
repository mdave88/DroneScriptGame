#pragma once

#include "GameLogic/Node.h"

namespace physics
{
class Explosion;
}

namespace effects
{

class ExplosionEffect : public Node
{
public:
	ExplosionEffect();
	virtual ~ExplosionEffect();

	virtual void render(const graphics::RenderContext& context);
	virtual void animate(const float dt);

private:
	float				m_lifeTime;

	GLuint				m_decalMap;

	physics::Explosion*	m_pExplosion;
};

typedef std::shared_ptr<ExplosionEffect> ExplosionEffectPtr;

} // namespace effects
