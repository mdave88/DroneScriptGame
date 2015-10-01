#include "GameStdAfx.h"
#include "Math/vec2.h"

#ifndef USE_GLM
vec2::vec2()
{
	x = y = 0;
}

vec2::vec2(float a)
{
	set(a);
}

vec2::vec2(float a, float b)
{
	set(a, b);
}

vec2::vec2(float t[2])
{
	set(t);
}

void vec2::set(float a)
{
	x = y = a;
}

void vec2::set(float a, float b)
{
	x = a;
	y = b;
}

void vec2::set(float t[2])
{
	x = t[0];
	y = t[1];
}

// Unary minus

vec2 vec2::operator-() const
{
	vec2 v;
	v.x = -x;
	v.y = -y;

	return v;
}

//----------------------------------------------------------
// Scalar multiplication

vec2 vec2::operator*(float f) const
{
	return vec2(x * f, y * f);
}

vec2 vec2::operator*(const vec2& v) const
{
	return vec2(x * v.x, y * v.y);
}

// Scalar division

vec2 vec2::operator/(float f) const
{
	return vec2(x / f, y / f);
}

vec2 vec2::operator/(const vec2& v) const
{
	return vec2(x / v.x, y / v.y);
}

//----------------------------------------------------------
// vec2 Arithmetic Operations

vec2 vec2::operator+(const vec2& v) const
{
	vec2 result;
	result.x = x + v.x;
	result.y = y + v.y;

	return result;
}

vec2 vec2::operator-(const vec2& v) const
{
	vec2 result;
	result.x = x - v.x;
	result.y = y - v.y;

	return result;
}

//----------------------------------------------------------
// Shorthand Ops

vec2& vec2::operator*=(float c)
{
	x *= c;
	y *= c;

	return *this;
}

vec2& vec2::operator/=(float c)
{
	x /= c;
	y /= c;

	return *this;
}

vec2& vec2::operator*=(const vec2& v)
{
	x *= v.x;
	y *= v.y;

	return *this;
}

vec2& vec2::operator/=(const vec2& v)
{
	x /= v.x;
	y /= v.y;

	return *this;
}

vec2& vec2::operator+=(const vec2& v)
{
	x += v.x;
	y += v.y;

	return *this;
}

vec2& vec2::operator-=(const vec2& v)
{
	x -= v.x;
	y -= v.y;

	return *this;
}

bool vec2::operator==(const vec2& v) const
{
	return (fabs(x - v.x) < EPSILON) && (fabs(y - v.y) < EPSILON);
}

bool vec2::operator!=(const vec2& v) const
{
	return (fabs(x - v.x) > EPSILON) || (fabs(y - v.y) > EPSILON);
}

float vec2::operator[](int i) const
{
	switch (i)
	{
		case 0:
			return x;
		case 1:
			return y;
	}
	return 0;
}

//----------------------------------------------------------
// vec2 Properties

vec2 vec2::abs()
{
	return vec2(fabs(x), fabs(y));
}

float vec2::length() const
{
	return sqrt(x * x + y * y);
}

float vec2::length2() const
{
	return (x * x + y * y);
}

void vec2::setLength(float l)
{
	float len = length();
	x *= l / len;
	y *= l / len;
}

//----------------------------------------------------------
// Special Operations

void vec2::normalize()
{
	const float len = length();
	if (len == 0)
	{
		return;
	}

	x /= len;
	y /= len;
}

vec2 vec2::normalized()
{
	vec2 result(x, y);

	const float len = result.length();
	if (len == 0)
	{
		return vec2(0.0f);
	}

	result.x /= len;
	result.y /= len;

	return result;
}

float vec2::dot(const vec2& v) const
{
	return (x * v.x + y * v.y);
}

vec2 vec2::rotate(float angle) const
{
	const float theta = angle * PI_DEG;

	const float cs = cos(theta);
	const float sn = sin(theta);

	return vec2(x * cs - y * sn, x * sn + y * cs);
}

vec2 vec2::interpolate(vec2 v, float t) const
{
	const float inv_t = 1.0f - t;

	v.x = x * inv_t + v.x * t;
	v.y = y * inv_t + v.y * t;

	return v;
}

bool vec2::isZeroVector() const
{
	if ((fabs(x) < EPSILON) && (fabs(y) < EPSILON))
	{
		return true;
	}

	return false;
}

float dot(const vec2& v1, const vec2& v2)
{
	return v1.dot(v2);
}

vec2 normalize(const vec2& v)
{
	return v.normalized();
}

#endif // USE_GLM

vec2 interpolate(const vec2& v1, vec2 v2, float t)
{
	const float inv_t = 1.0f - t;

	v2.x = v1.x * inv_t + v2.x * t;
	v2.y = v1.y * inv_t + v2.y * t;

	return v2;
}
