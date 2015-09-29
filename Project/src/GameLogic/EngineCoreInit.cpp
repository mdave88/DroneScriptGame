#include "GameStdAfx.h"
#include "GameLogic/EngineCore.h"
#include "Common/LuaManager.h"

#ifdef CLIENT_SIDE
#include "Models/3ds/Model3ds.h"
#include "Models/md5/ModelMd5.h"

#include "Graphics/Camera.h"
#include "Graphics/ShadedMesh.h"
#include "Graphics/RenderContext.h"
#include "Sound/SoundSource.h"
#endif

EngineCore::EngineCore()
#ifdef CLIENT_SIDE
	: m_elapsedTime(0)
	, m_timeBase(0)
	, m_pRenderContext(nullptr)

	, m_currentTextureResolutionDiv(2.0f)
	, m_currentMapTextureResolutionDiv(2.0f)

	, m_fps(0)
	, m_frame(0)
#endif
{
#ifdef CLIENT_SIDE
	m_pCamera = new Camera();
#endif
}

void EngineCore::release()
{
	LuaManager::getInstance()->close();

#ifdef CLIENT_SIDE
	delete m_pCamera;
	delete m_pRenderContext;

	for (auto& entry : m_meshDirectory)
	{
		SAFEDEL(entry.second);
	}

	for (auto& entry : m_textureDirectory)
	{
		glDeleteTextures(1, &entry.second);
	}

	m_soundDirectory["backgroundNoise"]->release();
	SoundSource::audioExit();
#endif
}

/**
 * Initializes the game logic: loads the game initializer lua scripts.
 */
bool EngineCore::initLogic()
{
	new LuaManager();

	m_configs = m_configs;

	TRACE_INFO("Initializing logic.", 0);

	reloadLuaScripts();

	TRACE_INFO("World initialized.", 0);

	return true;
}

#ifdef CLIENT_SIDE
/**
 * Initializes sfx and gfx.
 *
 * @param confings Base setting for the client.
 */
bool EngineCore::initAudioVisuals(const ClientConfigs& confings)
{
	m_configs = confings;

	// initialize devIL
	devilInit(true);

	// sounds
	SoundSource::audioInit();
	SoundSourcePtr backgroundNoise = SoundSourcePtr(new SoundSource(CONST_STR("dataDir") + "/Desert.wav"));

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
	    def("getIndentNum", &getIndentNum),
	    def("updateIndentNum", &updateIndentNum),
	    def("logToConsole", &logToConsole),

	    def("getElapsedTime", &getElapsedTime),

	    def("exitGame", &exitGame)
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
#endif

	ConstantManager::registerMethodsToLua();
}

void EngineCore::reloadLuaScripts()
{
	for (const std::string& scriptFile : m_luaInitializerScripts)
	{
		LuaManager::getInstance()->doFile(scriptFile);
	}
}

// getters-setters
#ifdef CLIENT_SIDE
Camera* EngineCore::getCamera() const
{
	return m_pCamera;
}

graphics::RenderContext* EngineCore::getRenderContext()
{
	return m_pRenderContext;
}
#endif

const ClientConfigs& EngineCore::getConfigs() const
{
	return m_configs;
}
