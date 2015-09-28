#pragma once

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/export.hpp>

#include "Math/matrix.h"


// short version of unsigned int
#ifndef uint
typedef uint32_t uint;
#endif


enum class NetworkPriority
{
	LOW = 0,
	MEDIUM,
	HIGH,				//
	TOP,				// eg. PLAYER: no float compression, max frequency
};


// bitwise operations
// a=target variable, b=bit number to act upon 0-n
#define BIT_SET(a,b)		((a) |=  (1<<(b)))
#define BIT_CLEAR(a,b)		((a) &= ~(1<<(b)))
#define BIT_FLIP(a,b)		((a) ^=  (1<<(b)))
#define BIT_CHECK(a,b)		((a) &   (1<<(b)))

// x=target variable, y=mask
#define BITMASK_SET(x,y)	((x) |= (y))
#define BITMASK_CLEAR(x,y)	((x) &= (~(y)))
#define BITMASK_FLIP(x,y)	((x) ^= (y))
#define BITMASK_CHECK(x,y)	((x) &  (y))

// rolling functions
#define BIT_SET_ROLL(a)			{ a = a << 1;	a |= 1; }
#define BIT_CLEAR_ROLL(a)		{ a = a << 1;	a &= ~1;}
#define BIT_CHECK_ROLL(b, a)	{ b = a & 1;	a = a >> 1; }


// declarations for classes with symmetric serialization
#define SERIALIZABLE_CLASS			public:																		\
									virtual Node* clone();														\
									virtual bool updateProperties(const Node& other, bool serverSide = true);	\
																												\
									template <typename Archive>													\
									void serialize(Archive& ar, const uint version);

// declarations for classes with assymmetric serialization (save/load)
#define SERIALIZABLE_CLASS_SEPARATED																			\
									SERIALIZABLE_CLASS															\
									public:																		\
									template <typename Archive>													\
									void load(Archive& ar, const uint version);									\
									template <typename Archive>													\
									void save(Archive& ar, const uint version) const;							\

// definitions for classes with symmetric serialization
#define SERIALIZABLE(T)				template void T::serialize(boost::archive::binary_oarchive&, unsigned);		\
									template void T::serialize(boost::archive::binary_iarchive&, unsigned);		\
									template void T::serialize(boost::archive::text_oarchive&, unsigned);		\
									template void T::serialize(boost::archive::text_iarchive&, unsigned);		\
																												\
									Node* T::clone() {															\
										T* clone = new T();														\
										clone->updateProperties(*this);											\
										return clone;															\
									}																			\
									BOOST_CLASS_EXPORT_IMPLEMENT(T);

// definitions for classes with symmetric serialization
#define SERIALIZABLE_(NAMESPACE, T)	template void NAMESPACE::T::serialize(boost::archive::binary_oarchive&, unsigned);		\
									template void NAMESPACE::T::serialize(boost::archive::binary_iarchive&, unsigned);		\
									template void NAMESPACE::T::serialize(boost::archive::text_oarchive&, unsigned);		\
									template void NAMESPACE::T::serialize(boost::archive::text_iarchive&, unsigned);		\
																															\
									Node* NAMESPACE::T::clone() {															\
										NAMESPACE::T* clone = new NAMESPACE::T();											\
										clone->updateProperties(*this);														\
										return clone;																		\
									}																						\
									BOOST_CLASS_EXPORT_IMPLEMENT(NAMESPACE::T);

// definitions for classes with assymmetric serialization (save/load)
#define SERIALIZABLE_SEPARATED(T)	SERIALIZABLE(T)																\
									template void T::save(boost::archive::binary_oarchive&, unsigned) const;	\
									template void T::save(boost::archive::text_oarchive&, unsigned) const;		\
									template void T::load(boost::archive::binary_iarchive&, unsigned);			\
									template void T::load(boost::archive::text_iarchive&, unsigned);			\


// definitions for serializable objects without clone() and updateProperties() methods (like GameState)
#define SERIALIZABLE_NOT_UPD(T)		template void T::serialize(boost::archive::binary_oarchive&, unsigned);		\
									template void T::serialize(boost::archive::binary_iarchive&, unsigned);		\
									template void T::serialize(boost::archive::text_oarchive&, unsigned);		\
									template void T::serialize(boost::archive::text_iarchive&, unsigned);



// float compression
template <typename Archive>
void serializeF32_F16(Archive& ar, float& attrib, uint& attribmask, uint& attribIndex)
{
	using namespace utils::conversion;

	if (BIT_CHECK(attribmask, attribIndex++))
	{
		if (!BIT_CHECK(attribmask, 31))
		{
			// save
			short val = float32Tofloat16(attrib);
			ar& val;
		}
		else
		{
			// load
			short val;
			ar& val;
			attrib = float16Tofloat32(val);
		}
	}
}

template <typename Archive>
void serializeVec3_F16(Archive& ar, vec3& v, uint32_t& attribmask, uint32_t& attribIndex)
{
	using namespace utils::conversion;

	if (BIT_CHECK(attribmask, attribIndex++))
	{
		if (!BIT_CHECK(attribmask, 31))
		{
			// save
			short x = float32Tofloat16(v.x), y = float32Tofloat16(v.y), z = float32Tofloat16(v.z);
			ar& x& y& z;
		}
		else
		{
			// load
			short x, y, z;
			ar& x& y& z;
			v = vec3F16_to_vec3F32(x, y, z);
		}
	}
}

template <typename Archive>
void serializeMatrix_F16(Archive& ar, Matrix& mat, uint& attribmask, uint& attribIndex)
{
	using namespace utils::conversion;

	if (BIT_CHECK(attribmask, attribIndex++))
	{
		if (!BIT_CHECK(attribmask, 31))
		{
			// save
			Matrix compressedMat = mat;
			float* matArray = mat.getArray();
			for (int i = 0; i < 16; i++)
			{
				short val = float32Tofloat16(mat[i]);
				ar& val;
			}
		}
		else
		{
			// load
			float decompressedMat[16];
			for (int i = 0; i < 16; i++)
			{
				short val;
				ar& val;
				decompressedMat[i] = float16Tofloat32(val);
			}

			mat.set(decompressedMat);
		}
	}
}


// attrib serialization
// attrib:		the attribute of the object
// minPriority:	the minimum priority - objects over this priority must be serialized without float compression

#define SER_P(attrib)						if (BIT_CHECK(m_attribmask, m_attribIndex++)) ar & attrib;
#define SER_P_STR(attrib)					if (BIT_CHECK(m_attribmask, m_attribIndex++)) ar & attrib;

#define SER_P_F16(attrib, minPriority)		if (m_networkPriority >= minPriority)	{	SER_P(attrib);													\
											} else									{	serializeF32_F16(ar, attrib, m_attribmask, m_attribIndex); }

#define SER_P_M_F16(attrib, minPriority)	if (m_networkPriority >= minPriority)	{	SER_P(attrib);													\
											} else									{	serializeMatrix_F16(ar, attrib, m_attribmask, m_attribIndex); }

#define SER_P_VEC_F16(attrib, minPriority)	if (m_networkPriority >= minPriority)	{	SER_P(attrib);													\
											} else									{	serializeVec3_F16(ar, attrib, m_attribmask, m_attribIndex); }

// attrib updating
#define UP_P(param)							updateProperty(this->param, other.param, serverSide)
#define UP_CP(param)						param.updateProperties(other, serverSide)
#define UP_CP_SPTR(param, type)				updateComplexPropertySP<type>(this->param, other.param, serverSide)
