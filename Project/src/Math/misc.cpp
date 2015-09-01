#include "GameStdAfx.h"

#include "Math/CrimsonMath.h"

float normalizeAngle(float angle)
{
	float result = fmod(angle, 360);
	if (result < 0.0f)
	{
		result += 360;
	}

	return result;
}

vec3 normalizeOrientationVector(const vec3& v)
{
	vec3 result(normalizeAngle(v.x), normalizeAngle(v.y), normalizeAngle(v.z));
	return result;
}
