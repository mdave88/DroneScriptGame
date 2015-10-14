#include "GameStdAfx.h"
#include "GameLogic/SerializationDefs.h"


#include <bitset>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/export.hpp>

#include "Math/matrix.h"


// float compression
template <typename Archive>
void serializeFloat(Archive& ar, float& attrib, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16)
{
	if (attribMask[attribIndex++])
	{
		if (!attribMask[0])
		{
			// save
			if (useF16)
			{
				short val = utils::float32Tofloat16(attrib);
				ar& val;
			}
			else
			{
				ar& attrib;
			}
		}
		else
		{
			// load
			if (useF16)
			{
				short val;
				ar& val;
				attrib = utils::float16Tofloat32(val);
			}
			else
			{
				ar& attrib;
			}
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
			// load
			if (useF16)
			{
				short x, y;
				ar& x & y;
				v = utils::vec2F16_to_vec2F32(x, y);
			}
			else
			{
				ar& v.x & v.y;
			}
		}
	}
}

template <typename Archive>
void serializeVec3(Archive& ar, vec3& v, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16)
{
	if (attribMask[attribIndex++])
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
void serializeMatrix(Archive& ar, Matrix& mat, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16)
{
	if (attribMask[attribIndex++])
	{
		if (!attribMask[0])
		{
			// save
			for (int i = 0; i < 16; i++)
			{
				if (useF16)
				{
					short val = utils::float32Tofloat16(mat[i]);
					ar& val;
				}
				else
				{
					float val = mat[i];
					ar& val;
				}
			}
		}
		else
		{
			// load
			float decompressedMat[16];
			for (int i = 0; i < 16; i++)
			{
				if (useF16)
				{
					short val;
					ar& val;
					decompressedMat[i] = utils::float16Tofloat32(val);
				}
				else
				{
					ar& decompressedMat[i];
				}
			}
			mat.set(decompressedMat);
		}
	}
}

template void serializeFloat(boost::archive::binary_oarchive&, float& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeFloat(boost::archive::binary_iarchive&, float& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeFloat(boost::archive::text_oarchive&, float& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeFloat(boost::archive::text_iarchive&, float& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);

template void serializeVec2(boost::archive::binary_oarchive&, vec2& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeVec2(boost::archive::binary_iarchive&, vec2& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeVec2(boost::archive::text_oarchive&, vec2& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeVec2(boost::archive::text_iarchive&, vec2& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);

template void serializeVec3(boost::archive::binary_oarchive&, vec3& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeVec3(boost::archive::binary_iarchive&, vec3& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeVec3(boost::archive::text_oarchive&, vec3& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeVec3(boost::archive::text_iarchive&, vec3& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);

template void serializeMatrix(boost::archive::binary_oarchive&, Matrix& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeMatrix(boost::archive::binary_iarchive&, Matrix& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeMatrix(boost::archive::text_oarchive&, Matrix& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
template void serializeMatrix(boost::archive::text_iarchive&, Matrix& attrib, std::bitset<ATTRIB_NUM>&, uint8_t&, bool useF16);
