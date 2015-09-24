#include "GameStdAfx.h"
#include "EngineCore.h"

#include "GameLogic/Camera.h"
#include "GameLogic/NodeGroup.h"
#include "GameLogic/Player.h"

#ifdef CLIENT_SIDE
#include "Sound/SoundSource.h"
#include "GameLogic/LightSource.h"
#endif

#include <entityx/entityx.h>

/**
 * Animates the game scene.
 *
 * @param dt Delta time.
 */
void EngineCore::animate(float dt)
{
	dt *= CONST_FLOAT("Gameplay::GameSpeedMultiplier");

	LuaManager::getInstance()->callFunction("animateSceneL", dt);

	// remove deleted nodes
	for (const std::string& deletedNodeName : m_deletedNodes)
	{
		m_pRootNode->removeByName(deletedNodeName);
	}
	m_deletedNodes.clear();

	//m_pMap->animate(dt);
	m_pRootNode->animate(dt);

#ifdef CLIENT_SIDE
	//SoundSource::setListener(m_pPlayer->getEyePos(), vec3(0.0f), vec3(0, 0, -1), vec3(0, 1, 0));
	LightSource::animateLights();
#endif
}

#ifdef CLIENT_SIDE
/**
* The main render call: clears the screen, renders the game scene and the fps info.
*
* @param debugLevel The level of visual debugging for the physical scene: 0 - no debug info; 1 - render actors; 2 - render actors and other debug stuff (like joint constraints)
*/
void EngineCore::render(const int debugLevel)
{
	if (m_pRenderContext->getEnableBit("wireframe"))
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

/**
 * Renders with gray scale effect.
 *
 * @param fboTarget		The target fbo.
 * @param debugLevel	The render debug level.
 */
void EngineCore::renderGrayScale(const GLuint fboTarget, const int debugLevel)
{
	renderScene(m_fbos[3], debugLevel);

	glBindFramebuffer(GL_FRAMEBUFFER, fboTarget);
	renderQuad(graphics::getEffect("grayScaleShader")->getProgram(), 3);
}

/**
 * Renders with bloom effect.
 *
 * @param fboTarget		The target fbo.
 * @param debugLevel	The render debug level.
 */
void EngineCore::renderBloom(const GLuint fboTarget, const int debugLevel)
{
	renderScene(m_fbos[0], debugLevel);

	// post processing

	// bloom pre
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[2]);
	const GLuint bloomPreShaderProgram = graphics::getEffect("bloomPreShader")->getProgram();
	const GLuint bloomPostShaderProgram = graphics::getEffect("bloomPostShader")->getProgram();

	const GLuint blurShaderXProgram = graphics::getEffect("blurShaderX")->getProgram();
	const GLuint blurShaderYProgram = graphics::getEffect("blurShaderY")->getProgram();

	glUseProgram(bloomPreShaderProgram);

	const float bloomLimit = m_pRenderContext->getContextFloatParam("bloomLimit");

	// limit
	graphics::getEffect("bloomPreShader")->setUniform1f("u_limit", bloomLimit);

	renderQuad(bloomPreShaderProgram, 0);

	// blur steps
	static const uint blurSteps = 5;
	for (uint i = 0; i < blurSteps; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[1]);
		renderQuad(blurShaderXProgram, 2, (float)( 1 << i));

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[2]);
		renderQuad(blurShaderYProgram, 1, (float)( 1 << i));
	}


	// bloom post
	glBindFramebuffer(GL_FRAMEBUFFER, fboTarget);

	glUseProgram(bloomPostShaderProgram);

	// bind color buffer from fbo
	graphics::getEffect("bloomPostShader")->setUniform1i("s_blurBuffer", 7);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, m_fboColorTextures[2]);

	renderQuad(bloomPostShaderProgram, 0);
}

/**
 * Renders with depth of field effect.
 *
 * @param fboTarget		The target fbo.
 * @param debugLevel	The render debug level.
 */
