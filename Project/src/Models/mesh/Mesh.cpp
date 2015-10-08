#include "GameStdAfx.h"
#include "Models/mesh/Mesh.h"
#include "Common/LoggerSystem.h"


namespace models
{

Mesh::Mesh()
	: m_numObjects(0)
	, m_verticesVboId(0)
	, m_indicesVboId(0)
	, m_lastShaderProg(UINT32_MAX)
{
}

Mesh::Mesh(const Mesh& other)
	: Mesh()
{
}

Mesh::~Mesh()
{
	//MQLOG("dest Mesh", 0);

	for (Object* object : m_objects)
	{
		delete object;
	}
}

void Mesh::addObject(Object* object)
{
	m_numObjects++;
	m_objects.push_back(object);
}

void Mesh::listObjects()
{
	for (uint i = 0; i < m_numObjects; i++)
	{
		TRACE_INFO(m_objects[i]->m_name, 0);
	}
}

void Mesh::animate(const float dt)
{
}

void Mesh::render()
{
	GLint v_i = 0;
	for (const Object* object : m_objects)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, object->m_decalMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, object->m_normalHeightMap);

		glDrawArrays(GL_TRIANGLES, v_i, object->m_numTriangles * 3);
		v_i += object->m_numTriangles * 3;
	}
}

void Mesh::renderSubset(const uint subset)
{
	// drawing
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_objects[subset]->m_decalMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_objects[subset]->m_normalHeightMap);

	GLint v_i = 0;
	uint i = 0;
	std::vector<Object*>::iterator it = m_objects.begin();
	while (i < subset)
	{
		v_i += (*it)->m_numTriangles * 3;
		i++;
		++it;
	}

	glDrawArrays(GL_TRIANGLES, v_i, m_objects[subset]->m_numTriangles * 3);
}

Mesh& Mesh::operator=(const Mesh& other)
{
	if (this != &other)
	{
		m_numObjects = other.m_numObjects;
		m_objects = other.m_objects;

		m_indicesVboId	= other.m_indicesVboId;
		m_verticesVboId	= other.m_verticesVboId;
	}
	return *this;
}

GLuint vec3Offset  = 3 * sizeof(GLfloat);
GLsizei vertStride = sizeof(Vertex);


/**
 * Initialize the vertex attributes, bind textures and buffers.
 *
 * @param program	the openGL shader program used for the rendering.
 * @param bindVbos	if true bind the available VBO buffers. True by default.
 */
void Mesh::preRender(const GLuint program, const bool bindVbos)
{
	if (bindVbos)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_verticesVboId);

		if (m_indicesVboId)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVboId);
		}
	}

	if (m_lastShaderProg != program)
	{
		m_posLoc = glGetAttribLocation(program, "a_posL");
		m_normLoc = glGetAttribLocation(program, "a_normalL");
		m_tangentLoc = glGetAttribLocation(program, "a_tangentL");
		m_bitangentLoc = glGetAttribLocation(program, "a_bitangentL");
		m_texcoordLoc = glGetAttribLocation(program, "a_texcoordL");
		m_lightMapTexcoordLoc = glGetAttribLocation(program, "a_lightMapTexcoordL");

		m_lastShaderProg = program;
	}

	//enable vertex attrib arrays

	glEnableVertexAttribArray(m_posLoc);
	glVertexAttribPointer(m_posLoc,						3, GL_FLOAT, GL_FALSE, vertStride, (const void*)0);

	if (m_normLoc != -1)
	{
		glEnableVertexAttribArray(m_normLoc);
		glVertexAttribPointer(m_normLoc,				3, GL_FLOAT, GL_FALSE, vertStride, (const void*)vec3Offset);
	}
	if (m_tangentLoc != -1)
	{
		glEnableVertexAttribArray(m_tangentLoc);
		glVertexAttribPointer(m_tangentLoc,				3, GL_FLOAT, GL_FALSE, vertStride, (const void*)(vec3Offset * 2));
	}
	if (m_bitangentLoc != -1)
	{
		glEnableVertexAttribArray(m_bitangentLoc);
		glVertexAttribPointer(m_bitangentLoc,			3, GL_FLOAT, GL_FALSE, vertStride, (const void*)(vec3Offset * 3));
	}
	if (m_texcoordLoc != -1)
	{
		glEnableVertexAttribArray(m_texcoordLoc);
		glVertexAttribPointer(m_texcoordLoc,			2, GL_FLOAT, GL_FALSE, vertStride, (const void*)(vec3Offset * 4));
	}
	if (m_lightMapTexcoordLoc != -1)
	{
		glEnableVertexAttribArray(m_lightMapTexcoordLoc);
		glVertexAttribPointer(m_lightMapTexcoordLoc,	2, GL_FLOAT, GL_FALSE, vertStride, (const void*)(vec3Offset * 4 + 2 * sizeof(float)));
	}
}

