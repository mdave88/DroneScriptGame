#pragma once

#include "Common/Configs.h"
#include "Common/luamanager/LuaManager.h"

#include <RapidXml/rapidxml.hpp>


#define	NUM_FBOS 4

class Camera;
class NodeGroup;

class Mesh;

namespace graphics
{
class RenderContext;
}


class EngineCore : public Singleton<EngineCore>
{
public:
	EngineCore();

	bool		initLogic();
	bool		initAudioVisuals(const Configs& confings);

	void		release();

	void		animate(float dt);
	void		render(const int debugLevel = 0);

	void		resetLuaScripts();
	void		reloadLuaScripts();

	void		deleteNode(const std::string& nodeName);

	// getters-setters
	NodeGroup*	getRootNode() const;
	void		setRootNode(NodeGroup* pRootNode);
	void		setRootNode(UnitDirectory& unitDirectory, ItemDirectory& itemDirectory, LightSourceDirectory& lightSourceDirectory);

	Player*		getPlayer() const;
	void		setPlayer(Player* pPlayer);

	const Configs& getConfigs() const;

	NodePtr getNode(const std::string& name) const { return m_nodeDirectory.at(name); }
	NodePtr getNodeById(const uint16_t id) const { return m_nodeIdDirectory.at(id); }
	NodeDirectory& getNodeDirectory() { return m_nodeDirectory; }
	NodeIdDirectory& getNodeIdDirectory() { return m_nodeIdDirectory; }

	EntityPtr getEntity(const std::string& name) const { return m_entityDirectory.at(name); }
	EntityDirectory& getEntityDirectory() { return m_entityDirectory; }

#ifdef CLIENT_SIDE
	void		onScreenResize(const int width, const int height);

	void		reloadTextures(const float textureResolutionDiv, const bool levelTextures);

	Camera*		getCamera() const;

	NodeGroup*	getEffectsNode() const;

	graphics::RenderContext* getRenderContext();

	models::Mesh* getMesh(const std::string& name) const { return m_meshDirectory.at(name); }
	MeshDirectory& getMeshDirectory() { return m_meshDirectory; }

	graphics::ShadedMeshPtr getShadedMesh(const std::string& name) const { return m_shadedMeshDirectory.at(name); };
	ShadedMeshDirectory& getShadedMeshDirectory() { return m_shadedMeshDirectory; };
#endif

private:
	void		parseXml(const std::string& filename, const bool justData = false);
	void		parseLuaSettings(const rapidxml::xml_node<>* rootXmlNode, bool justDefinitions = false);

#ifdef CLIENT_SIDE
	// render
	void		renderScene(const GLuint fboTarget = 0, int debugLevel = 0);
	void		renderFPS();
	//void		renderMotionBlur(GLuint textureID);

	// post processing
	void		renderQuad(const GLuint program, const GLuint fboTexId, float strength = 1.0f);
	void		renderShadowMaps(const GLuint fboTarget = 0);

	void		renderDOF(const GLuint fboTarget = 0, const int debugLevel = 0);
	void		renderBloom(const GLuint fboTarget = 0, const int debugLevel = 0);
	void		renderGrayScale(const GLuint fboTarget = 0, const int debugLevel = 0);

	// setup
	bool		setupShaders();
	void		setupFBOs();
	void		setupPostProcessing();


	// load entities and gfx
	void		loadMeshes(const rapidxml::xml_node<>* rootXmlNode, const bool justData = false);
	void		loadShadedMeshes(const rapidxml::xml_node<>* rootXmlNode);
	//void		loadLevel(const rapidxml::xml_node<>* rootXmlNode, const bool justData = false);
#endif

private:
	// game objects
	Player*						m_pPlayer;
	Camera*						m_pCamera;
	NodeGroup*					m_pRootNode;
	NodeGroup*					m_pShadersNode;
	std::set<std::string>		m_deletedNodes;

	// fps
	float						m_fps;
	uint32_t					m_frame, m_elapsedTime, m_timeBase;

#ifdef CLIENT_SIDE
	// render context
	graphics::RenderContext*	m_pRenderContext;

	// global textures
	GLuint						m_blurTexture;
	GLuint						m_skyboxCubeMap;
	float						m_currentTextureResolutionDiv;
	float						m_currentMapTextureResolutionDiv;

	// post processing
	GLuint						m_fullScreenQuadVbo;
	GLuint						m_fbos[NUM_FBOS];
	GLuint						m_fboColorTextures[NUM_FBOS];
	GLuint						m_fboDepthTextures[NUM_FBOS];
#endif

	Configs						m_configs;

	// lua scripts
	std::vector<std::string>	m_luaDefinitonScripts;
	std::vector<std::string>	m_luaInitializerScripts;

	NodeDirectory				m_nodeDirectory;
	NodeIdDirectory				m_nodeIdDirectory;
	EntityDirectory				m_entityDirectory;

#ifdef CLIENT_SIDE
	MeshDirectory				m_meshDirectory;
	ShadedMeshDirectory			m_shadedMeshDirectory;
	SoundDirectory				m_soundDirectory;
#endif
};
