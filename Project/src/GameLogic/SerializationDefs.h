#pragma once

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/export.hpp>

#include "Math/matrix.h"


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


// float compression
template <typename Archive, typename >
void serializeF32_F16(Archive& ar, float& attrib, uint8_t& attribMask, uint8_t& attribIndex)
{
	if (BIT_CHECK(attribMask, attribIndex++))
	{
		if (!BIT_CHECK(attribMask, 7))
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
void serializeVec2(Archive& ar, vec2& v, uint8_t& attribMask, uint8_t& attribIndex, bool useF16)
{
	if (BIT_CHECK(attribMask, attribIndex++))
	{
		if (!BIT_CHECK(attribMask, 7))
		{
			// save
			if (useF16)
			{
				short x = utils::float32Tofloat16(v.x), y = utils::float32Tofloat16(v.y);
				ar& x & y;
			}
			else
			{
				ar& v.x & v.y;
			}
		}
		else
		{
			if (useF16)
			{
				short x, y;
				ar& x & y;
				v = utils::vec2F16_to_vec2F32(x, y);
			}
			else
			{
				// load
				ar& v.x & v.y;
			}
		}
	}
}

template <typename Archive>
void serializeVec3(Archive& ar, vec3& v, uint8_t& attribMask, uint8_t& attribIndex, bool useF16)
{
	if (BIT_CHECK(attribmask, attribIndex++))
	{
		if (!BIT_CHECK(attribmask, 7))
		{
			// save
			if (useF16)
			{
				short x = utils::float32Tofloat16(v.x), y = utils::float32Tofloat16(v.y), z = utils::float32Tofloat16(v.z);
				ar& x & y & z;
			}
			else
			{
				ar& v.x & v.y & v.z;
			}
		}
		else
		{
			// load
			if (useF16)
			{
				short x, y, z;
				ar& x & y & z;
				v = utils::vec3F16_to_vec3F32(x, y, z);
			}
			else
			{
				// load
				ar& v.x & v.y & v.z;
			}
		}
	}
}

template <typename Archive>
void serializeVec3_F16(Archive& ar, vec3& v, uint8_t& attribMask, uint8_t& attribIndex)
{
	if (BIT_CHECK(attribMask, attribIndex++))
	{
		if (!BIT_CHECK(attribMask, 7))
		{
			// save
			short x = utils::float32Tofloat16(v.x), y = utils::float32Tofloat16(v.y), z = utils::float32Tofloat16(v.z);
			ar& x & y & z;
		}
		else
		{
			// load
			short x, y, z;
			ar& x & y & z;
			v = utils::vec3F16_to_vec3F32(x, y, z);
		}
	}
}

template <typename Archive>
void serializeMatrix_F16(Archive& ar, Matrix& mat, uint8_t& attribMask, uint8_t& attribIndex)
{
	if (BIT_CHECK(attribMask, attribIndex++))
	{
		if (!BIT_CHECK(attribMask, 7))
		{
			// save
			Matrix compressedMat = mat;
			float* matArray = mat.getArray();
			for (int i = 0; i < 16; i++)
			{
				short val = utils::float32Tofloat16(mat[i]);
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
				decompressedMat[i] = utils::float16Tofloat32(val);
			}

			mat.set(decompressedMat);
		}
	}
}


// attrib serialization
// attrib:		the attribute of the object
// minPriority:	the minimum priority - objects over this priority must be serialized without float compression

#define SER_P(attrib)						if (attribMask[m_attribIndex++]))		ar& attrib;

#define SER_P_F16(attrib, minPriority)		if (m_networkPriority >= minPriority)	SER_P(attrib);													\
											else									serializeF32_F16(ar, attrib, attribMask, m_attribIndex);

#define SER_P_M_F16(attrib, minPriority)	if (m_networkPriority >= minPriority)	SER_P(attrib);													\
											else									serializeMatrix_F16(ar, attrib, attribMask, m_attribIndex);

#define SER_P_VEC2(attrib, minPriority)		serializeVec2(ar, attrib, attribMask, m_attribIndex, m_networkPriority >= minPriority);
#define SER_P_VEC3(attrib, minPriority)		serializeVec3(ar, attrib, attribMask, m_attribIndex, m_networkPriority >= minPriority);

// attrib updating
#define UP_P(param)							updateProperty(this->param, other.param, serverSide)
#define UP_CP(param)						param.updateProperties(other, serverSide)
#define UP_CP_SPTR(param, type)				updateComplexPropertySP<type>(this->param, other.param, serverSide)

#define SERIALIZABLE(T)						template void T::serialize(boost::archive::binary_oarchive&, unsigned);		\
											template void T::serialize(boost::archive::binary_iarchive&, unsigned);		\
											template void T::serialize(boost::archive::text_oarchive&, unsigned);		\
											template void T::serialize(boost::archive::text_iarchive&, unsigned);		\