/**
 * Deinitialize the vertex attributes, unbind textures and buffers.
 */
void Mesh::postRender() const
{
	glDisableVertexAttribArray(m_posLoc);
	if (m_normLoc != -1)
	{
		glDisableVertexAttribArray(m_normLoc);
	}
	if (m_tangentLoc != -1)
	{
		glDisableVertexAttribArray(m_tangentLoc);
	}
	if (m_bitangentLoc != -1)
	{
		glDisableVertexAttribArray(m_bitangentLoc);
	}
	if (m_texcoordLoc != -1)
	{
		glDisableVertexAttribArray(m_texcoordLoc);
	}
	if (m_lightMapTexcoordLoc != -1)
	{
		glDisableVertexAttribArray(m_lightMapTexcoordLoc);
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_indicesVboId)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

// getters-setters
uint Mesh::getNumObjects() const
{
	return m_numObjects;
}

uint Mesh::getNumVertices() const
{
	uint num = 0;
	for (uint i = 0; i < m_numObjects; i++)
	{
		num += m_objects[i]->m_numVertices;
	}

	return num;
}

uint Mesh::getNumPolygons() const
{
	uint num = 0;
	for (uint i = 0; i < m_numObjects; i++)
	{
		num += m_objects[i]->m_numTriangles;
	}

	return num;
}

uint Mesh::getObjIndexByName(const std::string& objname) const
{
	for (uint i = 0; i < m_numObjects; i++)
	{
		if (m_objects[i]->m_name == objname)
		{
			return i;
		}
	}

	return 0;
}

Object*	Mesh::getObject(const uint index) const
{
	return m_objects[index];
}

bool calculateTangent(const vec3& v1, const vec3& v2, const vec3& v3, const texCoord& tc1, const texCoord& tc2, const texCoord& tc3, vec3& tangent, vec3& bitangent)
{
	const float x1 = v2.x - v1.x;
	const float x2 = v3.x - v1.x;
	const float y1 = v2.y - v1.y;
	const float y2 = v3.y - v1.y;
	const float z1 = v2.z - v1.z;
	const float z2 = v3.z - v1.z;

	const float s1 = tc2.u - tc1.u;
	const float s2 = tc3.u - tc1.u;
	const float t1 = tc2.v - tc1.v;
	const float t2 = tc3.v - tc1.v;

	const float div = s1 * t2 - s2 * t1;
	const float r = ((div == 0.0f) ? 0.0f : 1.0f / div);

	tangent = vec3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
	bitangent = vec3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

	tangent = normalize(tangent);
	bitangent = normalize(bitangent);

	return (div != 0.0f);
}

bool calculateTangent(const Vertex& v1, const Vertex& v2, const Vertex& v3, vec3& tangent, vec3& bitangent)
{
	return calculateTangent(v1.pos, v2.pos, v3.pos, v1.texcoord, v2.texcoord, v3.texcoord, tangent, bitangent);
}

} // namespace models
