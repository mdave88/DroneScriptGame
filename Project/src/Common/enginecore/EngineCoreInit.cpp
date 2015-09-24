#include "GameStdAfx.h"
#include "EngineCore.h"

#include "GameLogic/CrimsonGameLogic.h"

#ifdef CLIENT_SIDE
#include "Models/3ds/Model3ds.h"
#include "Models/md5/ModelMd5.h"

#include "Graphics/shaders/Shader.h"
#include "GameLogic/effects/SoundEmitter.h"
#endif


void EngineCore::release()
{
	//physics::PhysicsManager::getInstance()->getPhysicsResults();

	//Projectile::releaseDeadProjectiles();

	//delete player;
	delete m_pCamera;
	delete m_pRootNode;
#ifdef CLIENT_SIDE
	delete m_pRenderContext;
#endif

	LuaManager::getInstance()->close();

#ifdef CLIENT_SIDE
	for (auto& entry : m_meshDirectory)
	{
		SAFEDEL(entry.second);
	}

	for (auto& entry : graphics::getTextureDirectory())
	{
		glDeleteTextures(1, &entry.second);
	}

	m_soundDirectory["backgroundNoise"]->release();
	SoundSource::audioExit();
#endif

	//CharacterController::releaseControllers();

	//physics::PhysicsManager::getInstance()->exit();
	//physics::PhysicsManager::destroyInstance();
}

#ifdef CLIENT_SIDE
bool EngineCore::setupShaders()
{
	EffectDirectory& effectDirectory = graphics::getEffectDirectory();

	effectDirectory["simplest"] = graphics::ShaderPtr(new graphics::Shader(getResourcesDir() + "shaders/simplest"));
	effectDirectory["storeDepth"] = graphics::ShaderPtr(new graphics::Shader(getResourcesDir() + "shaders/storeDepth"));

	effectDirectory["cubeMappingShader"] = graphics::ShaderPtr(new graphics::Shader(getResourcesDir() + "shaders/skybox"));

	graphics::getEffect("cubeMappingShader")->setupUniforms({ "tex", "environmentMap" });
	graphics::getEffect("cubeMappingShader")->setUniform1i("tex", 0);
	graphics::getEffect("cubeMappingShader")->setUniform1i("environmentMap", 1);


	effectDirectory["newone"] = graphics::ShaderPtr(new graphics::Shader(getResourcesDir() + "shaders/newone"));

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
	if (m_fbos[0])
	{
		glDeleteFramebuffers(NUM_FBOS, m_fbos);
	}


	glGenFramebuffers(NUM_FBOS, m_fbos);

	glGenTextures(NUM_FBOS, m_fboColorTextures);
	glGenTextures(NUM_FBOS, m_fboDepthTextures);

	for (int i = 0; i < NUM_FBOS; i++)
	{
		// create texture for the fbo
		glBindTexture(GL_TEXTURE_2D, m_fboColorTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) m_configs.width, (GLsizei) m_configs.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


		// create depth texture for the fbo
		glBindTexture(GL_TEXTURE_2D, m_fboDepthTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (GLsizei) m_configs.width, (GLsizei) m_configs.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);

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

		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			TRACE_ERROR("Error: Framebuffer error.\n", 0);

			switch (status)
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
	EffectDirectory& effectDirectory = graphics::getEffectDirectory();
	effectDirectory["blurShaderX"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "BlurXShaderFS.frag"));
	effectDirectory["blurShaderY"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "BlurYShaderFS.frag"));

	effectDirectory["grayScaleShader"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "GrayScaleShaderFS.frag"));

	effectDirectory["bloomPreShader"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "Bloom/BloomPreShaderFS.frag"));
	effectDirectory["bloomPreShader"]->setupUniforms({ "u_limit" });

	effectDirectory["bloomPostShader"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "Bloom/BloomPostShaderFS.frag"));
	effectDirectory["bloomPostShader"]->setupUniforms({ "s_blurBuffer" });

	effectDirectory["depthOfFieldShader"] = graphics::ShaderPtr(new graphics::Shader(postProcVertexShader, postProcShaderPath + "DepthOfFieldShaderFS.frag"));
	effectDirectory["depthOfFieldShader"]->setupUniforms({ "u_clarity", "u_near", "u_far", "u_fadeDistance", "s_blurBuffer", "s_depthBuffer" });

	CheckGLError();


	// init full screen quad
	vec3 verticesData[4];
	verticesData[0] = vec3(-1,  1, 0);
	verticesData[1] = vec3( 1,  1, 0);
	verticesData[2] = vec3(-1, -1, 0);
	verticesData[3] = vec3( 1, -1, 0);

	glGenBuffers(1, &m_fullScreenQuadVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_fullScreenQuadVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesData), verticesData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CheckGLError();
}
#endif

