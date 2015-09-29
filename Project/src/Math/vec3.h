#pragma once

#define	PI 3.14159263f
#define PIHALF 1.57079632679f
#define PI_DEG 0.01745329f	//angle * PI_DEG = radians
#define EPSILON 0.001


#ifndef USE_GLM

class vec3
{
public:
	float x, y, z;

public:
	// Constructors
	vec3();
	vec3(float a);
	vec3(float a, float b, float c);
	vec3(float t[3]);

	void set(float a);
	void set(float a, float b, float c);
	void set(float t[3]);

	vec3 operator-() const;

	friend std::ostream& operator<<(std::ostream& os, const vec3& v)
	{
		return os << v.x << "  " << v.y << "  " << v.z;
	}

	//----------------------------------------------------------
	// Scalar multiplication
	vec3 operator*(float f) const;
	vec3 operator*(const vec3& v) const;

	// Scalar division

	vec3 operator/(float f) const;
	vec3 operator/(const vec3& v) const;

	//----------------------------------------------------------
	// vec3 Arithmetic Operations
	vec3 operator+(const vec3& v) const;
	vec3 operator-(const vec3& v) const;

	// 3D Exterior Cross Product
	//vec3 operator^(const vec3& v) const;

	//----------------------------------------------------------
	// Shorthand Ops
	vec3& operator*=(float c);
	vec3& operator/=(float c);

	vec3& operator+=(const vec3& v);
	vec3& operator-=(const vec3& v);

	vec3& operator*=(const vec3& v);
	vec3& operator/=(const vec3& v);
	vec3& operator^=(const vec3& v);

	bool operator==(const vec3& v) const;
	bool operator!=(const vec3& v) const;
	float operator[](int i) const;

	//----------------------------------------------------------
	// vec3 Properties
	vec3 abs();

	float length() const;
	float length2() const;

	void setLength(float l);

	//----------------------------------------------------------
	// Special Operations
	void normalize();
	vec3 normalized() const;

	float dot(const vec3& v) const;
	vec3 cross(const vec3& v) const;
	vec3 rotate(vec3& axis, float angle) const;

	vec3 interpolate(vec3 v, float t) const;


	bool isZeroVector() const;


	template <typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& x;
		ar& y;
		ar& z;
	}
};

// compatible with glm
float dot(const vec3& v1, const vec3& v2);
vec3 cross(const vec3& v1, const vec3& v2);
vec3 normalize(const vec3& v1);

#endif // USE_GLM

vec3 interpolate(const vec3& v1, vec3 v, float t);
