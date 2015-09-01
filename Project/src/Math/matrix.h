
#ifndef MATRIX_H
#define MATRIX_H

#define vec3ToFloatA4(v,fa)	fa[0]=v.x; fa[1]=v.y; fa[2]=v.z; fa[3]=0;

class Matrix
{
	float m[4][4];

public:
	Matrix();

	Matrix(float mf[16]);
	Matrix(float mf[4][4]);
	Matrix(float d1, float d2, float d3);
	Matrix(float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3, float z3, float w3, float x4, float y4, float z4, float w4);
	Matrix(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4);

	Matrix(const vec3& pos, const vec3& rot, const vec3& scale = vec3(1.0f));

	void set(float mf[16]);
	void set(float mf[4][4]);

	void clear();
	void loadIdentity();

	bool operator==(const Matrix& mat) const;
	bool operator!=(const Matrix& mat) const;

	Matrix& operator=(const Matrix& mat);
	Matrix operator+(const Matrix& mat) const;
	Matrix operator*(const Matrix& mat) const;
	vec3 operator*(const vec3& v) const;

	float operator[](int index) const;


	Matrix transpose() const;
	Matrix inverse() const;

	vec3 transform(const vec3& v) const;
	vec3 transform3x3(const vec3& v) const;

	vec3 invTransform(const vec3& v) const;
	vec3 invTransform3x3(const vec3& v) const;

	// getters-setters
	void setPerspective(float fovY, float aspect, float nearPlane, float farPlane);

	void setScale(const vec3& scale);

	void setRotation(const vec3& rotation);
	void setInverseRotation(const vec3& rotation);

	void setTranslation(const vec3& translation);
	vec3 getTranslation() const;

	vec3 getUpVec() const;
	vec3 getRightVec() const;
	vec3 getForwardVec() const;

	float* getArray() const;
	void toArray(float matrixArray[16]) const;

	friend std::ostream& operator<<(std::ostream& os, const Matrix& mat)
	{
		os.precision(2);
		os << mat.m[0][0] << "\t" << mat.m[1][0] << "\t" << mat.m[2][0] << "\t" << mat.m[3][0] << std::endl;
		os << mat.m[0][1] << "\t" << mat.m[1][1] << "\t" << mat.m[2][1] << "\t" << mat.m[3][1] << std::endl;
		os << mat.m[0][2] << "\t" << mat.m[1][2] << "\t" << mat.m[2][2] << "\t" << mat.m[3][2] << std::endl;
		os << mat.m[0][3] << "\t" << mat.m[1][3] << "\t" << mat.m[2][3] << "\t" << mat.m[3][3] << std::endl;

		return os;
	}


	template <typename Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& m;
	}
};

extern vec3 transformVector(const vec3& v, const vec3& trans, vec3 rot);

#endif
