#include "GameStdAfx.h"

#include "Math/quaternion.h"

Quat::Quat(const vec3& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;

	calculateS();
}

Quat::Quat(float s, float x, float y, float z)
{
	this->s = s;
	this->x = x;
	this->y = y;
	this->z = z;
}

void Quat::set(float s, float x, float y, float z)
{
	this->s = s;
	this->x = x;
	this->y = y;
	this->z = z;
}

void Quat::loadIdentity()
{
	s = 1;
	x = y = z = 0;
}

void Quat::calculateS()
{
	s = 1.0f - x * x - y * y - z * z;

	if (s < 0.0f)
	{
		s = 0.0f;
	}
	else
	{
		s = -sqrt(s);
	}
}

Quat Quat::operator*(const Quat& q) const
{
	// q*q' = (s * s' - v * v', s * v' + s' * v + v x v')
	Quat quat;

	quat.s = (s * q.s) - (x * q.x) - (y * q.y) - (z * q.z);
	quat.x = (x * q.s) + (s * q.x) + (y * q.z) - (z * q.y);
	quat.y = (y * q.s) + (s * q.y) + (z * q.x) - (x * q.z);
	quat.z = (z * q.s) + (s * q.z) + (x * q.y) - (y * q.x);

	return quat;
}

Quat Quat::operator*(const vec3& v) const
{
	Quat quat;

	quat.s = -(x * v.x + y * v.y + z * v.z);
	quat.x = s * v.x + y * v.z - z * v.y;
	quat.y = s * v.y + z * v.x - x * v.z;
	quat.z = s * v.z + x * v.y - y * v.x;

	return quat;
}

vec3 Quat::rotateVec(const vec3& v) const
{
	Quat qout;

	Quat inv = conjugate();
	//inv.normalize();
	qout = *this * v * inv;

	return vec3(qout.x, qout.y, qout.z);
}

float Quat::dot(const Quat& q) const
{
	return s * q.s + x * q.x + y * q.y + z * q.z;
}

Quat& Quat::operator*=(const Quat& q)
{
	float ts, tx, ty, tz;
	ts = s * q.s - (x * q.x + y * q.y + z * q.z);
	tx = s * q.x + q.s * x + y * q.z - z * q.y;
	ty = s * q.y + q.s * y + z * q.x - x * q.z;
	tz = s * q.z + q.s * z + x * q.y - y * q.x;

	set(ts, tx, ty, tz);

	return *this;
}

bool Quat::operator==(const Quat& q) const
{
	return (s == q.s) && (x == q.x) && (y == q.y) && (z == q.z);
}

bool Quat::operator!=(const Quat& q) const
{
	return (s != q.s) || (x != q.x) || (y != q.y) || (z != q.z);
}

float Quat::operator[](int i) const
{
	switch (i)
	{
		case 0:
			return s;
		case 1:
			return x;
		case 2:
			return y;
		case 3:
			return z;
	}
	return 0;
}

Quat Quat::conjugate() const
{
	return Quat(s, -x, -y, -z);
}

void Quat::normalize()
{
	float len = length();
	s /= len;
	x /= len;
	y /= len;
	z /= len;
}

float Quat::length() const
{
	return (float)sqrt( double(s * s + x * x + y * y + z * z) );
}

void Quat::toFloatMatrix(float mat[]) const
{
	mat[ 0] = 1.0f - 2.0f * ( y * y + z * z );
	mat[ 1] = 2.0f * (x * y + z * s);
	mat[ 2] = 2.0f * (x * z - y * s);
	mat[ 3] = 0.0f;

	mat[ 4] = 2.0f * ( x * y - z * s );
	mat[ 5] = 1.0f - 2.0f * ( x * x + z * z );
	mat[ 6] = 2.0f * (z * y + x * s );
	mat[ 7] = 0.0f;

	mat[ 8] = 2.0f * ( x * z + y * s );
	mat[ 9] = 2.0f * ( y * z - x * s );
	mat[10] = 1.0f - 2.0f * ( x * x + y * y );
	mat[11] = 0.0f;

	mat[12] = 0;
	mat[13] = 0;
	mat[14] = 0;
	mat[15] = 1.0f;
}

