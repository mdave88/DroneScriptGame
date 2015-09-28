#include "GameStdAfx.h"
#include "Graphics/shaders/Shader.h"
#include "Graphics/Camera.h"

namespace graphics
{

EffectDirectory effectDirectory;

// static fields
Shader* Shader::ms_pActiveShader = nullptr;
int		Shader::ms_maxShaderQuality = 10;

void Shader::bindShader(Shader* shader)
{
	shader->bind();
}

Shader* Shader::getActiveShader()
{
	return ms_pActiveShader;
}



Shader::Shader(const std::string& shaderName, const uint glslVersion)
{
	if (shaderName == "")
	{
		return;
	}

	load(shaderName, glslVersion);
}

Shader::Shader(const std::string& vertexFile, const std::string& fragmentFile, const uint glslVersion)
{
	load(vertexFile, fragmentFile, glslVersion);
}

void Shader::bind()
{
	ms_pActiveShader = this;
	glUseProgram(m_shaderProg);
}

void Shader::enable() const
{
	glUseProgram(m_shaderProg);
}

/**
* Disables the active shader.
*/
void Shader::disableActive()
{
	ms_pActiveShader = nullptr;
	glUseProgram(0);
}

/**
 * Disables the using of shaders.
 */
void Shader::disableAll()
{
	glUseProgram(0);
}

void Shader::beginPass(const uint pass) const
{
	setUniform1i("pass", pass);
}

void Shader::setupUniforms(const std::set<std::string>& uniformNames)
{
	for (const std::string& uniformName : uniformNames)
	{
		m_uniforms[uniformName] = glGetUniformLocation(m_shaderProg, uniformName.c_str());
	}
}

void Shader::setupCommonUniforms()
{
	std::set<std::string> commonUniformNames
	{
		"u_eyePositionW",
		"u_elapsedTime",
		"u_techniqueMax",
		"u_M_Mat",
		"u_MV_Mat",
		"u_MVP_Mat",
		"u_N_Mat",
		"s_colorTex",
		"s_normalHeightTex",
		"s_cubeTex",
		"s_lightMapTex",
		"s_tex1",
		"s_tex2",
		"s_tex3",
		"u_technique",
		"u_scale",
		"u_bias",
		"enabledLights",
		"u_enableShadows",
		"u_numLightSources",
		"u_shadowMapPixelOffsetX",
		"u_shadowMapPixelOffsetY",
		"u_shadowMapArray",
	};

	char tmp[256];
	const char* lightAttributes[7] = { "type", "pos", "ambient", "diffuse", "specular", "attenuation", "shadowMatrix" };
	for (uint lightId = 0; lightId < 6; lightId++)
	{
		for (uint attribId = 0; attribId < 7; attribId++)
		{
			sprintf(tmp, "u_lights[%d].%s", lightId, lightAttributes[attribId]);
			commonUniformNames.insert(tmp);
		}
	}

	setupUniforms(commonUniformNames);

	// common uniform parameters
	setUniform1i("s_colorTex", 0);
	setUniform1i("s_normalHeightTex", 1);
	setUniform1i("s_cubeTex", 2);

	//additional textures
	setUniform1i("s_tex1", 3);
	setUniform1i("s_tex2", 4);
	setUniform1i("s_tex3", 5);


	setUniform1i("u_technique", 0);
	setUniform1f("u_scale", 0.04f);
	setUniform1f("u_bias", -0.02f);
}

void Shader::updateCommonUniforms(const Camera* camera, const Matrix& modelMatrix) const
{
	//// update fog attributes
	//float	m_fogStart = 20.0f, m_fogLength = 20.0f;
	//vec3	m_fogColor(0.5f);
	//if ((uniformLoc = glGetUniformLocation(m_shaderProg, "u_fogStart")) != -1)
	//	glUniform1f(uniformLoc, m_fogStart);
	//if ((uniformLoc = glGetUniformLocation(m_shaderProg, "u_fogLength")) != -1)
	//	glUniform1f(uniformLoc, m_fogLength);
	//if ((uniformLoc = glGetUniformLocation(m_shaderProg, "u_fogColor")) != -1)
	//	glUniform3f(uniformLoc, m_fogColor.x, m_fogColor.y, m_fogColor.z);

	setUniform3f("u_eyePositionW", camera->getPos());

	setUniform1i("u_elapsedTime", glutGet(GLUT_ELAPSED_TIME));
	setUniform1i("u_techniqueMax", Shader::ms_maxShaderQuality);

	setUniformMatrix("u_M_Mat", modelMatrix.getArray());

	// ModelView
	Matrix	modelViewMatrix;
	Matrix modelViewProjectionMatrix;
	if (m_uniforms.at("u_MV_Mat") != -1)
	{
		modelViewMatrix = camera->getViewMatrix() * modelMatrix;
		setUniformMatrix("u_MV_Mat", modelViewMatrix.getArray());

		modelViewProjectionMatrix = camera->getProjMatrix() * modelViewMatrix;
	}
	else
	{
		modelViewProjectionMatrix = camera->getProjMatrix() * camera->getViewMatrix() * modelMatrix;
	}
	setUniformMatrix("u_MVP_Mat", modelViewProjectionMatrix.getArray());

	// Normal
	Matrix normalMatrix = modelMatrix;
	normalMatrix.setTranslation(vec3(0.0f));

	setUniformMatrix("u_N_Mat", normalMatrix.getArray());
}


// getters-setters
void Shader::setTexture(const char* textureName, const GLuint textureSlot, const GLuint value) const
{
	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_2D, value);
	glUniform1i(m_uniforms.at(textureName), textureSlot);
}

