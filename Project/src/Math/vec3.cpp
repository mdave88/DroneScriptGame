#include "GameStdAfx.h"
#include "Math/vec3.h"

#ifndef USE_GLM
vec3::vec3()
{
	x = y = z = 0;
}

vec3::vec3(float a)
{
	set(a);
}

vec3::vec3(float a, float b, float c)
{
	set(a, b, c);
}

vec3::vec3(float t[3])
{
	set(t);
}

void vec3::set(float a)
{
	x = y = z = a;
}

void vec3::set(float a, float b, float c)
{
	x = a;
	y = b;
	z = c;
}

void vec3::set(float t[3])
{
	x = t[0];
	y = t[1];
	z = t[2];
}

// Unary minus

vec3 vec3::operator-() const
{
	vec3 v;
	v.x = -x;
	v.y = -y;
	v.z = -z;

	return v;
}

//----------------------------------------------------------
// Scalar multiplication

vec3 vec3::operator*(float f) const
{
	return vec3(x * f, y * f, z * f);
}

vec3 vec3::operator*(const vec3& v) const
{
	return vec3(x * v.x, y * v.y, z * v.z);
}

// Scalar division

vec3 vec3::operator/(float f) const
{
	return vec3(x / f, y / f, z / f);
}

vec3 vec3::operator/(const vec3& v) const
{
	return vec3(x / v.x, y / v.y, z / v.z);
}

//----------------------------------------------------------
// vec3 Arithmetic Operations

vec3 vec3::operator+(const vec3& v) const
{
	vec3 result;
	result.x = x + v.x;
	result.y = y + v.y;
	result.z = z + v.z;

	return result;
}

vec3 vec3::operator-(const vec3& v) const
{
	vec3 result;
	result.x = x - v.x;
	result.y = y - v.y;
	result.z = z - v.z;

	return result;
}

// 3D Exterior Cross Product
//vec3 vec3::operator^(const vec3& v) const {
//	vec3 result;
//
//	result.x = (y * v.z) - (v.y * z);
//	result.y = (z * v.x) - (v.z * x);
//	result.z = (x * v.y) - (v.x * y);
//
//	return result;
//}

//----------------------------------------------------------
// Shorthand Ops

vec3& vec3::operator*=(float c)
{
	x *= c;
	y *= c;
	z *= c;

	return *this;
}

vec3& vec3::operator/=(float c)
{
	x /= c;
	y /= c;
	z /= c;

	return *this;
}

vec3& vec3::operator*=(const vec3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;

	return *this;
}

vec3& vec3::operator/=(const vec3& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;

	return *this;
}

vec3& vec3::operator+=(const vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;

	return *this;
}

vec3& vec3::operator-=(const vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;

	return *this;
}

vec3& vec3::operator^=(const vec3& v)
{
	vec3 o(x, y, z);

	x = (o.y * v.z) - (v.y * o.z);
	y = (o.z * v.x) - (v.z * o.x);
	z = (o.x * v.y) - (v.x * o.y);

	return *this;
}

bool vec3::operator==(const vec3& v) const
{
	return (fabs(x - v.x) < EPSILON) && (fabs(y - v.y) < EPSILON) && (fabs(z - v.z) < EPSILON);
}

bool vec3::operator!=(const vec3& v) const
{
	return (fabs(x - v.x) > EPSILON) || (fabs(y - v.y) > EPSILON) || (fabs(z - v.z) > EPSILON);
}

float vec3::operator[](int i) const
{
	switch (i)
	{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
	}
	return 0;
}

//----------------------------------------------------------
// vec3 Properties

vec3 vec3::abs()
{
	return vec3(fabs(x), fabs(y), fabs(z));
}

float vec3::length() const
{
	return sqrt(x * x + y * y + z * z);
}

float vec3::length2() const
{
	return (x * x + y * y + z * z);
}

void vec3::setLength(float l)
{
	float len = length();
	x *= l / len;
	y *= l / len;
	z *= l / len;
}

//----------------------------------------------------------
// Special Operations

void vec3::normalize()
{
	float len = length();
	if (len == 0)
	{
		return;
	}

	x /= len;
	y /= len;
	z /= len;
}

vec3 vec3::normalized() const
{
	vec3 result(x, y, z);

	float len = result.length();
	if (len == 0)
	{
		return vec3(0.0f);
	}

	result.x /= len;
	result.y /= len;
	result.z /= len;

	return result;
}

float vec3::dot(const vec3& v) const
{
	return (x * v.x + y * v.y + z * v.z);
}

vec3 vec3::cross(const vec3& v) const
{
	vec3 result;

	result.x = (y * v.z) - (v.y * z);
	result.y = (z * v.x) - (v.z * x);
	result.z = (x * v.y) - (v.x * y);

	return result;
}

vec3 vec3::rotate(vec3& axis, float angle) const
{
	vec3 iv(x, y, z);
	iv.normalize();

	axis.normalize();
	axis = axis.cross(iv);

	float radian = angle * PI_DEG;

	return (iv * cos(radian) + axis * sin(radian));
}

vec3 vec3::interpolate(vec3 v, float t) const
{
	float inv_t = 1.0f - t;

	v.x = x * inv_t + v.x * t;
	v.y = y * inv_t + v.y * t;
	v.z = z * inv_t + v.z * t;

	return v;
}

bool vec3::isZeroVector() const
{
	if ((fabs(x) < EPSILON) && (fabs(y) < EPSILON) && (fabs(z) < EPSILON))
	{
		return true;
	}

	return false;
}

float dot(const vec3& v1, const vec3& v2)
{
	return v1.dot(v2);
}

vec3 cross(const vec3& v1, const vec3& v2)
{
	return v1.cross(v2);
}

vec3 normalize(const vec3& v)
{
	return v.normalized();
}

#endif // USE_GLM

vec3 interpolate(const vec3& v1, vec3 v2, float t)
{
	const float inv_t = 1.0f - t;

	v2.x = v1.x * inv_t + v2.x * t;
	v2.y = v1.y * inv_t + v2.y * t;
	v2.z = v1.z * inv_t + v2.z * t;

	return v2;
}