void EngineCore::renderDOF(const GLuint fboTarget, const int debugLevel)
{
	renderBloom(m_fbos[3]);

	const GLuint blurShaderXProgram = graphics::getEffect("blurShaderX")->getProgram();
	const GLuint blurShaderYProgram = graphics::getEffect("blurShaderY")->getProgram();

	// blur steps
	static const uint blurSteps = 5;
	for (uint i = 0; i < blurSteps; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[1]);
		if (i == 0)
		{
			renderQuad(blurShaderXProgram, 3, (float)(1 << i));
		}
		else
		{
			renderQuad(blurShaderYProgram, 2, (float)(1 << i));
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[2]);
		if (i == 0)
		{
			renderQuad(blurShaderYProgram, 1, (float)(1 << i));
		}
		else
		{
			renderQuad(blurShaderXProgram, 1, (float)(1 << i));
		}
	}


	// dof
	const graphics::ShaderPtr depthOfFieldShader = graphics::getEffect("depthOfFieldShader");

	const float dofFadeDist	= m_pRenderContext->getContextFloatParam("dofFadeDist");
	const float dofSaturation = m_pRenderContext->getContextFloatParam("dofSaturation");

	glUseProgram(depthOfFieldShader->getProgram());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// clarity		u_near + saturation * (u_far - u_near)
	const float clarity = 1.0f + dofSaturation * (1000.0f - 1.0f);

	depthOfFieldShader->setUniform1f("u_clarity", clarity);
	depthOfFieldShader->setUniform1f("u_near", 1.0f);
	depthOfFieldShader->setUniform1f("u_far", 1000.0f);
	depthOfFieldShader->setUniform1f("u_fadeDistance", dofFadeDist);
	depthOfFieldShader->setUniform1i("s_blurBuffer", 7);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, m_fboColorTextures[2]);

	depthOfFieldShader->setUniform1i("s_depthBuffer", 6);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_fboDepthTextures[0]);

	renderQuad(depthOfFieldShader->getProgram(), 3);
}

/**
 * Renders the shadow maps of the spot lights (creates a depth buffer from the light source's view).
 *
 * @param fboTarget The target frame buffer the screen is rendered after creating the shadow map.
 */
void EngineCore::renderShadowMaps(const GLuint fboTarget)
{
	for (const auto& entry : getLightSourceDirectory())
	{
		LightSourcePtr light = entry.second;
		if (light->isVisible() && light->getType() == LightSource::LIGHTTYPE_SPOT)
		{
			// pass 1
			light->shadowingPass1(*m_pRenderContext);

			// render scene
			//m_pMap->updateObservers(m_pRenderContext->m_pCamera);
			//m_pPlayer->setVisibility(true);						// always show the player
			m_pRootNode->render(*m_pRenderContext);

			m_pRenderContext->setEnableBit("shadow", true);

			// pass 2
			m_pRenderContext->setViewPort(m_configs.width, m_configs.height);

			glBindFramebuffer(GL_FRAMEBUFFER, fboTarget);
			light->shadowingPass2(*m_pRenderContext);
		}
	}
}

/**
 * Renders the scene to the target fbo.
 *
 * @param fboTarget		The target fbo.
 * @param debugLevel	The render debug level.
 */
