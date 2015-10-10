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

#define SER_P_F16(attrib, minPriority)		if (m_networkPriority >= minPriority)		SER_P(attrib);													\
											else										serializeF32_F16(ar, attrib, attribMask, attribIndex);

#define SER_P_M_F16(attrib, minPriority)	if (m_networkPriority >= minPriority)		SER_P(attrib);													\
											else										serializeMatrix_F16(ar, attrib, attribMask, attribIndex);

#define SER_P_VEC2(attrib, minPriority)		serializeVec2(ar, attrib, *attribMaskPtr, *attribIndexPtr, m_networkPriority >= minPriority);
#define SER_P_VEC3(attrib, minPriority)		serializeVec3(ar, attrib, *attribMaskPtr, *attribIndexPtr, m_networkPriority >= minPriority);

// attrib updating
#define SERIALIZABLE(T)						template void T::serialize(boost::archive::binary_oarchive&, unsigned);		\
											template void T::serialize(boost::archive::binary_iarchive&, unsigned);		\
											template void T::serialize(boost::archive::text_oarchive&, unsigned);		\
											template void T::serialize(boost::archive::text_iarchive&, unsigned);		\
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

	NetworkPriority m_networkPriority;

	Serializable(uint8_t id = 0, uint8_t attribIndex = 0, NetworkPriority networkPriority = NetworkPriority::MEDIUM)
		: m_id(id)
		, m_attribIndex(attribIndex)
		, m_networkPriority(networkPriority)
	{

	}
};

class PersistentComponent
{
public:
	PersistentComponent(NetworkPriority networkPriority = NetworkPriority::MEDIUM)
		: attribMaskPtr(nullptr)
		, attribIndexPtr(nullptr)
		, m_networkPriority(networkPriority)
	{
	}

	void setAttribMask(std::bitset<64>* _attribMask, uint8_t* _attribIndex)
	{
		attribMaskPtr = _attribMask;
		attribIndexPtr = _attribIndex;
	}


	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		SER_P(m_networkPriority);	// network priority can change on the fly -> compressions can vary
	}

protected:
	template <typename Archive>
	void serializePrimitiveFields(Archive& ar) {}

	template <typename Archive, typename T, typename... Args>
	void serializePrimitiveFields(Archive& ar, T& field, Args... args)
	{
		SER_P(field);
		serializePrimitiveFields(ar, args...);
	}

	template <typename Archive>
	void serializeVec2Fields(Archive& ar) {}

	template <typename Archive, typename... Args>
	void serializeVec2Fields(Archive& ar, vec2& field, NetworkPriority priority, Args... args)
	{
		SER_P_VEC2(field, priority);
		serializeVec2Fields(ar, args...);
	}

protected:
	uint8_t* attribIndexPtr;
	std::bitset<ATTRIB_NUM>* attribMaskPtr;
	NetworkPriority m_networkPriority;
};


// float compression
template <typename Archive>
void serializeF32_F16(Archive& ar, float& attrib, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex)
{
	if (attribMask[attribIndex++])
	{
		if (!attribMask[7])
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
		if (!attribMask[7])
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
		if (!attribmask[7])
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
		if (!attribMask[7])
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
		if (!attribMask[7])
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
