
#ifndef MESH_H
#define MESH_H

#include "Models/mesh/Object.h"

/**
 * @brief Namespace of the meshes and models.
 */
namespace models
{

class Mesh
{
public:
	Mesh();
	Mesh(const Mesh& other);
	virtual ~Mesh();

	void addObject(Object* object);
	void listObjects();

	virtual void animate(const float dt);


	virtual void preRender(const GLuint program, const bool bindVbos = true);
	virtual void postRender() const;

	virtual void render();
	virtual void renderSubset(const uint subset);


	virtual bool isMesh() {	return true; }

	Mesh& operator=(const Mesh& other);


	// getters-setters
	uint	getNumObjects() const;
	uint	getNumVertices() const;
	uint	getNumPolygons() const;
	uint	getObjIndexByName(const std::string& objname) const;
	Object*	getObject(const uint index) const;

protected:
	uint					m_numObjects;
	std::vector<Object*>	m_objects;


	GLuint					m_verticesVboId;
	GLuint					m_indicesVboId;

	GLuint					m_lastShaderProg;
	GLint					m_posLoc;
	GLint					m_normLoc;
	GLint					m_tangentLoc;
	GLint					m_bitangentLoc;
	GLint					m_texcoordLoc;
	GLint					m_lightMapTexcoordLoc;

#ifdef GX_DEBUG_INFO
	std::string				m_debug_meshFileName;
#endif
};

bool calculateTangent(const vec3& v1, const vec3& v2, const vec3& v3, const texCoord& tc1, const texCoord& tc2, const texCoord& tc3, vec3& tangent, vec3& bitangent);
bool calculateTangent(const Vertex& v1, const Vertex& v2, const Vertex& v3, vec3& tangent, vec3& bitangent);

} // namespace models

#endif	// MESH_H
