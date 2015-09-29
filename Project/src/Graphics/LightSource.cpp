#include "GameStdAfx.h"
#include "Graphics/LightSource.h"

#include "Graphics/Camera.h"
#include "Graphics/RenderContext.h"
#include "GameLogic/EngineCore.h"

GLint LightSource::ms_numMaxLights = 0;
std::vector<GLint> LightSource::ms_enabledLights = std::vector<GLint>();

GLuint	LightSource::ms_shadowMapArrayTexId = 0;
int		LightSource::ms_shadowMapWidth = 512;
int		LightSource::ms_shadowMapHeight = 512;

float	LightSource::ms_shadowMapRatio = 2;
GLfloat	LightSource::ms_shadowMapPixelOffsetX = 0;
GLfloat	LightSource::ms_shadowMapPixelOffsetY = 0;

typedef std::map<std::string, std::shared_ptr<LightSource>> LightSourceDirectory;
LightSourceDirectory lightSourceDirectory;

Matrix biasMatrix = Matrix(	0.5, 0.0, 0.0, 0.0,
                            0.0, 0.5, 0.0, 0.0,
                            0.0, 0.0, 0.5, 0.0,
                            0.5, 0.5, 0.5, 1.0);


LightSource::LightSource(GLuint id, LightType type)
	: m_id(id)
	, m_type(type)
	, m_shadowFboId(0)
{
	if (m_id < ms_enabledLights.size())
	{
		ms_enabledLights[m_id] = 1;
	}
	else
	{
		ms_enabledLights.push_back(1);
	}
}

LightSource::LightSource(GLuint id, const vec3& ambient, const vec3& diffuse, const vec3& specular, const vec3& pos, LightType type)
	: m_id(id)
	, m_type(type)
	, m_shadowFboId(0)
{
	set(ambient, diffuse, specular, pos);
	setBase(ambient, diffuse, specular, pos);
}

LightSource::~LightSource()
{
	if (m_shadowFboId)
	{
		glDeleteFramebuffers(1, &m_shadowFboId);
		m_shadowFboId = 0;
	}
}

/**
 * Resets the lights parameters to the base ones.
 */
void LightSource::reset()
{
	m_ambient	= m_ambientBase;
	m_diffuse	= m_diffuseBase;
	m_specular	= m_specularBase;
}

void LightSource::updateUniforms(const graphics::Shader* shader)
{
	if (m_type == LIGHTTYPE_SPOT && !m_shadowFboId)
	{
		const ClientConfigs& configs = EngineCore::getInstance()->getConfigs();
		generateShadowFBO(configs.width, configs.height);
	}

	if (m_id < ms_enabledLights.size())
	{
		ms_enabledLights[m_id] = 1;
	}
	else
	{
		ms_enabledLights.push_back(1);
	}

	std::stringstream ss;
	ss << "u_lights[" << ((m_id == GL_LIGHT0) ? 0 : m_id) << "]";
	m_uniformName = ss.str();

	shader->setUniform1i(std::string(m_uniformName + ".type"), m_type);
	shader->setUniform3f(std::string(m_uniformName + ".pos"), m_pos);
	shader->setUniform4f(std::string(m_uniformName + ".ambient"), m_ambient, 1.0f);
	shader->setUniform4f(std::string(m_uniformName + ".diffuse"), m_diffuse, 1.0f);
	shader->setUniform4f(std::string(m_uniformName + ".specular"), m_specular, 1.0f);

	if (m_type != LIGHTTYPE_DIRECTIONAL)
	{
		shader->setUniform3f(std::string(m_uniformName + ".attenuation"), vec3(m_constantAttenuation, m_linearAttenuation, m_quadricAttenuation));
	}
	shader->setUniformMatrix(std::string(m_uniformName + ".shadowMatrix"), m_shadowMatrix.getArray());
}

/**
 * Updates the parameters of all light sources.
 *
 * @param context	The render context.
 */
void LightSource::updateLightUniforms(const graphics::Shader* shader)
{
	for (const auto& entry : lightSourceDirectory)
	{
		entry.second->updateUniforms(shader);
	}

	shader->setUniform1i("u_numLightSources", (GLuint) LightSource::ms_enabledLights.size());
	shader->setUniform1f("u_shadowMapPixelOffsetX", ms_shadowMapPixelOffsetX);
	shader->setUniform1f("u_shadowMapPixelOffsetY", ms_shadowMapPixelOffsetY);
	shader->setUniform1i("u_shadowMapArray", 6);

	if (ms_shadowMapArrayTexId)
	{
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D_ARRAY, ms_shadowMapArrayTexId);
	}
}

void LightSource::animateLights()
{
	for (const auto& entry : lightSourceDirectory)
	{
		const auto& light = entry.second;
		if (light->getType() == LightSource::LIGHTTYPE_SPOT)
		{
			Camera camera;
			camera.update(light->m_pos, light->m_rot);
			camera.setPerspective(45, ms_shadowMapWidth, ms_shadowMapHeight, 1.0f, 400.0f);

			light->m_shadowMatrix = biasMatrix * camera.getProjMatrix() * camera.getViewMatrix();
		}
	}
}

