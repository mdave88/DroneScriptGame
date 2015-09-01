
#ifndef MODELMD2_H
#define MODELMD2_H

#include <imageLoad.h>

#include "Models/mesh/AnimatedMesh.h"

namespace models
{

class ModelMd2 : public AnimatedMesh
{
// member structure declarations
protected:
	struct Header;

	struct VertexMd2;
	struct TriangleMd2;
	struct TexcoordMd2;
	struct TexcoordMd2f;

	struct FrameMd2;
	struct AnimMd2;


public:
	static bool load(const char* meshFile, MeshDirectory& meshDirectory, TextureDirectory& textureDirectory, const char* name, const bool justData = false);

	ModelMd2();
	~ModelMd2();

	ModelMd2(const ModelMd2& other);
	ModelMd2& operator=(const ModelMd2& other);

	void copy(const ModelMd2& other);

	bool load(const char* filename, TextureDirectory& textureDirectory, const bool justData = false);


	virtual void animate(const float dt);

	virtual void render();
	virtual void renderSubset(const uint subset);


	// getters-setters
	virtual float getFrame() const;
	virtual void setFrame(float time);

	virtual float getFrameTime() const;
	virtual void setFrameTime(float time);


protected:
	GLuint									m_decalMap;
	GLuint									m_normalMap;
	GLuint									m_heightMap;
	GLuint									m_normalHeightMap;

	int										m_numTriangles;
	TriangleMd2*							m_pTriangles;

	int										m_numTexCoords;
	texCoord*								m_pTexcoords;

	int										m_numVertices;
	vec3*									m_pVertices;
	vec3*									m_pNormals;
	vec3*									m_pTangents;

	vec3*									m_pVertexArray;
	vec3*									m_pNormalArray;
	vec3*									m_pTangentArray;
	texCoord*								m_pTexcoordArray;
	poly3*									m_pTriangleArray;


	// animations
	int										m_numAnims;
	typedef std::map<const std::string, AnimMd2> AnimMd2Directory;

	AnimMd2Directory						m_animations;
	float									m_time;


// member structure definitions
protected:
	struct Header
	{
		int magic;
		int version;
		int skinWidth;
		int skinHeight;
		int frameSize;
		int numSkins;
		int numVertices;
		int numTexCoords;
		int numTriangles;
		int numGlCommands;
		int numFrames;
		int offsetSkins;
		int offsetTexCoords;
		int offsetTriangles;
		int offsetFrames;
		int offsetGlCommands;
		int offsetEnd;
	};

	struct TexcoordMd2
	{
		short s, t;
	};

	struct VertexMd2
	{
		uint8_t v[3];
		uint8_t normalIndex;
	};

	struct TriangleMd2
	{
		int16_t vertIdx[3];
		int16_t texIdx[3];
	};

	struct FrameMd2
	{
		vec3 scale;
		vec3 translate;
		char name[16];
		VertexMd2  verts[1];	//csak egy pointer kell nekünk, minden frame GetNumVerts db vertexet tartalmaz,
		//ha ezen belül maradunk nem lesz gáz (de télleg)
	};

	struct AnimMd2
	{
		int start, end;
	};
};

} // namespace models

#endif // MODELMD2_H
