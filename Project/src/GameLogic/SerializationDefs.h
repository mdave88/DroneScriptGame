#pragma once

#include <bitset>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/export.hpp>

#include "Math/matrix.h"


#define SERIALIZE_I(idx0, name, attribIndex)	public: \
												const decltype(name)& get_##name##() const { return name; } \
												void set_##name##(const decltype(name)& newval) { name = newval; attribMask[attribIndex + idx0 + 2] = true; }

#define SERIALIZE(name, attribIndex)			SERIALIZE_I(0, name, attribIndex)


#ifdef WIN32
// Workaroud for bug in VC compiler
// https://connect.microsoft.com/VisualStudio/feedback/details/380090/variadic-macro-replacement

#define SERIALIZE1(X)							SERIALIZE(X, 0)
#define SERIALIZE2(X, _1)						SERIALIZE(X, 1) SERIALIZE1(_1)
#define SERIALIZE3(X, _2, _1)					SERIALIZE(X, 2) SERIALIZE2(_2, _1)
#define SERIALIZE4(X, _3, _2, _1)				SERIALIZE(X, 3) SERIALIZE3(_3, _2, _1)

#define SERIALIZE1_I(idx0, X)					SERIALIZE_I(idx0, X, 0)
#define SERIALIZE2_I(idx0, X, _1)				SERIALIZE_I(idx0, X, 1) SERIALIZE1_I(idx0, _1)
#define SERIALIZE3_I(idx0, X, _2, _1)			SERIALIZE_I(idx0, X, 2) SERIALIZE2_I(idx0, _2, _1)
#define SERIALIZE4_I(idx0, X, _3, _2, _1)		SERIALIZE_I(idx0, X, 3) SERIALIZE3_I(idx0, _3, _2, _1)
#else
#define SERIALIZE1(X)							SERIALIZE(X, 0)
#define SERIALIZE2(X, ...)						SERIALIZE(X, 1) SERIALIZE1(__VA_ARGS__)
#define SERIALIZE3(X, ...)						SERIALIZE(X, 2) SERIALIZE2(__VA_ARGS__)
#define SERIALIZE4(X, ...)						SERIALIZE(X, 3) SERIALIZE3(__VA_ARGS__)

#define SERIALIZE1_I(idx0, X)					SERIALIZE_I(idx0, X, 0)
#define SERIALIZE2_I(idx0, X, ...)				SERIALIZE_I(idx0, X, 1) SERIALIZE1_I(idx0, __VA_ARGS__)
#define SERIALIZE3_I(idx0, X, ...)				SERIALIZE_I(idx0, X, 2) SERIALIZE2_I(idx0, __VA_ARGS__)
#define SERIALIZE4_I(idx0, X, ...)				SERIALIZE_I(idx0, X, 3) SERIALIZE3_I(idx0, __VA_ARGS__)
#endif

// attrib serialization
// attrib:		the attribute of the object
// minPriority:	the minimum priority - objects over this priority must be serialized without float compression

#define SER_P(attrib)							if (attribMask[attribIndex++])		ar& attrib;
#define SER_P_CONST(attrib)						if (attribMask[attribIndex++])		ar& const_cast<decltype(attrib)>(attrib);

#define SER_P_F(attrib, minPriority)			serializeFloat(ar, attrib, attribMask, attribIndex, networkPriority >= minPriority);
#define SER_P_VEC3(attrib, minPriority)			serializeVec3(ar, attrib, attribMask, attribIndex, networkPriority >= minPriority);
#define SER_P_VEC2(attrib, minPriority)			serializeVec2(ar, attrib, attribMask, attribIndex, networkPriority >= minPriority);
#define SER_P_MAT(attrib, minPriority)			serializeMatrix(ar, attrib, attribMask, attribIndex, networkPriority >= minPriority);


#define SERIALIZABLE_CLASS						private:																		\
												friend class boost::serialization::access;										\
												template <typename Archive>														\
												void serialize(Archive& ar, const uint version);

// declarations for classes with asymmetric	serialization (save/load)
#define SERIALIZABLE_CLASS_SEPARATED			SERIALIZABLE_CLASS																\
												template <typename Archive>														\
												void load(Archive& ar, const uint version);										\
												template <typename Archive>														\
												void save(Archive& ar, const uint version) const;

#define SERIALIZABLE(T)							template void T::serialize(boost::archive::binary_oarchive&, const uint);		\
												template void T::serialize(boost::archive::binary_iarchive&, const uint);		\
												template void T::serialize(boost::archive::text_oarchive&, const uint);			\
												template void T::serialize(boost::archive::text_iarchive&, const uint);			\
																																\
												BOOST_CLASS_EXPORT_IMPLEMENT(T);


typedef uint8_t attribMaskIntType;
static const uint8_t ATTRIB_NUM	= sizeof(attribMaskIntType) * 8;

enum class NetworkPriority
{
	LOW = 0,
	MEDIUM,
	HIGH,				//
	TOP,				// eg. PLAYER: no float compression, max frequency
};


class Serializable
{
public:
	Serializable(NetworkPriority networkPriority = NetworkPriority::MEDIUM, uint8_t id = 0, uint8_t attribIndex = 0)
		: m_id(id)
		, attribIndex(attribIndex)
		, networkPriority(networkPriority)
	{
	}

	virtual void printInfo() const { }

protected:
	template <typename Archive>
	void serializeFields(Archive& ar) {}

	

	template <typename Archive, typename T, typename... Args>
	void serializeFields(Archive& ar, T& field, Args... args)
	{
		SER_P(field);
		serializeFields(ar, args...);
	}

	template <typename Archive, typename T, typename... Args>
	void serializeFields(Archive& ar, const T& field, Args... args)
	{
		SER_P_CONST(field);
		serializeFields(ar, args...);
	}

	template <typename Archive, typename... Args>
	void serializeFields(Archive& ar, float& field, NetworkPriority priority, Args... args)
	{
		SER_P_F(field, priority);
		serializeFields(ar, args...);
	}

	template <typename Archive, typename... Args>
	void serializeFields(Archive& ar, vec2& field, NetworkPriority priority, Args... args)
	{
		SER_P_VEC2(field, priority);
		serializeFields(ar, args...);
	}

private:
	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive& ar, const uint version) const
	{
		attribMaskIntType attribMaskInt = attribMask.to_ulong();
		ar << attribMaskInt;
	}

	template<class Archive>
	void load(Archive& ar, const uint version)
	{
		attribMaskIntType attribMaskInt;
		ar >> attribMaskInt;
		attribMask = std::bitset<ATTRIB_NUM>(attribMaskInt);
		attribMask.set(0);
	}

	template <typename Archive>
	void serialize(Archive& ar, const uint version)
	{
		// reset a direction marking bit
		attribMask.reset(0);
		attribIndex = 1;

		// mark the direction on the first bit on the attrib mask: 0 - save, 1 - load
		boost::serialization::split_member(ar, *this, version);

		SER_P(networkPriority);	// network priority can change on the fly -> compressions can vary
	}

protected:
	uint8_t m_id;
	uint8_t attribIndex;
	std::bitset<ATTRIB_NUM> attribMask;

	NetworkPriority networkPriority;
};


// float compression
template <typename Archive>
void serializeFloat(Archive& ar, float& attrib, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16);

template <typename Archive>
void serializeVec2(Archive& ar, vec2& v, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16);

template <typename Archive>
void serializeVec3(Archive& ar, vec3& v, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16);

template <typename Archive>
void serializeMatrix(Archive& ar, Matrix& mat, std::bitset<ATTRIB_NUM>& attribMask, uint8_t& attribIndex, bool useF16);