/**
 *	Disables all the light sources.
 */
void LightSource::disableAll()
{
	for(const auto& entry : lightSourceDirectory)
	{
		entry.second->setVisibility(false);
	}
}

/**
 * Animates the light source.
 *
 * @param dt Delta time.
 */
void LightSource::animate(const float dt)
{
}

// getters-setters

/**
 * Sets the visibility of the light source and updates the array of enabled lights.
 *
 * @param isVisible The visibility of the light source.
 */
void LightSource::setVisibility(bool isVisible)
{
	///m_isVisible = isVisible;

	if (m_id < ms_enabledLights.size())
	{
		ms_enabledLights[m_id] = isVisible;
	}
	else
	{
		ms_enabledLights.push_back(isVisible);
	}
}

/**
 * Returns the type of the light source.
 */
LightSource::LightType LightSource::getType() const
{
	return m_type;
}

/**
 * Sets the type of the light source.
 *
 * @param type The type of the light source.
 */
void LightSource::setType(LightType type)
{
	m_type = type;
}

/**
 * Sets the parameters of the light source.
 *
 * @param ambient	Ambient color.
 * @param diffuse	Diffuse color.
 * @param specular	Specular color.
 * @param pos		Position.
 */
void LightSource::set(const vec3& ambient, const vec3& diffuse, const vec3& specular, const vec3& pos)
{
	m_pos		= pos;
	m_ambient	= ambient;
	m_diffuse	= diffuse;
	m_specular	= specular;
}

/**
 * Sets the base parameters of the light source.
 *
 * @param ambient	Ambient color base.
 * @param diffuse	Diffuse color base.
 * @param specular	Specular color base.
 * @param pos		Position base.
 */
void LightSource::setBase(const vec3& ambient, const vec3& diffuse, const vec3& specular, const vec3& pos)
{
	m_ambientBase = ambient;
	m_diffuseBase = diffuse;
	m_specularBase = specular;

	set(ambient, diffuse, specular, pos);
}

/**
 * Sets the attenuation parameters.
 *
 * @param attenuation	Contains constant, linear, quadric attenuation values.
 */
void LightSource::setAttenuation(const vec3& attenuation)
{
	m_constantAttenuation = attenuation.x;
	m_linearAttenuation = attenuation.y;
	m_quadricAttenuation = attenuation.z;
}

/**
 * Sets the target position of the light source.
 *
 * @param target Target position.
 */
void LightSource::setTarget(const vec3& target)
{
	m_target = target;
}

void LightSource::setLightModel(const vec3& val, float a)
{
	//float f[] = { VEC3_TO_F3(val), a };
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, f);
}

/**
 * Renders a low-poly sphere at the light's position.
 *
 * @param context The render context.
 */
void LightSource::render(const graphics::RenderContext& context)
{
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Matrix projMatrix = context.m_pCamera->getProjMatrix();
	glMultMatrixf(projMatrix.getArray());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Matrix viewMatrix = context.m_pCamera->getViewMatrix();
	glMultMatrixf(viewMatrix.getArray());
	glTranslatef(m_pos.x, m_pos.y, m_pos.z);

	graphics::Shader::disableAll();
	glutSolidSphere(0.5, 4, 4);
}


//// register to lua
//void LightSource::registerMethodsToLua()
//{
//	using namespace luabind;
//
//#ifdef USE_LUABIND_DEBOOSTIFIED
//	class_<LightSource, Entity> thisClass("LightSource");
//#else
//	class_<LightSource, NodePtr, bases<Entity>> thisClass("LightSource");
//#endif
//	thisClass.def(constructor<GLuint>());
//	thisClass.def(constructor<GLuint, vec3, vec3, vec3, vec3>());
//
//	thisClass.enum_("LightType")
//	[
//		value("LIGHTTYPE_DIRECTIONAL", 0),
//		value("LIGHTTYPE_POINT", 1),
//		value("LIGHTTYPE_SPOT", 2)
//	];
//
//	REG_ATTR("type", &LightSource::m_type);
//
//	REG_ATTR("pos", &LightSource::m_pos)
//
//	REG_ATTR("ambient", &LightSource::m_ambient);
//	REG_ATTR("diffuse", &LightSource::m_diffuse);
//	REG_ATTR("specular", &LightSource::m_specular);
//
//	REG_ATTR("ambientBase", &LightSource::m_ambientBase);
//	REG_ATTR("diffuseBase", &LightSource::m_diffuseBase);
//	REG_ATTR("specularBase", &LightSource::m_specularBase);
//
//	REG_ATTR("constantAttenuation", &LightSource::m_constantAttenuation);
//	REG_ATTR("linearAttenuation", &LightSource::m_linearAttenuation);
//	REG_ATTR("quadricAttenuation", &LightSource::m_quadricAttenuation);
//
//	REG_FUNC("reset", &LightSource::reset);
//
//	REG_FUNC("set", &LightSource::set);
//	REG_FUNC("setBase", &LightSource::setBase);
//	REG_FUNC("setAttenuation", &LightSource::setAttenuation);
//
//	thisClass.scope [
//	    def("disableAll", &LightSource::disableAll),
//	    def("setLightModel", &LightSource::setLightModel)
//	];
//
//	module(LuaManager::getInstance()->getState()) [ thisClass ];
//}


