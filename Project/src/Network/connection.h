#pragma once

#define NOMINMAX

#include <stdint.h>
#include <iostream>

#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>


//#define BINARY_MARSHAL

#ifdef BINARY_MARSHAL
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#else
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#endif

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <iomanip>
#include <string>
#include <sstream>


#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>


#include <enet/enet.h>

#include "Common/LoggerSystem.h"


namespace network
{

/**
* Compresses the string with zlib compressor.
*
* @param data The string to be compressed.
*/
inline std::string compress1(const std::string& data)
{
	std::string compressed;
	boost::iostreams::filtering_streambuf<boost::iostreams::output> out;
	out.push(boost::iostreams::zlib_compressor());
	out.push(boost::iostreams::back_inserter(compressed));
	boost::iostreams::copy(boost::make_iterator_range(data), out);

	return compressed;
}

/**
* Decompresses the std::string with zlib compressor.
*
* @param data The compressed data to be decompressed to a std::string.
*/
inline std::string decompress1(const std::string& data)
{
	boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
	in.push(boost::iostreams::zlib_decompressor());
	in.push(boost::make_iterator_range(data));
	std::string decompressed;
	boost::iostreams::copy(in, boost::iostreams::back_inserter(decompressed));

	return decompressed;
}


//#ifdef BINARY_MARSHAL
/**
* Marshals the object into binary format.
*
* @param t The object to be marshalled.
* @param compressionScheme Selects the compression scheme.
*/
template <typename T>
std::string marshalBinary(T& t, short compressionScheme = 0)
{
	std::string serialStr;

	try
	{
		boost::iostreams::back_insert_device<std::string> inserter(serialStr);
		boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > stream(inserter);
		boost::archive::binary_oarchive oa(stream, boost::archive::no_header);

		oa << t;

		stream.flush();
	}
	catch (boost::archive::archive_exception ex)
	{
		TRACE_ERROR("Error: archive exception: " << ex.what() << std::endl, 0);
		throw ex;
	}
	catch (std::exception ex2)
	{
		TRACE_ERROR("Error: stl exception: " << ex2.what() << std::endl, 0);
	}
	catch (...)
	{
		TRACE_ERROR("Error: unknown exception: " << std::endl, 0);
		return "";
	}


	// compress data
	if (compressionScheme == 1)
	{
		return compress1(serialStr);
	}

	return serialStr;
}

/**
* Unmarshals the object from binary format.
*
* @param t The object to be unmarshalled from the std::string.
* @param serialStr0 The serialized form of the marshalled object.
* @param decompressionScheme Selects the decompression scheme.
*/
template <typename T>
bool unmarshalBinary(T& t, const std::string& serialStr0, short decompressionScheme = 0)
{

	std::string serialStr = serialStr0;

	// decompress data
	if (decompressionScheme == 1)
	{
		serialStr = decompress1(serialStr);
	}


	try
	{
		// wrap buffer inside a stream and deserialize serialStr into obj
		boost::iostreams::basic_array_source<char> device(serialStr.data(), serialStr.size());
		boost::iostreams::stream<boost::iostreams::basic_array_source<char> > stream(device);
		boost::archive::binary_iarchive ia(stream, boost::archive::no_header);
		ia >> t;
	}
	catch (boost::archive::archive_exception str)
	{
		TRACE_ERROR("Error: archive exception: " << str.what() << std::endl, 0);
		return false;
	}
	catch (std::exception ex2)
	{
		TRACE_ERROR("Error: stl exception: " << ex2.what() << std::endl, 0);
		return false;
	}
	catch (...)
	{
		TRACE_ERROR("Error: unknown exception: " << std::endl, 0);
		return false;
	}

	return true;
}

/**
* Marshals the packet into binary format and sends it using the peer.
*
* @param t The object to be sent.
* @param peer The peer.
* @param compressionScheme Selects the compression scheme.
*/
template <typename T>
void sendBinary(T& t, ENetPeer* peer, short compressionScheme = 0)
{
	std::string serialStr = marshalBinary(t, compressionScheme);
	ENetPacket* packet = enet_packet_create(serialStr.c_str(), serialStr.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

//#else

/**
* Marshals the object into text format.
*
* @param t The object to be marshalled.
* @param compressionScheme Selects the compression scheme.
*/
template <typename T>
std::string marshalText(T& t, short compressionScheme = 0)
{
	std::string serialStr;

	try
	{
		std::ostringstream archive_stream;
		boost::archive::text_oarchive archive(archive_stream, boost::archive::no_header);
		archive << t;
		serialStr = archive_stream.str();
	}
	catch (boost::archive::archive_exception ex)
	{
		TRACE_ERROR("Error: archive exception: " << ex.what() << std::endl, 0);
		throw ex;
	}

	// compress data
	if (compressionScheme == 1)
	{
		return compress1(serialStr);
	}

	return serialStr;
}

/**
* Unmarshals the object from text format.
*
* @param t						The object to be unmarshalled from the std::string.
* @param serialStr0				The serialized form of the marshalled object.
* @param decompressionScheme	Selects the decompression scheme.
*/
template <typename T>
bool unmarshalText(T& t, const std::string& serialStr0, short decompressionScheme = 0)
{
	std::string serialStr = serialStr0;

	// decompress data
	if (decompressionScheme == 1)
	{
		serialStr = decompress1(serialStr);
	}

	try
	{
		std::istringstream archive_stream(serialStr);
		boost::archive::text_iarchive archive(archive_stream, boost::archive::no_header);
		archive >> t;
	}
	catch (boost::archive::archive_exception str)
	{
		TRACE_ERROR("Error: archive exception: " << str.what() << std::endl, 0);
		return false;
	}
	catch (std::exception ex2)
	{
		TRACE_ERROR("Error: stl exception: " << ex2.what() << std::endl, 0);
		return false;
	}
	catch (...)
	{
		TRACE_ERROR("Error: unknown exception: " << std::endl, 0);
		return false;
	}

	return true;
}

/**
* Marshals the packet into text format and send it using the peer.
*
* @param t					The object to be sent.
* @param peer				The peer.
* @param compressionScheme	Selects the compression scheme.
*/
template <typename T>
void sendText(T& t, ENetPeer* peer, short compressionScheme = 0)
{
	std::string serialStr = marshalText(t, compressionScheme);
	ENetPacket* packet = enet_packet_create(serialStr.c_str(), serialStr.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

//#endif


/**
* Marshals the packet to a std::string form.
*
* @param t					The object to be marshalled.
* @param compressionScheme	Selects the compression scheme.
*/
template <typename T>
std::string marshal(T& t, short compressionScheme = 1)
{
#ifdef BINARY_MARSHAL
	return marshalBinary(t, compressionScheme);
#else
	return marshalText(t, compressionScheme);
#endif
}

/**
* Unmarshals the packet.
*
* @param t						The object to be unmarshalled from the std::string.
* @param serialStr				The serialized form of the marshalled object.
* @param decompressionScheme	Selects the decompression scheme.
*/
template <typename T>
bool unmarshal(T& t, const std::string& serialStr, short decompressionScheme = 1)
{
#ifdef BINARY_MARSHAL
	return unmarshalBinary(t, serialStr, decompressionScheme);
#else
	return unmarshalText(t, serialStr, decompressionScheme);
#endif
}

/**
* Marshals the packet and send it using the peer.
*
* @param t					The object to be sent.
* @param peer				The peer.
* @param compressionScheme	Selects the compression scheme.
*/
template <typename T>
void send(T& t, ENetPeer* peer, short compressionScheme = 1)
{
#ifdef BINARY_MARSHAL
	sendBinary(t, peer, compressionScheme);
#else
	sendText(t, peer, compressionScheme);
#endif
}

} // namespace network
