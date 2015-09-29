#pragma once

#include "Common/ClientConfigs.h"

#ifdef CLIENT_SIDE
#define	NUM_FBOS 4

class Camera;

namespace graphics
{
	class RenderContext;
}
#endif

class EngineCore : public Singleton<EngineCore>
{
public:
	EngineCore();

	bool initLogic();
	bool initAudioVisuals(const ClientConfigs& confings);
		 
	void release();
		 
	void animate(float dt);
	void render(const int debugLevel = 0);
		 
	void resetLuaScripts();
	void reloadLuaScripts();

	// getters-setters
	const ClientConfigs& getConfigs() const;

#ifdef CLIENT_SIDE
	void onScreenResize(const int width, const int height);
	void reloadTextures(const float textureResolutionDiv, const bool levelTextures);

	Camera* getCamera() const;
	graphics::RenderContext* getRenderContext();

	models::Mesh* getMesh(const std::string& name) const { return m_meshDirectory.at(name); }
	MeshDirectory& getMeshDirectory() { return m_meshDirectory; }
	graphics::ShadedMeshPtr getShadedMesh(const std::string& name) const { return m_shadedMeshDirectory.at(name); };
	ShadedMeshDirectory& getShadedMeshDirectory() { return m_shadedMeshDirectory; };

	graphics::ShaderPtr getShader(const std::string& name) const { return m_shaderDirectory.at(name); };
#endif

private:
#ifdef CLIENT_SIDE
	// setup
	bool setupShaders();
	void setupFBOs();
	void setupPostProcessing();

	// render
	void renderScene(const GLuint fboTarget = 0, int debugLevel = 0);
	void renderFPS();
	//void renderMotionBlur(GLuint textureID);

	// post processing
	void renderQuad(const GLuint program, const GLuint fboTexId, float strength = 1.0f);
	void renderShadowMaps(const GLuint fboTarget = 0);
	
	void renderDOF(const GLuint fboTarget = 0, const int debugLevel = 0);
	void renderBloom(const GLuint fboTarget = 0, const int debugLevel = 0);
	void renderGrayScale(const GLuint fboTarget = 0, const int debugLevel = 0);
#endif

private:
	ClientConfigs						m_configs;
	
	// lua scripts
	std::vector<std::string>	m_luaDefinitonScripts;
	std::vector<std::string>	m_luaInitializerScripts;

#ifdef CLIENT_SIDE
	float						m_fps;
	uint32_t					m_frame, m_elapsedTime, m_timeBase;

	Camera*						m_pCamera;
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


	TextureDirectory			m_textureDirectory;
	ShaderDirectory				m_shaderDirectory;
	MeshDirectory				m_meshDirectory;
	ShadedMeshDirectory			m_shadedMeshDirectory;
	SoundDirectory				m_soundDirectory;
#endif
};
