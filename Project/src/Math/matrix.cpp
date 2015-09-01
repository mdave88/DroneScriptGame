#include "GameStdAfx.h"

#include "Math/matrix.h"

Matrix::Matrix()
{
	loadIdentity();
}

Matrix::Matrix(float d1, float d2, float d3)
{
	clear();
	m[0][0] = d1;
	m[1][1] = d2;
	m[2][2] = d3;
	m[3][3] = 1;
}

Matrix::Matrix(float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3, float z3, float w3, float x4, float y4, float z4, float w4)
{
	m[0][0] = x1;
	m[0][1] = y1;
	m[0][2] = z1;
	m[0][3] = w1;
	m[1][0] = x2;
	m[1][1] = y2;
	m[1][2] = z2;
	m[1][3] = w2;
	m[2][0] = x3;
	m[2][1] = y3;
	m[2][2] = z3;
	m[2][3] = w3;
	m[3][0] = x4;
	m[3][1] = y4;
	m[3][2] = z4;
	m[3][3] = w4;
}

Matrix::Matrix(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4)
{
	vec3ToFloatA4(v1, m[0]);
	vec3ToFloatA4(v2, m[1]);
	vec3ToFloatA4(v3, m[2]);
	vec3ToFloatA4(v4, m[3]);
	m[3][3] = 1;
}

Matrix::Matrix(float mf[4][4])
{
	set(mf);
}

Matrix::Matrix(float mf[16])
{
	set(mf);
}

void Matrix::set(float mf[4][4])
{
	m[0][0] = mf[0][0];
	m[0][1] = mf[0][1];
	m[0][2] = mf[0][2];
	m[0][3] = mf[0][3];
	m[1][0] = mf[1][0];
	m[1][1] = mf[1][1];
	m[1][2] = mf[1][2];
	m[1][3] = mf[1][3];
	m[2][0] = mf[2][0];
	m[2][1] = mf[2][1];
	m[2][2] = mf[2][2];
	m[2][3] = mf[2][3];
	m[3][0] = mf[3][0];
	m[3][1] = mf[3][1];
	m[3][2] = mf[3][2];
	m[3][3] = mf[3][3];
}

void Matrix::set(float mf[16])
{
	m[0][0] = mf[0];
	m[0][1] = mf[1];
	m[0][2] = mf[2];
	m[0][3] = mf[3];
	m[1][0] = mf[4];
	m[1][1] = mf[5];
	m[1][2] = mf[6];
	m[1][3] = mf[7];
	m[2][0] = mf[8];
	m[2][1] = mf[9];
	m[2][2] = mf[10];
	m[2][3] = mf[11];
	m[3][0] = mf[12];
	m[3][1] = mf[13];
	m[3][2] = mf[14];
	m[3][3] = mf[15];
}

Matrix::Matrix(const vec3& pos, const vec3& rot, const vec3& scale)
{
	loadIdentity();
	setRotation(rot);
	setScale(scale);
	setTranslation(pos);
}

void Matrix::clear()
{
	memset(&m[0][0], 0, sizeof(m));
}

void Matrix::loadIdentity()
{
	clear();
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
}

bool Matrix::operator==(const Matrix& mat) const
{
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			if (fabs(m[x][y] - mat.m[x][y]) > EPSILON)
			{
				return false;
			}

	return true;
}

bool Matrix::operator!=(const Matrix& mat) const
{
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			if (fabs(m[x][y] - mat.m[x][y]) > EPSILON)
			{
				return true;
			}

	return false;
}

Matrix& Matrix::operator=(const Matrix& mat)
{
	if (this != &mat)
	{
		set(((Matrix) mat).getArray());
	}

	return *this;
}

Matrix Matrix::operator+(const Matrix& mat) const
{
	Matrix result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = m[i][j] + mat.m[i][j];
		}
	}

	return result;
}

Matrix Matrix::operator*(const Matrix& mat) const
{
	Matrix result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = 0;
			for (int k = 0; k < 4; k++)
			{
				result.m[i][j] += mat.m[i][k] * m[k][j];
			}
		}
	}

	return result;
}

vec3 Matrix::operator*(const vec3& v) const
{
	float Xh = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0];
	float Yh = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1];
	float Zh = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2];
	float h = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3];

	return vec3(Xh / h, Yh / h, Zh / h);
}

float Matrix::operator[](int index) const
{
	return m[index / 4][index % 4];
}


void Matrix::setPerspective(float fovY, float aspect, float nearPlane, float farPlane)
{
	float height	= 2.0f * nearPlane * tanf(fovY * PI / 360.0f);
	float width		= height * aspect;
	float n2		= 2.0f * nearPlane;
	float rcpnmf	= 1.0f / (nearPlane - farPlane);

	m[0][0] = n2 / width;
	m[1][0] = 0;
	m[2][0] = 0;
	m[3][0] = 0;

	m[0][1] = 0;
	m[1][1] = n2 / height;
	m[2][1] = 0;
	m[3][1] = 0;

	m[0][2] = 0;
	m[1][2] = 0;
	m[2][2] = (farPlane + nearPlane) * rcpnmf;
	m[3][2] = farPlane * rcpnmf * n2;

	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = -1.f;
	m[3][3] = 0;
}

void Matrix::setTranslation(const vec3& translation)
{
	m[3][0] = translation.x;
	m[3][1] = translation.y;
	m[3][2] = translation.z;
}

vec3 Matrix::getTranslation() const
{
	return vec3(m[3][0], m[3][1], m[3][2]);
}

void Matrix::setScale(const vec3& scale)
{
	Matrix mat = Matrix(scale.x, scale.y, scale.z);
	*this = *this * mat;
}