void Shader::setUniform1i(const std::string& uniformName, GLuint value) const
{
	glUniform1i(m_uniforms.at(uniformName), value);
}

void Shader::setUniform1iv(const std::string& uniformName, GLsizei count, const GLint* values) const
{
	glUniform1iv(m_uniforms.at(uniformName), count, values);
}

void Shader::setUniform1f(const std::string& uniformName, GLfloat value) const
{
	glUniform1f(m_uniforms.at(uniformName), value);
}

void Shader::setUniform3f(const std::string& uniformName, const vec3& values) const
{
	glUniform3f(m_uniforms.at(uniformName), values.x, values.y, values.z);
}

void Shader::setUniform4f(const std::string& uniformName, const vec3& values, const float w) const
{
	glUniform4f(m_uniforms.at(uniformName), values.x, values.y, values.z, w);
}

void Shader::setUniformMatrix(const std::string& uniformName, GLfloat* mat) const
{
	glUniformMatrix4fv(m_uniforms.at(uniformName), 1, false, mat);
}


uint Shader::getNumPasses() const
{
	return m_numPasses;
}

uint Shader::getCurrentPass() const
{
	return m_currentPass;
}

GLuint Shader::getProgram() const
{
	return m_shaderProg;
}

GLuint loadShaderFile(const GLenum type, const char* filename, const uint glslVersion)
{
	GLuint shader;
	GLint compiled;

	// create the shader object
	shader = glCreateShader(type);

	FILE* commonShaderFile;

	std::string shaderSource;

	// load the common definitions from Common
	if (type == GL_VERTEX_SHADER)
	{
		char vertexShaderFileName[256];
		if (glslVersion == 0)
		{
			sprintf_s(vertexShaderFileName, "%s/shaders/Common_vOld.vert", getResourcesDir().c_str());
		}
		else
		{
			sprintf_s(vertexShaderFileName, "%s/shaders/Common_v%d.vert", getResourcesDir().c_str(), glslVersion);
		}

		if (fopen_s(&commonShaderFile, vertexShaderFileName, "rb") != 0)
		{
			TRACE_ERROR("Error: Error loading common vertex shader definitions.", 0);
			return 0;
		}
	}
	else
	{
		char fragmentShaderFileName[256];
		if (glslVersion == 0)
		{
			sprintf_s(fragmentShaderFileName, "%s/shaders/Common_vOld.frag", getResourcesDir().c_str());
		}
		else
		{
			sprintf_s(fragmentShaderFileName, "%s/shaders/Common_v%d.frag", getResourcesDir().c_str(), glslVersion);
		}

		if (fopen_s(&commonShaderFile, fragmentShaderFileName, "rb") != 0)
		{
			TRACE_ERROR("Error: Error loading common fragment shader definitions.", 0);
			return 0;
		}
	}

	fseek(commonShaderFile, 0, SEEK_END);
	long size = ftell(commonShaderFile);
	fseek(commonShaderFile, 0, SEEK_SET);

	char* shaderSrc = new char[size + 1];
	fread(shaderSrc, sizeof(char), size, commonShaderFile);
	fclose(commonShaderFile);

	shaderSrc[size] = 0;
	shaderSource = shaderSrc;

	delete[] shaderSrc;


	// load the file
	FILE* shaderFile;
	if (fopen_s(&shaderFile, filename, "rb" ) != 0)
	{
		return 0;
	}

	fseek(shaderFile, 0, SEEK_END);
	size = ftell(shaderFile);
	fseek(shaderFile, 0, SEEK_SET);

	shaderSrc = new char[size + 1];
	fread(shaderSrc, sizeof(char), size, shaderFile);
	fclose(shaderFile);

	shaderSrc[size] = 0;
	shaderSource += shaderSrc;

	delete[] shaderSrc;

	shaderSrc = (char*) shaderSource.c_str();
	shaderSrc[shaderSource.size()] = 0;

	glShaderSource(shader, 1, (const char**) &shaderSrc, nullptr);

	// compile the shader
	glCompileShader(shader);

	// check the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint infoLen = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
			char* infoLog = new char[infoLen];

			glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
			TRACE_ERROR("Error: Error compiling shader <" << filename << ">:" << std::endl << infoLog, 0);

			delete[] infoLog;
		}

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}


