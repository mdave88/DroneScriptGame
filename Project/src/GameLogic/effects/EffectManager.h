#pragma once

#include "GameLogic/Node.h"

namespace effects
{

class Effect;

class EffectManager : public Node
{
public:
	EffectManager(float globalLifeTime = 100.0f);
	virtual ~EffectManager() {}

	virtual void init();
	virtual void addEffect(const Effect* pEffect);

	virtual void render(const graphics::RenderContext& context);
	virtual void animate(const float dt);

protected:
	GLuint	m_vbo;
	GLuint	m_decalMap;
	GLuint	m_normalHeightMap;

	float	m_globalLifeTime;

	int		m_currentEffectIndex;
};

} // namespace effects
