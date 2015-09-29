#include "GameStdAfx.h"
#include "Common/EngineCore/EngineCore.h"
#include "Common/luamanager/LuaManager.h"

#include "Graphics/RenderContext.h"

/**
 * Animates the game scene.
 *
 * @param dt Delta time.
 */
void EngineCore::animate(float dt)
{
	dt *= CONST_FLOAT("Gameplay::GameSpeedMultiplier");

	LuaManager::getInstance()->callFunction("animateSceneL", dt);

	// TODO: animate components
}

#ifdef CLIENT_SIDE
/**
* The main render call: clears the screen, renders the game scene and the fps info.
*
* @param debugLevel The level of visual debugging for the physical scene: 0 - no debug info; 1 - render actors; 2 - render actors and other debug stuff (like joint constraints)
*/
void EngineCore::render(const int debugLevel)
{
	if(m_pRenderContext->getEnableBit("wireframe"))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		renderScene(debugLevel);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//renderGrayScale(0, debugLevel);
		renderDOF(0, debugLevel);
	}

	renderFPS();
}
#endif
