#pragma once

#include "Graphics/shaders/Shader.h"

#pragma warning( disable : 4512 ) // 'assignment operator could not be generated' because of const member

namespace graphics
{

class Material
{
public:
	Material(Shader* pEffect = nullptr);

	Material& operator=(const Material& s);

	Shader* apply(Shader* pCustomEffect);
	void resetTextures(Shader* pCustomEffect);


	void setTexture(const std::string& textureName, GLuint texture);
	void setCubeTexture(const std::string& cubeTextureName, GLuint cubeTexture);

	void setFloatVariable(const std::string& variableName, const float value);
	void setIntVariable(const std::string& variableName, const int value);
	void setVector(const std::string& vectorName, const vec3& value);

private:
	Shader*					m_pShader;

	TextureDirectory		m_textureEffectVariables;
	CubeTextureDirectory	m_cubeTextureEffectVariables;

	FloatDirectory			m_floatEffectVariables;
	IntDirectory			m_intEffectVariables;
	VectorDirectory			m_vectorEffectVariables;
};

#pragma warning( default : 4512 )

} // namespace graphics
