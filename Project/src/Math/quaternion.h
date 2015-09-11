#pragma once

class Quat
{
public:
	float s, x, y, z;

public:
	static Quat slerp(const Quat& q1, const Quat& q2, const float t);
	static Quat fromAxis(const float angle, const vec3& axis);

	// Constructors
	Quat(const vec3& v);
	Quat(float s = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);

	void loadIdentity();
	void set(float a, float b, float c, float d);

	void calculateS();

	//----------------------------------------------------------
	// Scalar multiplication

	Quat operator*(const Quat& q) const;
	Quat operator*(const vec3& v) const;
	Quat& operator*=(const Quat& q);

	bool operator==(const Quat& q) const;
	bool operator!=(const Quat& q) const;
	float operator[](int i) const;

	Quat conjugate() const;
	float dot(const Quat& q) const;
	float length() const;
	void normalize();

	vec3 rotateVec(const vec3& v) const;

	void toFloatMatrix(float mat[]) const;
	Matrix getMatrix() const;

	vec3 getRotVector() const;

	friend std::ostream& operator<<(std::ostream& os, const Quat& q)
	{
		return os << q.s << " + " << q.x << "i + " << q.y << "j + " << q.z << "k";
	}
};
