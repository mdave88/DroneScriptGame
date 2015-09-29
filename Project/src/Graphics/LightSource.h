#pragma once

namespace graphics
{
	class RenderContext;
}

/**
 * @brief Base class of the light sources.
 *
 * Main properties: ambient, diffuse, specular colors and attenuation settings.
 */
class LightSource
{
public:
	enum LightType
	{
		LIGHTTYPE_DIRECTIONAL = 0,
		LIGHTTYPE_POINT,
		LIGHTTYPE_SPOT
	};

public:
	LightSource(GLuint id = 0, LightType type = LIGHTTYPE_POINT);
	LightSource(GLuint id, const vec3& ambient, const vec3& diffuse, const vec3& specular, const vec3& pos, LightType type = LIGHTTYPE_POINT);
	virtual ~LightSource();

	void reset();

	static void animateLights();
	static void updateLightUniforms(const graphics::Shader* shader);

	void updateUniforms(const graphics::Shader* shader);


	virtual void render(const graphics::RenderContext& context);
	virtual void animate(const float dt);


	// getters-setters
	LightType getType() const;
	void setType(LightType type);

	void set(const vec3& ambient, const vec3& diffuse, const vec3& specular, const vec3& pos);
	void setBase(const vec3& ambient, const vec3& diffuse, const vec3& specular, const vec3& pos);
	void setAttenuation(const vec3& attenuation);

	void setTarget(const vec3& target);

	float getShadowMapRatio() const;
	void setShadowMapRatio(float ratio);

	virtual void setVisibility(bool isVisible);


	// static methods
	static void disableAll();
	static void setLightModel(const vec3& val, float a);

	// shadowmap
	static void generateShadowMapArray(int width, int height);
	void generateShadowFBO(int width, int height);

	void shadowingPass1(graphics::RenderContext& context);
	void shadowingPass2(graphics::RenderContext& context);


	//// register to lua
	//static void registerMethodsToLua();

protected:
	std::string					m_uniformName;

public:
	static GLint				ms_numMaxLights;
	static std::vector<GLint>	ms_enabledLights;

	static GLuint				ms_shadowMapArrayTexId;
	static int					ms_shadowMapWidth;
	static int					ms_shadowMapHeight;

	static float				ms_shadowMapRatio;
	static GLfloat				ms_shadowMapPixelOffsetX;
	static GLfloat				ms_shadowMapPixelOffsetY;

	LightType		m_type;

	GLuint			m_id;

	vec3			m_pos;
	vec3			m_rot;

	vec3			m_target;

	vec3			m_ambient;
	vec3			m_diffuse;
	vec3			m_specular;

	vec3			m_ambientBase;
	vec3			m_diffuseBase;
	vec3			m_specularBase;

	GLfloat			m_constantAttenuation;
	GLfloat			m_linearAttenuation;
	GLfloat			m_quadricAttenuation;

	// shadowMap

	GLuint			m_shadowFboId;
	Matrix			m_shadowMatrix;
};

//BOOST_CLASS_EXPORT_KEY(LightSource);
