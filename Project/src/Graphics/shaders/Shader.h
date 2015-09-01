
#ifndef SHADER_H
#define SHADER_H

class Camera;

namespace graphics
{

class Shader
{
public:
	Shader(const std::string& shaderName = "", const uint glslVersion = 0);
	Shader(const std::string& vertexFile, const std::string& fragmentFile, const uint glslVersion = 0);

	void load(const std::string& shaderName, const uint glslVersion = 0);
	void load(const std::string& vertexFile, const std::string& fragmentFile, const uint glslVersion = 0);

	void setupCommonUniforms();
	void setupUniforms(const std::set<std::string>& uniformNames);
	void updateCommonUniforms(const Camera* camera, const Matrix& modelMatrix) const;

	void bind();
	void enable() const;

	static void disableAll();
	static void disableActive();

	void beginPass(const uint pass) const;


	// getters-setters
	void setUniform1i(const std::string& uniformName, GLuint value) const;
	void setUniform1f(const std::string& uniformName, GLfloat value) const;
	void setUniform3f(const std::string& uniformName, const vec3& values) const;
	void setUniform4f(const std::string& uniformName, const vec3& values, const float w) const;
	void setUniform1iv(const std::string& uniformName, GLsizei count, const GLint* values) const;
	void setUniformMatrix(const std::string& uniformName, GLfloat* mat) const;

	void setTexture(const char* textureName, const GLuint textureSlot, const GLuint value) const;

	uint getNumPasses() const;
	uint getCurrentPass() const;

	GLuint getProgram() const;

	static void bindShader(Shader* shader);
	static Shader* getActiveShader();

public:
	static Shader*					ms_pActiveShader;
	static int						ms_maxShaderQuality;

	GLuint							m_shaderProg;

	uint							m_numPasses;
	uint							m_currentPass;

	std::map<std::string, GLint>	m_uniforms;
};

typedef std::shared_ptr<graphics::Shader>	ShaderPtr;

ShaderPtr			getEffect(const std::string& name);
EffectDirectory&	getEffectDirectory();

} // namespace graphics

#endif // SHADER_H
