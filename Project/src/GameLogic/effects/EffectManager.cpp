#include "GameStdAfx.h"
#include "EffectManager.h"

#include <imageLoad.h>

#include "Common/enginecore/EngineCore.h"

#include "Graphics/RenderContext.h"

#include "Models/mesh/Object.h"
#include "Models/mesh/Mesh.h"

#include "GameLogic/Entity.h"
#include "GameLogic/NodeGroup.h"
#include "GameLogic/Camera.h"


namespace effects
{

EffectManager::EffectManager(float globalLifeTime)
	:	m_globalLifeTime(globalLifeTime),
		m_currentEffectIndex(0)
{
}

void EffectManager::init()
{
	// load textures
	loadTexture(getDataDir() + "textures/effects/decals/decal2.png", m_decalMap, graphics::getTextureDirectory());
	loadTexture(getDataDir() + "textures/effects/decals/decal2_nh.tga", m_normalHeightMap, graphics::getTextureDirectory());
}

void EffectManager::addEffect(const Effect* pEffect)
{
	//EngineCore::getInstance()->getRootNode()->addWithName((Node*) pBulletHole, "h");
	EngineCore::getInstance()->getRootNode()->addWithName((Node*) pEffect, "e", true);
}

void EffectManager::render(const graphics::RenderContext& context)
{
}

void EffectManager::animate(const float dt)
{
	m_globalLifeTime -= dt;
	// delete dead bulletHoles
}

} // namespace effects