EngineCore::EngineCore()
	: m_pPlayer(nullptr)
	, m_pRootNode(nullptr)
	// , m_pMap(nullptr)
#ifdef CLIENT_SIDE
	, m_pShadersNode(nullptr)
	, m_pRenderContext(nullptr)

	, m_currentTextureResolutionDiv(2.0f)
	, m_currentMapTextureResolutionDiv(2.0f)
#endif
	, m_fps(0)
	, m_frame(0)
	, m_elapsedTime(0)
	, m_timeBase(0)
{
	m_pCamera = new Camera();
}

/**
 * Initializes the game logic: loads the game initializer lua scripts.
 */
bool EngineCore::initLogic()
{
	new LuaManager();

	m_configs = m_configs;

	TRACE_INFO("Initializing logic.", 0);

	m_pRootNode = new NodeGroup();

#ifdef CLIENT_SIDE
	m_pShadersNode = new NodeGroup(false);
	m_pRootNode->setNodeRegistration(false);
	m_pRootNode->add(m_pShadersNode);
	m_pRootNode->setNodeRegistration(true);
#endif

	// lua scripts
	TRACE_INFO("Parsing world xml.", 0);

#ifndef CLIENT_SIDE
	const bool loadJustData = false;
#else
	const bool loadJustData = true;
#endif
	parseXml(getDataDir() + "settings/level.xml", loadJustData);

	TRACE_INFO("Loading lua scripts.", 0);

#ifndef SERVER_SIDE
	// client: just register the lua methods
	for(const std::string& script : m_luaDefinitonScripts)
	{
		LuaManager::getInstance()->doFile(script);
	}
#else
	// server/both: initialize scene
	reloadLuaScripts();

	//m_pMap->setupScripting();
#endif

	TRACE_INFO("World initialized.", 0);

	return true;
}

#ifdef CLIENT_SIDE
/**
 * Initializes sfx and gfx.
 *
 * @param confings Base setting for the client.
 */
bool EngineCore::initAudioVisuals(const Configs& confings)
{
	m_configs = confings;

	// initialize devIL
	devilInit(true);

	// sounds
	SoundSource::audioInit();
	SoundSourcePtr backgroundNoise = SoundSourcePtr(new SoundSource(getDataDir() + "Desert.wav"));

	backgroundNoise->set(vec3(0, -20, 0), vec3(0.0f), true);
	backgroundNoise->update();
	backgroundNoise->play();

	m_soundDirectory["backgroundNoise"] = backgroundNoise;

	SoundSource::setListener(vec3(0.0f), vec3(0.0f), vec3(0, 0, -1), vec3(0, 1, 0));

	// shaders
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		TRACE_ERROR("Error: Error initializing glew: ", 0);
		TRACE_ERROR("Error: " << glewGetErrorString(err), 0);
		exit(EXIT_FAILURE);
	}

	if (!setupShaders())
	{
		return false;
	}

	// blurtexture
	utils::gfx::blankTexture(&m_blurTexture, 512, 3, GL_RGB);

	m_pRenderContext = new graphics::RenderContext();
	m_pRenderContext->setEnableBit("shadow", m_configs.m_shadowsOn);

	m_pCamera->setPerspective(45, m_configs.width, m_configs.height, 1.0f, 400.0f);

	return true;
}
#endif

void exitGame(int status)
{
	exit(status);
}

long getElapsedTime()
{
#ifdef CLIENT_SIDE
	return glutGet(GLUT_ELAPSED_TIME);
#else
	return glutGet(GLUT_ELAPSED_TIME);
#endif
}

EntityPtr getEntityFunc(const std::string& name)
{
	return EngineCore::getInstance()->getEntity(name);
}

#ifdef CLIENT_SIDE
models::Mesh* getMeshFunc(const std::string& name)
{
	return EngineCore::getInstance()->getMesh(name);
}

graphics::ShadedMeshPtr getShadedMeshFunc(const std::string& name)
{
	return EngineCore::getInstance()->getShadedMesh(name);
}

MeshDirectory& getMeshDirectoryFunc()
{
	return EngineCore::getInstance()->getMeshDirectory();
}
#endif

