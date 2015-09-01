#include "GameStdAfx.h"
#include "Material.h"

#include "GameLogic/LightSource.h"


namespace graphics
{

// not needed
Material::Material(Shader* pEffect) : m_pShader(pEffect)
{
}

Material& Material::operator=(const Material& other)
{
	if (this != &other)
	{
		m_pShader = other.m_pShader;
		m_textureEffectVariables = other.m_textureEffectVariables;
		m_cubeTextureEffectVariables = other.m_cubeTextureEffectVariables;
		m_intEffectVariables = other.m_intEffectVariables;
		m_floatEffectVariables = other.m_floatEffectVariables;
		m_vectorEffectVariables = other.m_vectorEffectVariables;
	}

	return *this;
}

void Material::setTexture(const std::string& textureName, GLuint texture)
{
	m_textureEffectVariables[textureName] = texture;
}

void Material::setCubeTexture(const std::string& cubeTextureName, GLuint cubeTexture)
{
	m_cubeTextureEffectVariables[cubeTextureName] = cubeTexture;
}

void Material::setVector(const std::string& vectorName, const vec3& value)
{
	m_vectorEffectVariables[vectorName] = value;
}

void Material::setIntVariable(const std::string& variableName, const int value)
{
	m_intEffectVariables[variableName] = value;
}

void Material::setFloatVariable(const std::string& variableName, const float value)
{
	m_floatEffectVariables[variableName] = value;
}

Shader* Material::apply(Shader* pCustomEffect)
{
	if (!pCustomEffect)
	{
		pCustomEffect = m_pShader;
	}

	pCustomEffect->bind();
	//pCustomEffect->setTexture("tex", 0, 0);

	//textures
	pCustomEffect->setTexture("s_colorTex", 0, 0);
	pCustomEffect->setTexture("s_normalHeightTex", 0, 0);
	pCustomEffect->setTexture("s_lightMapTex", 0, 0);
	pCustomEffect->setTexture("s_cubeTex", 0, 0);


	// set all textures
	int idx = 0;
	for (const auto& entry : m_textureEffectVariables)
	{
		// the default textures are: tex, normalHeightMap
		pCustomEffect->setTexture(entry.first.c_str(), 3 + idx, entry.second);
		idx++;
	}

	// set all cube textures
	for (const auto& entry : m_cubeTextureEffectVariables)
	{
		pCustomEffect->setUniform1i(entry.first.c_str(), entry.second);
	}

	// set all vectors
	for (const auto& entry : m_vectorEffectVariables)
	{
		pCustomEffect->setUniform3f(entry.first.c_str(), entry.second);
	}

	// set all float params
	for (const auto& entry : m_intEffectVariables)
	{
		pCustomEffect->setUniform1i(entry.first.c_str(), entry.second);
	}

	// set all int params
	for (const auto& entry : m_floatEffectVariables)
	{
		pCustomEffect->setUniform1f(entry.first.c_str(), entry.second);
	}


	// enable lights if enabled in the opengl context
	pCustomEffect->setUniform1iv("enabledLights", (GLsizei) LightSource::ms_enabledLights.size(), (const GLint*) LightSource::ms_enabledLights.data());

	//LightSource::updateAll?

	return pCustomEffect;
}

void Material::resetTextures(Shader* pCustomEffect)
{
	if (!pCustomEffect)
	{
		pCustomEffect = m_pShader;
	}

	pCustomEffect->bind();
	pCustomEffect->setTexture("tex", 0, 0);

	// set all textures
	int idx = 0;
	for (const auto& entry : m_textureEffectVariables)
	{
		// the default textures are: tex, normalHeightMap
		pCustomEffect->setTexture(entry.first.c_str(), 3 + idx, 0);
		idx++;
	}
}

// register to lua
void Material::registerMethodsToLua()
{
	using namespace luabind;

	class_<Material> thisClass("Material");
	thisClass.def(constructor<>());
	thisClass.def(constructor<Shader*>());

	REG_FUNC("setTexture", &Material::setTexture);
	REG_FUNC("setCubeTexture", &Material::setCubeTexture);

	REG_FUNC("setFloatVariable", &Material::setFloatVariable);
	REG_FUNC("setIntVariable", &Material::setIntVariable);
	REG_FUNC("setVector", &Material::setVector);

	module(LuaManager::getInstance()->getState()) [ thisClass ];
}

} // namespace graphics
