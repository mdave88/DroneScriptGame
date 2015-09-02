#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

#pragma warning( disable : 4512 ) // 'assignment operator could not be generated' because of const member

#include "Graphics/shaders/Shader.h"


class Camera;


/**
 * @brief Namespace of the classes assigned to graphics.
 */
namespace graphics
{

class RenderContext
{
public:
	RenderContext(Shader* pEffect = nullptr, Camera* pCamera = nullptr, const std::string& roleName = "none", float dt = 0)
		: m_viewPortWidth(0)
		, m_viewPortHeight(0)
		, m_roleName(roleName)
		, m_pShader(pEffect)
		, m_pCamera(pCamera)
		, m_dt(dt)
	{
	}

	void setFboTarget(const GLuint fboTarget)
	{
		m_fboTarget = fboTarget;
	}

	void setViewPort(const int width, const int height)
	{
		m_viewPortWidth = width;
		m_viewPortHeight = height;
	}

	Matrix getModelMatrix() const
	{
		return m_modelMatrix;
	}

	void setModelMatrix(const Matrix& modelMatrix)
	{
		m_modelMatrix = modelMatrix;
	}

	bool getEnableBit(const std::string& name) const
	{
		const auto& it = m_enabledBitsMap.find(name);
		if(it != m_enabledBitsMap.end())
		{
			return it->second;
		}

		return false;
	}

	void setEnableBit(const std::string& name, const bool value)
	{
		m_enabledBitsMap[name] = value;
	}

	float getContextFloatParam(const std::string& name) const
	{
		const auto& it = m_contextFloatParams.find(name);
		if (it != m_contextFloatParams.end())
		{
			return it->second;
		}

		return 0.0f;
	}

	void setContextFloatParam(const std::string& name, const float value)
	{
		m_contextFloatParams[name] = value;
	}

	Matrix getContextMatrixParam(const std::string& name) const
	{
		const auto& it = m_contextMatrixParams.find(name);
		if(it != m_contextMatrixParams.end())
		{
			return it->second;
		}

		return Matrix();
	}

	void setContextMatrixParam(const std::string& name, const Matrix& value)
	{
		m_contextMatrixParams[name] = value;
	}

public:
	Shader*								m_pShader;
	Camera*								m_pCamera;
	Matrix								m_modelMatrix;
	float								m_dt;

	GLuint								m_fboTarget;
	uint								m_viewPortWidth;
	uint								m_viewPortHeight;

	std::string							m_roleName;

	std::map<const std::string, bool>	m_enabledBitsMap;
	std::map<const std::string, float>	m_contextFloatParams;
	std::map<const std::string, Matrix>	m_contextMatrixParams;
};

#pragma warning( default : 4512 )

} // namespace graphics

#endif // RENDERCONTEXT_H