void Matrix::setRotation(const vec3& rotation)
{
	vec3 rot = rotation;
	rot *= -PI_DEG;

	float cr = cos(rot.x);
	float sr = sin(rot.x);
	float cp = cos(rot.y);
	float sp = sin(rot.y);
	float cy = cos(rot.z);
	float sy = sin(rot.z);

	float srsp = sr * sp;
	float crsp = cr * sp;

	m[0][0] = (float) (cp * cy);
	m[1][0] = (float) (cp * sy);
	m[2][0] = (float) (-sp);

	m[0][1] = (float) (srsp * cy - cr * sy);
	m[1][1] = (float) (srsp * sy + cr * cy);
	m[2][1] = (float) (sr * cp);

	m[0][2] = (float) (crsp * cy + sr * sy);
	m[1][2] = (float) (crsp * sy - sr * cy);
	m[2][2] = (float) (cr * cp);
}

void Matrix::setInverseRotation(const vec3& rotation)
{
	vec3 rot = rotation;
	rot *= -PI_DEG;

	float cr = cos(rot.x);
	float sr = sin(rot.x);
	float cp = cos(rot.y);
	float sp = sin(rot.y);
	float cy = cos(rot.z);
	float sy = sin(rot.z);

	float srsp = sr * sp;
	float crsp = cr * sp;

	m[0][0] = (float) (cp * cy);
	m[0][1] = (float) (cp * sy);
	m[0][2] = (float) (-sp);

	m[1][0] = (float) (srsp * cy - cr * sy);
	m[1][1] = (float) (srsp * sy + cr * cy);
	m[1][2] = (float) (sr * cp);

	m[2][0] = (float) (crsp * cy + sr * sy);
	m[2][1] = (float) (crsp * sy - sr * cy);
	m[2][2] = (float) (cr * cp);
}

vec3 Matrix::getUpVec() const
{
	return vec3(m[0][1], m[1][1], m[2][1]);
}

vec3 Matrix::getRightVec() const
{
	return vec3(m[0][0], m[1][0], m[2][0]);
}

vec3 Matrix::getForwardVec() const
{
	return vec3(m[0][2], m[1][2], m[2][2]);
}

/**
 * Transposing our matrix.
 */
Matrix Matrix::transpose() const
{
	Matrix result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[j][i] = m[i][j];
		}
	}

	return result;
}

Matrix Matrix::inverse() const
{
	Matrix res;

	vec3 T(m[3][0], m[3][1], m[3][2]);

	vec3 Rx(m[0][0], m[0][1], m[0][2]);
	vec3 Ry(m[1][0], m[1][1], m[1][2]);
	vec3 Rz(m[2][0], m[2][1], m[2][2]);

	float Tx = Rx.dot(T);
	float Ty = Ry.dot(T);
	float Tz = Rz.dot(T);

	res.m[0][0] = m[0][0];
	res.m[0][1] = m[1][0];
	res.m[0][2] = m[2][0];
	res.m[0][3] = 0;
	res.m[1][0] = m[0][1];
	res.m[1][1] = m[1][1];
	res.m[1][2] = m[2][1];
	res.m[1][3] = 0;
	res.m[2][0] = m[0][2];
	res.m[2][1] = m[1][2];
	res.m[2][2] = m[2][2];
	res.m[2][3] = 0;
	res.m[3][0] =      Tx;
	res.m[3][1] =      Ty;
	res.m[3][2] =      Tz;
	res.m[3][3] = 1;

	return res;
}

/**
 * Transforming the given vector with our matrix.
 */
vec3 Matrix::transform(const vec3& v) const
{
	//return vec3(v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2] + m[0][3], v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2] + m[1][3], v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2] + m[2][3]);
	//return (*this) * v;
	return vec3(v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + m[3][0], v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + m[3][1], v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + m[3][2]);
}

/**
 * Transforming the given vector with our matrix's upper left 3x3 corner.
 */
vec3 Matrix::transform3x3(const vec3& v) const
{
	return vec3(v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0], v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1], v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2]);
}

/**
 * Transforming the given vector with our matrix's inverse.
 */
vec3 Matrix::invTransform(const vec3& v) const
{
	//return (*this).transpose() * v;
	return vec3(v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2] + m[0][3], v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2] + m[1][3], v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2] + m[2][3]);
}

/**
 * Transforming the given vector with our matrix's inverse upper left 3x3 corner.
 */
vec3 Matrix::invTransform3x3(const vec3& v) const
{
	return vec3(v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2], v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2], v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2]);
}

float* Matrix::getArray() const
{
	return (float*)(&m[0][0]);
}

void Matrix::toArray(float matrixArray[16]) const
{
	memcpy(matrixArray, &m[0][0], 16 * sizeof(float));
}

vec3 transformVector(const vec3& v, const vec3& trans, vec3 rot)
{
	Matrix mat;
	mat.setInverseRotation(rot);
	mat.setTranslation(trans);

	return mat * v;

	/*
	 rot *= -PI_DEG;

	 Matrix mrz(cos(rot.z), sin(rot.z), 0.0, 0.0,
	 -sin(rot.z), cos(rot.z), 0.0, 0.0,
	 0.0, 0.0, 1.0, 0.0,
	 0.0, 0.0, 0.0, 1.0);

	 Matrix mry(cos(rot.y), 0.0, -sin(rot.y), 0.0,
	 0.0, 1.0, 0.0, 0.0,
	 sin(rot.y), 0.0, cos(rot.y), 0.0,
	 0.0, 0.0, 0.0, 1.0);

	 Matrix mrx(1.0, 0.0, 0.0, 0.0,
	 0.0, cos(rot.x), sin(rot.x), 0.0,
	 0.0, -sin(rot.x), cos(rot.x), 0.0,
	 trans.x, trans.y, trans.z, 1.0);


	 return (mrz * mry * mrx) * v;
	 */
}