void EngineCore::resetLuaScripts()
{
	using namespace luabind;

	LuaManager::getInstance()->init();

	lua_State* luaManagerState = LuaManager::getInstance()->getState();

	module(luaManagerState) [

	    // register directories
#ifdef CLIENT_SIDE
	    class_<TextureDirectory>("TextureDirectory"),
	    class_<EffectDirectory>("EffectDirectory"),
	    class_<MeshDirectory>("MeshDirectory"),
	    class_<ShadedMeshDirectory>("ShadedMeshDirectory"),
#endif
	    // register global functions

	    def("getIndentNum", &getIndentNum),
	    def("updateIndentNum", &updateIndentNum),
	    def("logToConsole", &logToConsole),

	    def("getEntity", &getEntityFunc),

#ifdef CLIENT_SIDE
	    def("getLightSource", &getLightSource),
	    def("getMesh", &getMeshFunc),
	    def("getShadedMesh", &getShadedMeshFunc),
	    def("getTexture", &graphics::getTexture),
#endif
	    def("getElapsedTime", &getElapsedTime),

	    def("exitGame", &exitGame)

	    //def("getMapElement", (Mesh* (EngineCore::*)(const std::string&, const std::string&)) &EngineCore::getMapElement)

	];


#ifdef CLIENT_SIDE
	// Mesh
	module(luaManagerState) [
	    class_<models::Mesh>("Mesh")
	];

	models::Model3ds::registerMethodsToLua();
	models::ModelMd5::registerMethodsToLua();

	// Shader
	module(luaManagerState) [
	    class_<graphics::Shader>("Shader")
	    .def(constructor<>())
	    .def(constructor<const char*>())
	];

	graphics::ShadedMesh::registerMethodsToLua();
#endif

	ConstantManager::registerMethodsToLua();

	Node::registerMethodsToLua();
	NodeGroup::registerMethodsToLua();
	Entity::registerMethodsToLua();
	Player::registerMethodsToLua();
#ifdef CLIENT_SIDE
	LightSource::registerMethodsToLua();
	effects::SoundEmitter::registerMethodsToLua();
#endif
}

void EngineCore::reloadLuaScripts()
{
	for (const std::string& scriptFile : m_luaInitializerScripts)
	{
		LuaManager::getInstance()->doFile(scriptFile);
	}
}

#ifdef CLIENT_SIDE
/**
 * Reloads the map or model textures with new size. The new size is <imageSize> x <textureResolutionDiv>.
 *
 * @param textureResolutionDiv	The texture division factor.
 * @param levelTextures			Reload the level or the model textures.
 */
void EngineCore::reloadTextures(const float textureResolutionDiv, const bool levelTextures)
{
	if (levelTextures)
	{
		if (m_currentMapTextureResolutionDiv == textureResolutionDiv)
		{
			return;
		}

		m_currentMapTextureResolutionDiv = textureResolutionDiv;

		for (auto entry : graphics::getTextureDirectory())
		{
			if (entry.first.find("/bsp/") != std::string::npos)
			{
				glDeleteTextures(1, &entry.second);
				loadTexture(entry.first, entry.second, graphics::getTextureDirectory(), true, textureResolutionDiv);
			}
		}
	}
	else
	{
		if (m_currentTextureResolutionDiv == textureResolutionDiv)
		{
			return;
		}

		m_currentTextureResolutionDiv = textureResolutionDiv;

		for(auto entry : graphics::getTextureDirectory())
		{
			if (entry.first.find("/bsp/") == std::string::npos)
			{
				glDeleteTextures(1, &entry.second);
				loadTexture(entry.first, entry.second, graphics::getTextureDirectory(), true, textureResolutionDiv);
			}
		}
	}

	TRACE_INFO("Textures reloaded.", 0);
}
#endif

void EngineCore::deleteNode(const std::string& nodeName)
{
	m_deletedNodes.insert(nodeName);
}


// getters-setters

NodeGroup* EngineCore::getRootNode() const
{
	return m_pRootNode;
}

void EngineCore::setRootNode(NodeGroup* pRootNode)
{
	SAFEDEL(m_pRootNode);
	m_pRootNode = pRootNode;
}

void EngineCore::setRootNode(UnitDirectory& unitDirectory, ItemDirectory& itemDirectory, LightSourceDirectory& lightSourceDirectory)
{
	//pRootNode->addSP(NodePtr e);
}

Camera* EngineCore::getCamera() const
{
	return m_pCamera;
}

Player* EngineCore::getPlayer() const
{
	return m_pPlayer;
}

void EngineCore::setPlayer(Player* pPlayer)
{
	m_pPlayer = pPlayer;
}

#ifdef CLIENT_SIDE
NodeGroup* EngineCore::getEffectsNode() const
{
	return m_pShadersNode;
}

graphics::RenderContext* EngineCore::getRenderContext()
{
	return m_pRenderContext;
}
#endif

const Configs& EngineCore::getConfigs() const
{
	return m_configs;
}
