#include "GameStdAfx.h"
#include "Object.h"

namespace models
{

Object::Object()
	:	m_numVertices(0),
	    m_numTriangles(0),

	    m_pVertices(nullptr),
	    m_pNormals(nullptr),
	    m_pTriangles(nullptr),
	    m_pTexcoords(nullptr)
{
}

Object& Object::operator=(const Object& o)
{
	if (this != &o)
	{
		m_name = o.m_name;
		m_numVertices = o.m_numVertices;
		m_numTriangles = o.m_numTriangles;

		SAFEDEL2(m_pVertices);
		SAFEDEL2(m_pTriangles);
		SAFEDEL2(m_pTexcoords);

		m_pVertices = new vec3[m_numVertices];
		m_pTriangles = new poly3[m_numTriangles];
		m_pTexcoords = new texCoord[m_numVertices];

		for (uint i = 0; i < m_numVertices; i++)
		{
			m_pNormals[i] = o.m_pNormals[i];
			m_pVertices[i] = o.m_pVertices[i];
			m_pTexcoords[i] = o.m_pTexcoords[i];
		}

		for (uint i = 0; i < m_numTriangles; i++)
		{
			m_pTriangles[i] = o.m_pTriangles[i];
		}

		m_decalMap = o.m_decalMap;
		m_normalMap = o.m_normalMap;
		m_heightMap = o.m_heightMap;
		m_normalHeightMap = o.m_normalHeightMap;
	}

	return *this;
}

Object::~Object()
{
	SAFEDEL2(m_pVertices);
	SAFEDEL2(m_pNormals);
	SAFEDEL2(m_pTriangles);
	SAFEDEL2(m_pTexcoords);
}

} // namespace models
