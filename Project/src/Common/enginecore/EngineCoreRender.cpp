#include "GameStdAfx.h"
#ifdef CLIENT_SIDE
#include "Common/EngineCore/EngineCore.h"

#include <imageLoad.h>
#include "Graphics/Camera.h"
#include "Graphics/RenderContext.h"
#include "Graphics/ShadedMesh.h"


bool EngineCore::setupShaders()
{
	m_shaderDirectory["simplest"] = graphics::ShaderPtr(new graphics::Shader(getResourcesDir() + "shaders/simplest"));
	m_shaderDirectory["storeDepth"] = graphics::ShaderPtr(new graphics::Shader(getResourcesDir() + "shaders/storeDepth"));

	m_shaderDirectory["cubeMappingShader"] = graphics::ShaderPtr(new graphics::Shader(getResourcesDir() + "shaders/skybox"));

	m_shaderDirectory["cubeMappingShader"]->setupUniforms({ "tex", "environmentMap" });
	m_shaderDirectory["cubeMappingShader"]->setUniform1i("tex", 0);
	m_shaderDirectory["cubeMappingShader"]->setUniform1i("environmentMap", 1);

	m_shaderDirectory["newone"] = graphics::ShaderPtr(new graphics::Shader(getResourcesDir() + "shaders/newone"));

	setupPostProcessing();

	return true;
}

void EngineCore::onScreenResize(const int width, const int height)
{
	m_configs.width = width;
	m_configs.height = height;

	m_pRenderContext->m_viewPortWidth = width;
	m_pRenderContext->m_viewPortHeight = height;

	setupFBOs();
}

/**
*	Sets up the frame buffer objects used for post processing.
*/
void EngineCore::setupFBOs()
{
	// release old fbos
	if(m_fbos[0])
	{
		glDeleteFramebuffers(NUM_FBOS, m_fbos);
	}


	glGenFramebuffers(NUM_FBOS, m_fbos);

	glGenTextures(NUM_FBOS, m_fboColorTextures);
	glGenTextures(NUM_FBOS, m_fboDepthTextures);

	for(int i = 0; i < NUM_FBOS; i++)
	{
		// create texture for the fbo
		glBindTexture(GL_TEXTURE_2D, m_fboColorTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_configs.width, (GLsizei)m_configs.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


		// create depth texture for the fbo
		glBindTexture(GL_TEXTURE_2D, m_fboDepthTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (GLsizei)m_configs.width, (GLsizei)m_configs.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



		// create the FBO
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[i]);

		// attach the textures to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fboDepthTextures[i], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboColorTextures[i], 0);


		// check for errors
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if(status != GL_FRAMEBUFFER_COMPLETE)
		{
			TRACE_ERROR("Error: Framebuffer error.\n", 0);

			switch(status)
			{
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				TRACE_ERROR("Error: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.\n", 0);
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				TRACE_ERROR("Error: GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS.\n", 0);
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				TRACE_ERROR("Error: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.\n", 0);
				break;

			case GL_FRAMEBUFFER_UNSUPPORTED:
				TRACE_ERROR("Error: GL_FRAMEBUFFER_UNSUPPORTED.\n", 0);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		CheckGLError();
	}

	//
}

/**
* Sets up the post processing: load the shader sources and builds the full screen quad vbo.
*/
void EngineCore::setupPostProcessing()
{
	setupFBOs();

	const std::string postProcShaderPath = getResourcesDir() + "shaders/PostProc/";
	const std::string postProcVertexShader = postProcShaderPath + "PostProcShaderVS.vert";

	// load shaders
	m_shaderDirectory["blurShaderX"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "BlurXShaderFS.frag"));
	m_shaderDirectory["blurShaderY"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "BlurYShaderFS.frag"));

	m_shaderDirectory["grayScaleShader"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "GrayScaleShaderFS.frag"));

	m_shaderDirectory["bloomPreShader"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "Bloom/BloomPreShaderFS.frag"));
	m_shaderDirectory["bloomPreShader"]->setupUniforms({ "u_limit" });

	m_shaderDirectory["bloomPostShader"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "Bloom/BloomPostShaderFS.frag"));
	m_shaderDirectory["bloomPostShader"]->setupUniforms({ "s_blurBuffer" });

	m_shaderDirectory["depthOfFieldShader"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "DepthOfFieldShaderFS.frag"));
	m_shaderDirectory["depthOfFieldShader"]->setupUniforms({ "u_clarity", "u_near", "u_far", "u_fadeDistance", "s_blurBuffer", "s_depthBuffer" });

	CheckGLError();


	// init full screen quad
	vec3 verticesData[4];
	verticesData[0] = vec3(-1, 1, 0);
	verticesData[1] = vec3(1, 1, 0);
	verticesData[2] = vec3(-1, -1, 0);
	verticesData[3] = vec3(1, -1, 0);

	glGenBuffers(1, &m_fullScreenQuadVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_fullScreenQuadVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesData), verticesData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CheckGLError();
}

