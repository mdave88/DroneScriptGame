#include "GameStdAfx.h"
#include "Camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::setPerspective(float fovY, int screenWidth, int screenHeight, float nearPlane, float farPlane)
{
	m_projMat.setPerspective(fovY, (float) screenWidth / (float) screenHeight, nearPlane, farPlane);
	//m_viewMat.setTranslation(pos);
}

void Camera::update(vec3 pos, vec3 rot)
{
	m_pos = pos;
	m_rot = rot;

	Matrix rotMat, rotMatT;
	rotMat.setRotation(rot);
	rotMatT = rotMat.transpose();

	Matrix transMat;
	transMat.setTranslation(-pos);

	Matrix m_transformMat;
	m_transformMat.setScale(vec3(1.0f));
	//m_transformMat = m_transformMat * rotMat;
	m_transformMat = m_transformMat * transMat * rotMatT;

	m_viewMat = m_transformMat.inverse();
}

const Matrix& Camera::getViewMatrix() const
{
	return m_viewMat;
}

const Matrix& Camera::getProjMatrix() const
{
	return m_projMat;
}