void EngineCore::renderScene(const GLuint fboTarget, const int debugLevel)
{
	// TODO move to animateSceneL()
	getLightSource("light0")->setPos(m_pPlayer->getPos());

	m_pRenderContext->m_pShader = nullptr;
	m_pRenderContext->m_dt = 0;
	m_pRenderContext->m_pCamera = m_pCamera;

	glClearColor(0.0, 0.1, 0.5, 1.0);

	if (m_pRenderContext->getEnableBit("shadow"))
	{
		renderShadowMaps(fboTarget);
	}


	glBindFramebuffer(GL_FRAMEBUFFER, fboTarget);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pRenderContext->m_pShader = nullptr;
	m_pRenderContext->m_roleName = "basic";

	// setup player camera
	//m_pCamera->update(m_pPlayer->getEyePos(), m_pPlayer->getRot());
	m_pCamera->setPerspective(45, m_configs.width, m_configs.height, 1.0f, 400.0f);

	// render scene
	//m_pMap->updateObservers(m_pCamera);
	//m_pPlayer->setVisibility(true);					// always show the player


	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glActiveTexture(GL_TEXTURE0);

	//// render PhysX scene
	//graphics::Shader::disableAll();
	//physics::PhysicsManager::getInstance()->render(debugLevel);
}

/**
 *	Prints info to the corner of the screen.
 */
void EngineCore::renderFPS()
{
	m_frame++;
	m_elapsedTime = glutGet(GLUT_ELAPSED_TIME);
	if (m_elapsedTime - m_timeBase > 1000)
	{
		m_fps = m_frame * 1000.0f / (m_elapsedTime - m_timeBase);
		m_timeBase = m_elapsedTime;
		m_frame = 0;
	}


	utils::gfx::printGL(utils::conversion::intToStr(m_fps), vec3(1, 1, 0));

	//std::stringstream ss;
	//ss << "<color 0 1 0.5>health: </color>\t\t<color 1 1 0>";
	//ss << utils::conversion::floatToStr( m_player->getHealth() );
	//ss << "</color>\n";
	//ss << "<color 0 1 0.5>weapon: </color>\t\t<color 1 1 0>";
	//ss << m_player->getSelectedWeapon()->getName();
	//ss << "</color>\n";
	//ss << "<color 0 1 0.5>bullets left: </color>\t<color 1 1 0>";
	//ss << utils::conversion::intToStr( m_player->getSelectedWeapon()->getBulletCount() );
	//ss << "</color>\n\n";
	//ss << "<color 0 1 0.5>player pos: </color>\t<color 1 1 0>";
	//ss << utils::conversion::vec3ToStr(m_player->getPos());
	//ss << "</color>\n";

	////vec3 textPos = utils::gfx::printGL(text, vec3(1, 2, 0));
	//utils::gfx::printGL(ss.str(), vec3(1, 2, 0));

	////utils::gfx::printGL(";", textPos);
	utils::gfx::flushScreenTextBuffer();

	utils::gfx::drawCrosshair();
}

/**
 * Renders a full screen quad using the given color texture (used for post processing effects).
 *
 * @param program	The active openGL program id.
 * @param fboTexId	The texture id of the source color texture.
 * @param strength	Controls the strength of the blurring effects.
 */
void EngineCore::renderQuad(const GLuint program, const GLuint fboTexId, const float strength)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisable(GL_CULL_FACE);

	glClearColor(0.0f, 0.5f, 1.0f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glBindBuffer(GL_ARRAY_BUFFER, m_fullScreenQuadVbo);

	// bind color buffer from fbo
	int uniformLoc;
	if ((uniformLoc = glGetUniformLocation(program, "s_colorBuffer")) != -1)
	{
		glUniform1i(uniformLoc, 8);
	}

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, m_fboColorTextures[fboTexId]);


	// pixelsize
	if ((uniformLoc = glGetUniformLocation(program, "u_pixelSize")) != -1)
	{
		glUniform2f(uniformLoc, strength / m_configs.width, strength / m_configs.height);
	}


	static const size_t vertStride = sizeof(vec3);

	GLint attribLoc;
	if ((attribLoc = glGetAttribLocation(program, "a_posL")) != -1)
	{
		glEnableVertexAttribArray(attribLoc);
		glVertexAttribPointer(attribLoc, 3, GL_FLOAT, GL_FALSE, vertStride, 0);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	glActiveTexture(GL_TEXTURE0);
	glDisableVertexAttribArray(attribLoc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);

	glEnable(GL_CULL_FACE);
}
#endif
