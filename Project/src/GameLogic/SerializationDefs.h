#pragma once

#include <bitset>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/export.hpp>

#include "Math/matrix.h"


// attrib serialization
// attrib:		the attribute of the object
// minPriority:	the minimum priority - objects over this priority must be serialized without float compression

#define SER_P(attrib)						if ((*attribMaskPtr)[(*attribIndexPtr)++])	ar & attrib;

#define SER_P_F(attrib, minPriority)		if (networkPriority >= minPriority)			SER_P(attrib);													\
											else										serializeF32_F16(ar, attrib, attribMask, attribIndex);

#define SER_P_M_F16(attrib, minPriority)	if (networkPriority >= minPriority)			SER_P(attrib);													\
											else										serializeMatrix_F16(ar, attrib, attribMask, attribIndex);

#define SER_P_VEC2(attrib, minPriority)		serializeVec2(ar, attrib, *attribMaskPtr, *attribIndexPtr, networkPriority >= minPriority);
#define SER_P_VEC3(attrib, minPriority)		serializeVec3(ar, attrib, *attribMaskPtr, *attribIndexPtr, networkPriority >= minPriority);


#define SERIALIZABLE_CLASS					private:																		\
											friend class boost::serialization::access;										\
											template <typename Archive>														\
											void serialize(Archive& ar, const uint version);

// declarations for classes with asymmetric serialization (save/load)
#define SERIALIZABLE_CLASS_SEPARATED		SERIALIZABLE_CLASS																\
											template <typename Archive>														\
											void load(Archive& ar, const uint version);										\
											template <typename Archive>														\
											void save(Archive& ar, const uint version) const;

#define SERIALIZABLE(T)						template void T::serialize(boost::archive::binary_oarchive&, const uint);		\
											template void T::serialize(boost::archive::binary_iarchive&, const uint);		\
											template void T::serialize(boost::archive::text_oarchive&, const uint);			\
											template void T::serialize(boost::archive::text_iarchive&, const uint);			\
																															\
											BOOST_CLASS_EXPORT_IMPLEMENT(T);


typedef uint64_t attribMaskIntType;
static const uint8_t ATTRIB_NUM	= sizeof(attribMaskIntType) * 8;

enum class NetworkPriority
{
	LOW = 0,
	MEDIUM,
	HIGH,				//
	TOP,				// eg. PLAYER: no float compression, max frequency
};


struct Serializable
{
	uint8_t m_id;
	uint8_t m_attribIndex;
	std::bitset<ATTRIB_NUM> m_attribMask;

	NetworkPriority networkPriority;

	Serializable(uint8_t id = 0, uint8_t attribIndex = 0, NetworkPriority networkPriority = NetworkPriority::MEDIUM)
		: m_id(id)
		, m_attribIndex(attribIndex)
		, networkPriority(networkPriority)
	{

	}
};


// float compression
template <typename Archive>
void serializeF32_F16(Archive& ar, float& attrib, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex)
{
	if (attribMask[attribIndex++])
	{
		if (!attribMask[0])
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
void serializeVec2(Archive& ar, vec2& v, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16)
{
	if (attribMask[attribIndex++])
	{
		if (!attribMask[0])
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
void serializeVec3(Archive& ar, vec3& v, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16)
{
	if (attribmask[attribIndex++])
	{
		if (!attribMask[0])
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
void serializeVec3_F16(Archive& ar, vec3& v, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex)
{
	if (attribMask[attribIndex++])
	{
		if (!attribMask[0])
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
void serializeMatrix_F16(Archive& ar, Matrix& mat, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex)
{
	if (attribMask[attribIndex++])
	{
		if (!attribMask[0])
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