//// serialization
//bool LightSource::updateProperties(const Node& otherN, bool serverSide)
//{
//	const LightSource& other = (const LightSource&) otherN;
//
//	Entity::updateProperties(other, serverSide);
//
//	UP_P(m_id);
//	UP_P(m_type);
//
//	UP_P(m_ambient);
//	UP_P(m_diffuse);
//	UP_P(m_specular);
//
//	UP_P(m_constantAttenuation);
//	UP_P(m_linearAttenuation);
//	UP_P(m_quadricAttenuation);
//
//	return m_attribmask;
//}
//
//template <typename Archive>
//void LightSource::serialize(Archive& ar, const uint version)
//{
//	ar& boost::serialization::base_object<Entity>(*this);
//
//	SER_P(m_id);
//	SER_P(m_type);
//
//	SER_P(m_ambient);
//	SER_P(m_diffuse);
//	SER_P(m_specular);
//
//	SER_P(m_constantAttenuation);
//	SER_P(m_linearAttenuation);
//	SER_P(m_quadricAttenuation);
//}

//SERIALIZABLE(LightSource);


// ----------------------------------------------------
// Shadow computation
// ----------------------------------------------------

/**
 * Generates a shadow map array for the light sources.
 *
 * @param
 */
void LightSource::generateShadowMapArray(int width, int height)
{
	// Try to use a texture depth component
	glGenTextures(1, &ms_shadowMapArrayTexId);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ms_shadowMapArrayTexId);

	// GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF. Using GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Remove artifact on the edges of the shadowmap
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// This is to allow usage of shadow2DProj function in the shader
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	int numShadowMaps = 3;

	// No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24_ARB, width, height, numShadowMaps, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	CheckGLError();
}

bool extensionSupported(char* extensionName)
{
	return (strstr((char*)glGetString(GL_EXTENSIONS), extensionName) != nullptr);
}


/**
 * Generates a shadow frame buffer for the light with only depth texture component.
 *
 * @param width		The width of the depth texture multiplied by the shadowmap ratio.
 * @param height	The height of the depth texture multiplied by the shadowmap ratio.
 */
void LightSource::generateShadowFBO(int width, int height)
{
	ms_shadowMapWidth = width * ms_shadowMapRatio;
	ms_shadowMapHeight = height * ms_shadowMapRatio;

	//GLfloat borderColor[4] = {0,0,0,0};

	if (!extensionSupported("GL_EXT_texture_array"))
	{
		TRACE_ERROR("Error: GL_EXT_texture_array not supported!", 0);
	}

	if (!ms_shadowMapArrayTexId)
	{
		generateShadowMapArray(ms_shadowMapWidth, ms_shadowMapHeight);
	}

	// create a framebuffer object
	glGenFramebuffers(1, &m_shadowFboId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFboId);

	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffers(0, nullptr);

	// check FBO status
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

	// switch back to window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * The first pass of the shadow computation: binds and clears the target shadow frame buffer, sets the new viewport and the shadow matrix.
 * @param context	The render context.
 */
void LightSource::shadowingPass1(graphics::RenderContext& context)
{
	//glEnable(GL_TEXTURE_2D_ARRAY);

	// First step: Render from the light POV to a FBO, store depth values only
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFboId);	// rendering off screen
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ms_shadowMapArrayTexId, 0, m_id);

	// In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
	glViewport(0, 0, ms_shadowMapWidth, ms_shadowMapHeight);

	// Clear previous frame values
	glClear(GL_DEPTH_BUFFER_BIT);

	// Disable color rendering, we only want to write to the Z-Buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Culling switching, rendering only backface, this is done to avoid self-shadowing
	glCullFace(GL_FRONT);


	// setup the render context
	EngineCore::getInstance()->getShader("simplest")->bind();
	context.m_pShader = EngineCore::getInstance()->getShader("simplest").get();
	context.m_roleName = "none";
	context.setEnableBit("shadow", false);

	// setup camera
	context.m_pCamera->setPerspective(45, context.m_viewPortWidth, context.m_viewPortHeight, 1.0f, 400.0f);
	context.m_pCamera->update(m_pos, m_rot);
}

// setup the shadow matrix


/**
 * The second pass of the shadow computation: resets the viewport and clears the frame buffer.
 *
 * @param context	The render context.
 */
void LightSource::shadowingPass2(graphics::RenderContext& context)
{
	glViewport(0, 0, context.m_viewPortWidth, context.m_viewPortHeight);

	//Enabling color write (previously disabled for light POV z-buffer rendering)
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glCullFace(GL_BACK);
}


float LightSource::getShadowMapRatio() const
{
	return ms_shadowMapRatio;
}

void LightSource::setShadowMapRatio(float ratio)
{
	ms_shadowMapRatio = ratio;
}
