#pragma once

class Camera
{
public:
	Camera();
	virtual ~Camera();

	void setPerspective(float fovY = 45.0f, int screenWidth = 600, int screenHeight = 600, float nearPlane = 1.0f, float farPlane = 1000.0f);

	void update(vec3 pos, vec3 rot);

	const vec3& getPos() const;
	const Matrix& getViewMatrix() const;
	const Matrix& getProjMatrix() const;

private:
	vec3	m_pos;
	vec3	m_rot;

	Matrix	m_projMat;
	Matrix	m_viewMat;
};

typedef std::shared_ptr<Camera> CameraPtr;
