#pragma once

#ifdef USE_GLM
typedef glm::vec2 vec2;

#else
class vec2
{
public:
	float x, y;

public:
	// Constructors
	vec2();
	vec2(float a);
	vec2(float a, float b);
	vec2(float t[2]);

	void set(float a);
	void set(float a, float b);
	void set(float t[2]);

	vec2 operator-() const;

	friend std::ostream& operator<<(std::ostream& os, const vec2& v)
	{
		return os << v.x << "  " << v.y;
	}

	//----------------------------------------------------------
	// Scalar multiplication
	vec2 operator*(float f) const;
	vec2 operator*(const vec2& v) const;

	// Scalar division

	vec2 operator/(float f) const;
	vec2 operator/(const vec2& v) const;

	//----------------------------------------------------------
	// vec2 Arithmetic Operations
	vec2 operator+(const vec2& v) const;
	vec2 operator-(const vec2& v) const;

	// 3D Exterior Cross Product
	//vec2 operator^(const vec2& v) const;

	//----------------------------------------------------------
	// Shorthand Ops
	vec2& operator*=(float c);
	vec2& operator/=(float c);

	vec2& operator+=(const vec2& v);
	vec2& operator-=(const vec2& v);

	vec2& operator*=(const vec2& v);
	vec2& operator/=(const vec2& v);

	bool operator==(const vec2& v) const;
	bool operator!=(const vec2& v) const;
	float operator[](int i) const;

	//----------------------------------------------------------
	// vec2 Properties
	vec2 abs();

	float length() const;
	float length2() const;

	void setLength(float l);

	//----------------------------------------------------------
	// Special Operations
	void normalize();
	vec2 normalized();

	float dot(const vec2& v) const;
	vec2 rotate(float angle) const;

	vec2 interpolate(vec2 v, float t) const;


	bool isZeroVector() const;


	template <typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& x;
		ar& y;
	}
};

// compatible with glm
float dot(const vec2& v1, const vec2& v2);
vec2 normalize(const vec2& v1);

#endif // USE_GLM

vec2 interpolate(const vec2& v1, vec2 v, float t);
