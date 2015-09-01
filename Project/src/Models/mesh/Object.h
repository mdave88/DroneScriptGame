
#ifndef OBJECT_H
#define OBJECT_H

#include "Models/mesh/PolyTex.h"

namespace models
{


struct Vertex
{
	vec3		pos;
	vec3		normal;
	vec3		tangent;
	vec3		bitangent;
	texCoord	texcoord;
	texCoord	lightmapCoord;

	Vertex(vec3 pos = vec3(0.0f), vec3 normal = vec3(0.0f), vec3 tangent = vec3(0.0f), vec3 bitangent = vec3(0.0f), texCoord texcoord = texCoord(0.0f, 0.0f), texCoord lightmapCoord = texCoord(0.0f, 0.0f))
		:	pos(pos),
		    normal(normal),
		    tangent(tangent),
		    bitangent(bitangent),
		    texcoord(texcoord),
		    lightmapCoord(lightmapCoord)
	{
	}

	Vertex operator+(const Vertex& other) const
	{
		Vertex result;
		result.pos				= pos + other.pos;
		result.normal			= normal + other.normal;
		result.texcoord.u		= texcoord.u + other.texcoord.u;
		result.texcoord.v		= texcoord.v + other.texcoord.v;
		result.lightmapCoord.u	= lightmapCoord.u + other.lightmapCoord.u;
		result.lightmapCoord.v	= lightmapCoord.v + other.lightmapCoord.v;

		return result;
	}

	Vertex operator*(const float value) const
	{
		Vertex result;
		result.pos				= pos * value;
		result.normal			= normal * value;
		result.texcoord.u		= texcoord.u * value;
		result.texcoord.v		= texcoord.v * value;
		result.lightmapCoord.u	= lightmapCoord.u * value;
		result.lightmapCoord.v	= lightmapCoord.v * value;

		return result;
	}
};


class Object
{
public:
	Object();
	virtual ~Object();

	Object& operator=(const Object& o);


public:
	std::string m_name;

	uint		m_numVertices;
	uint		m_numTriangles;

	GLuint		m_decalMap;
	GLuint		m_normalMap;
	GLuint		m_heightMap;
	GLuint		m_normalHeightMap;

	// TODO Vertex* m_pVertexArray;
	vec3*		m_pVertices;
	vec3*		m_pNormals;
	poly3*		m_pTriangles;
	texCoord*	m_pTexcoords;
};

} // namespace models

#endif // OBJECT_H