Matrix Quat::getMatrix() const
{
	float fmat[16];
	toFloatMatrix(fmat);

	return Matrix(fmat);
}

vec3 Quat::getRotVector() const
{
	vec3 result;								// (heading, attitude, bank)

	double sqw = s * s;
	double sqx = x * x;
	double sqy = y * y;
	double sqz = z * z;
	double unit = sqx + sqy + sqz + sqw;		// if normalized is one, otherwise is correction factor
	double test = x * y + z * s;

	if (test > 0.499 * unit)  					// singularity at north pole
	{
		result.y = 2 * atan2(x, s);
		result.x = (float) PI / 2.0f;
		result.z = 0;

		return result;
	}

	if (test < -0.499 * unit)  					// singularity at south pole
	{
		result.y = -2 * atan2(x, s);
		result.x = (float)(-PI) / 2.0f;
		result.z = 0;

		return result;
	}

	result.y = (float) atan2((double) 2 * y * s - 2 * x * z, sqx - sqy - sqz + sqw);
	result.x = (float) asin(2 * test / unit);
	result.z = (float) atan2((double) 2 * x * s - 2 * y * z, -sqx + sqy - sqz + sqw);

	return result;
}

Quat Quat::fromAxis(const float angle, const vec3& axis)
{
	Quat quat;

	float radians = angle * PI_DEG;
	float sinThetaDiv2 = (float)sin( double(radians / 2.0f) );

	quat.x = axis.x * sinThetaDiv2;
	quat.y = axis.y * sinThetaDiv2;
	quat.z = axis.z * sinThetaDiv2;

	quat.s = (float)cos( double(radians / 2.0f) );

	return quat;
}

Quat Quat::slerp(const Quat& a, const Quat& b, const float t)
{
	Quat out;

	/* Check for out-of range parameter and return edge points if so */
	if (t <= 0.0)
	{
		return a;
	}

	if (t >= 1.0)
	{
		return b;
	}

	/* Compute "cosine of angle between quaternions" using dot product */
	Quat qa = a;
	Quat qb = b;
	float cosOmega = qa.dot(qb);

	/* If negative dot, use -q1.  Two quaternions q and -q
	 represent the same rotation, but may produce
	 different slerp.  We chose q or -q to rotate using
	 the acute angle. */
	float q1w = b.s;
	float q1x = b.x;
	float q1y = b.y;
	float q1z = b.z;

	if (cosOmega < 0.0f)
	{
		q1w = -q1w;
		q1x = -q1x;
		q1y = -q1y;
		q1z = -q1z;
		cosOmega = -cosOmega;
	}

	/* We should have two unit quaternions, so dot should be <= 1.0 */
	// assert (cosOmega < 1.1f);

	/* Compute interpolation fraction, checking for quaternions
	 almost exactly the same */
	float k0, k1;

	if (cosOmega > 0.9999f)
	{
		/* Very close - just use linear interpolation,
		 which will protect againt a divide by zero */

		k0 = 1.0f - t;
		k1 = t;
	}
	else
	{
		/* Compute the sin of the angle using the
		 trig identity sin^2(omega) + cos^2(omega) = 1 */
		float sinOmega = sqrt(1.0f - (cosOmega * cosOmega));

		/* Compute the angle from its sin and cosine */
		float omega = atan2(sinOmega, cosOmega);

		/* Compute inverse of denominator, so we only have
		 to divide once */
		float oneOverSinOmega = 1.0f / sinOmega;

		/* Compute interpolation parameters */
		k0 = sin((1.0f - t) * omega) * oneOverSinOmega;
		k1 = sin(t * omega) * oneOverSinOmega;
	}

	/* Interpolate and return new quaternion */
	out.s = (k0 * a.s) + (k1 * q1w);
	out.x = (k0 * a.x) + (k1 * q1x);
	out.y = (k0 * a.y) + (k1 * q1y);
	out.z = (k0 * a.z) + (k1 * q1z);

	return out;
}