/**
 * Loads the shaders from the vertex ([filename].vert) and fragment ([filename].frag) shader files.
 * @param filename	The name of the shader files without the extension.
 */
void Shader::load(const std::string& shaderName, const uint glslVersion)
{
	TRACE_INFO("Loading shader" << shaderName, 0);

	if (!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader)
	{
		TRACE_ERROR("Error: Cannot find GLEW_ARB_vertex_shader and GLEW_ARB_fragment_shader.", 0);
		TRACE_ERROR("Error: Probably not initialized glew.", 0);
		return;
	}

	//creation of shaders and program
	const GLuint vertexShaderHandle = loadShaderFile(GL_VERTEX_SHADER, (const char*)(shaderName + ".vert").c_str(), glslVersion);
	const GLuint fragmentShaderHandle = loadShaderFile(GL_FRAGMENT_SHADER, (const char*)(shaderName + ".frag").c_str(), glslVersion);


	// create the program object
	m_shaderProg = glCreateProgram();

	glAttachShader(m_shaderProg, vertexShaderHandle);
	glAttachShader(m_shaderProg, fragmentShaderHandle);

	// link the program
	glLinkProgram(m_shaderProg);

	// check the link status
	GLint isLinked;
	glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &isLinked);

	if (!isLinked)
	{
		GLint infoLen = 0;
		glGetProgramiv(m_shaderProg, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
			char* infoLog = new char[sizeof(char) * infoLen];

			glGetProgramInfoLog(m_shaderProg, infoLen, nullptr, infoLog);
			TRACE_ERROR("Error: Error linking program:" << std::endl << infoLog, 0);

			delete[] infoLog;
		}

		glDeleteProgram(m_shaderProg);
		return;
	}

	setupCommonUniforms();
}

void Shader::load(const std::string& vertexFile, const std::string& fragmentFile, const uint glslVersion)
{
	TRACE_INFO("Loading shader from: " << vertexFile << ", " << fragmentFile, 0);

	if (!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader)
	{
		TRACE_ERROR("Error: Cannot find GLEW_ARB_vertex_shader and GLEW_ARB_fragment_shader.", 0);
		TRACE_ERROR("Error: Probably not initialized glew.", 0);
		return;
	}

	//creation of shaders and program
	const GLuint vertexShaderHandle = loadShaderFile(GL_VERTEX_SHADER, (const char*) vertexFile.c_str(), glslVersion);
	const GLuint fragmentShaderHandle = loadShaderFile(GL_FRAGMENT_SHADER, (const char*) fragmentFile.c_str(), glslVersion);


	// create the program object
	m_shaderProg = glCreateProgram();

	glAttachShader(m_shaderProg, vertexShaderHandle);
	glAttachShader(m_shaderProg, fragmentShaderHandle);

	// link the program
	glLinkProgram(m_shaderProg);

	// check the link status
	GLint isLinked;
	glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &isLinked);

	if (!isLinked)
	{
		GLint infoLen = 0;
		glGetProgramiv(m_shaderProg, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
			char* infoLog = new char[sizeof(char) * infoLen];

			glGetProgramInfoLog(m_shaderProg, infoLen, nullptr, infoLog);
			TRACE_ERROR("Error: Error linking program:" << std::endl << infoLog, 0);

			delete[] infoLog;
		}

		glDeleteProgram(m_shaderProg);
		return;
	}

	setupCommonUniforms();
}



EffectDirectory& getEffectDirectory()
{
	return effectDirectory;
}

std::shared_ptr<Shader> getEffect(const std::string& name)
{
	return effectDirectory.at(name);
}

} // namespace graphics
