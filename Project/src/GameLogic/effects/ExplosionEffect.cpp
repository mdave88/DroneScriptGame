#include "GameStdAfx.h"
#include "ExplosionEffect.h"


namespace effects
{

ExplosionEffect::ExplosionEffect() : m_lifeTime(0.0f), m_pExplosion(nullptr)
{
}

ExplosionEffect::~ExplosionEffect()
{
}

void ExplosionEffect::render(const graphics::RenderContext& context)
{
}

void ExplosionEffect::animate(const float dt)
{
	m_lifeTime -= dt;

	if (m_lifeTime <= 0)
	{
		m_isVisible = false;
	}
}

} // namespace effects