/**
* Reloads the map or model textures with new size. The new size is <imageSize> x <textureResolutionDiv>.
*
* @param textureResolutionDiv	The texture division factor.
* @param levelTextures			Reload the level or the model textures.
*/
void EngineCore::reloadTextures(const float textureResolutionDiv, const bool levelTextures)
{
	if(levelTextures)
	{
		if(m_currentMapTextureResolutionDiv == textureResolutionDiv)
		{
			return;
		}

		m_currentMapTextureResolutionDiv = textureResolutionDiv;

		for(auto entry : m_textureDirectory)
		{
			if(entry.first.find("/bsp/") != std::string::npos)
			{
				glDeleteTextures(1, &entry.second);
				loadTexture(entry.first, entry.second, m_textureDirectory, true, textureResolutionDiv);
			}
		}
	}
	else
	{
		if(m_currentTextureResolutionDiv == textureResolutionDiv)
		{
			return;
		}

		m_currentTextureResolutionDiv = textureResolutionDiv;

		for(auto entry : m_textureDirectory)
		{
			if(entry.first.find("/bsp/") == std::string::npos)
			{
				glDeleteTextures(1, &entry.second);
				loadTexture(entry.first, entry.second, m_textureDirectory, true, textureResolutionDiv);
			}
		}
	}

	TRACE_INFO("Textures reloaded.", 0);
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
	renderQuad(m_shaderDirectory["grayScaleShader"]->getProgram(), 3);
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
	const GLuint bloomPreShaderProgram = m_shaderDirectory["bloomPreShader"]->getProgram();
	const GLuint bloomPostShaderProgram = m_shaderDirectory["bloomPostShader"]->getProgram();

	const GLuint blurShaderXProgram = m_shaderDirectory["blurShaderX"]->getProgram();
	const GLuint blurShaderYProgram = m_shaderDirectory["blurShaderY"]->getProgram();

	glUseProgram(bloomPreShaderProgram);

	const float bloomLimit = m_pRenderContext->getContextFloatParam("bloomLimit");

	// limit
	m_shaderDirectory["bloomPreShader"]->setUniform1f("u_limit", bloomLimit);

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
	m_shaderDirectory["bloomPostShader"]->setUniform1i("s_blurBuffer", 7);

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

	const GLuint blurShaderXProgram = m_shaderDirectory["blurShaderX"]->getProgram();
	const GLuint blurShaderYProgram = m_shaderDirectory["blurShaderY"]->getProgram();

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
	const graphics::ShaderPtr depthOfFieldShader = m_shaderDirectory["depthOfFieldShader"];

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
	//for (const auto& entry : getLightSourceDirectory())
	//{
	//	LightSourcePtr light = entry.second;
	//	if (light->isVisible() && light->getType() == LightSource::LIGHTTYPE_SPOT)
	//	{
	//		// pass 1
	//		light->shadowingPass1(*m_pRenderContext);

	//		// render scene
	//		//m_pMap->updateObservers(m_pRenderContext->m_pCamera);
	//		//m_pPlayer->setVisibility(true);						// always show the player
	//		///m_pRootNode->render(*m_pRenderContext);

	//		///m_pRenderContext->setEnableBit("shadow", true);

	//		// pass 2
	//		///m_pRenderContext->setViewPort(m_configs.width, m_configs.height);

	//		glBindFramebuffer(GL_FRAMEBUFFER, fboTarget);
	//		light->shadowingPass2(*m_pRenderContext);
	//	}
	//}
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
	///getLightSource("light0")->setPos(m_pPlayer->getPos());

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

	// TODO: render renderable components

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
