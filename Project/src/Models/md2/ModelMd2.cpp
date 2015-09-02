#include "GameStdAfx.h"
#include "ModelMd2.h"

namespace models
{

#define FRAMETIME   10

// Magic number
#define MD2_MAGIC_NUM	(('2'<<24) + ('P'<<16) + ('D'<<8) + 'I')

bool ModelMd2::load(const char* meshFile, MeshDirectory& meshDirectory, TextureDirectory& textureDirectory, const char* name, const bool justData)
{
	ModelMd2* model = new ModelMd2();

	if (model->load(meshFile, textureDirectory, justData))
	{
		meshDirectory[name] = model;
		return true;
	}

	return false;
}

ModelMd2::ModelMd2()
	:	m_time(0),

		m_numVertices(0),
		m_numTriangles(0),
		m_numTexCoords(0),

		// pointers
		m_pTexcoords(nullptr),
		m_pTangents(nullptr),
		m_pVertices(nullptr),
		m_pNormals(nullptr),
		m_pTriangles(nullptr),

		m_pVertexArray(nullptr),
		m_pNormalArray(nullptr),
		m_pTangentArray(nullptr),
		m_pTriangleArray(nullptr),
		m_pTexcoordArray(nullptr),

		m_numAnims(0)
{
	m_numObjects = 1;
}

ModelMd2::~ModelMd2()
{
	SAFEDEL2(m_pTexcoords);
	SAFEDEL2(m_pTexcoordArray);
	SAFEDEL2(m_pVertices);
	SAFEDEL2(m_pVertexArray);
	SAFEDEL2(m_pNormals);
	SAFEDEL2(m_pNormalArray);
	SAFEDEL2(m_pTangents);
	SAFEDEL2(m_pTangentArray);
	SAFEDEL2(m_pTriangles);
	SAFEDEL2(m_pTriangleArray);
}

void ModelMd2::copy(const ModelMd2& other)
{
	m_numTexCoords = other.m_numTexCoords;
	m_pTexcoords = new texCoord[m_numTexCoords];
	m_pTexcoordArray = new texCoord[m_numVertices];

	for (int i = 0; i < m_numTexCoords; i++)
	{
		m_pTexcoords[i] = other.m_pTexcoords[i];
	}


	m_numTriangles = other.m_numTriangles;
	m_pTriangles = new TriangleMd2[m_numTriangles];

	for (int i = 0; i < m_numTriangles; i++)
	{
		m_pTriangles[i] = other.m_pTriangles[i];
	}


	m_numAnims = other.m_numAnims;
	for (const auto& entry : other.m_animations)
	{
		m_animations[entry.first].start = entry.second.start;
		m_animations[entry.first].end   = entry.second.end;
	}

	int numFrames = m_animations.crbegin()->second.start + m_animations.crbegin()->second.end + 1;
	m_numVertices = other.m_numVertices;

	m_pVertices = new vec3[numFrames * m_numVertices];
	m_pNormals = new vec3[numFrames * m_numVertices];
	m_pTangents = new vec3[numFrames * m_numVertices];

	m_pVertexArray = new vec3[m_numVertices];
	m_pNormalArray = new vec3[m_numVertices];
	m_pTangentArray = new vec3[m_numVertices];

	m_pTriangleArray = nullptr;

	for (int i = 0; i < numFrames * m_numVertices; i++)
	{
		m_pVertices[i] = other.m_pVertices[i];
		m_pNormals[i]  = other.m_pNormals[i];
		m_pTangents[i]  = other.m_pTangents[i];
	}

	for (int i = 0; i < m_numVertices; i++)
	{
		m_pTexcoordArray[i] = other.m_pTexcoordArray[i];
	}

	m_decalMap = other.m_decalMap;
	m_normalMap = other.m_normalMap;
	m_heightMap = other.m_heightMap;
	m_normalHeightMap = other.m_normalHeightMap;

	m_time = 0;
	m_animationSpeed = 1;
}

ModelMd2::ModelMd2(const ModelMd2& other)
{
	copy(other);
}

ModelMd2& ModelMd2::operator=(const ModelMd2& other)
{
	if (this == &other)
	{
		SAFEDEL2(m_pTexcoords);
		SAFEDEL2(m_pTexcoordArray);
		SAFEDEL2(m_pVertices);
		SAFEDEL2(m_pVertexArray);
		SAFEDEL2(m_pNormals);
		SAFEDEL2(m_pNormalArray);
		SAFEDEL2(m_pTangents);
		SAFEDEL2(m_pTangentArray);
		SAFEDEL2(m_pTriangles);
		SAFEDEL2(m_pTriangleArray);


		copy(other);
	}

	return *this;
}

bool filter(char c)
{
	return c == '"' || c == '_';
}

bool ModelMd2::load(const char* filename, TextureDirectory& textureDirectory, const bool justData)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary);

	if (file.fail())
	{
		return false;
	}

	int fileSize;
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	char* buffer = new char[fileSize];
	file.read((char*) buffer, fileSize);
	file.close();

	// header
	Header* header = reinterpret_cast<Header*>(buffer);
	if ((header->magic != MD2_MAGIC_NUM) && (header->version != 8))
	{
		file.close();
		return false;
	}

	m_numTexCoords = header->numTexCoords;
	m_numVertices  = header->numVertices;
	m_numTriangles = header->numTriangles;

	int numFrames = header->numFrames;

	// texcoords
	TexcoordMd2* tmptex = new TexcoordMd2[m_numTexCoords];
	memcpy(tmptex, &buffer[header->offsetTexCoords], m_numTexCoords * sizeof(TexcoordMd2));

	m_pTexcoords = new texCoord[m_numTexCoords];
	for (int i = 0; i < m_numTexCoords; i++)
	{
		m_pTexcoords[i].u = (float) tmptex[i].s / header->skinWidth;
		m_pTexcoords[i].v = -(float) tmptex[i].t / header->skinHeight;
	}
	delete[] tmptex;
	m_pTexcoordArray = new texCoord[m_numTriangles * 3];							// one vertex can have more texture coordinates (eg. a corner of a cube)

	// triangles
	m_pTriangles = new TriangleMd2[m_numTriangles];
	memcpy(m_pTriangles, &buffer[header->offsetTriangles], m_numTriangles * sizeof(TriangleMd2));

	// vertices
	m_pVertices = new vec3[numFrames * m_numVertices];
	m_pVertexArray = new vec3[m_numTriangles * 3];

	// normal vectors
	m_pNormals = new vec3[numFrames * m_numVertices];
	m_pNormalArray = new vec3[m_numTriangles * 3];

	// tangent vectors
	m_pTangents = new vec3[numFrames * m_numVertices];
	m_pTangentArray = new vec3[m_numTriangles * 3];

	// precalculated normal vectors
	float anorms[162][3] =
	{
#include "anorms.h"
	};

	FrameMd2* frame;
	std::string lastname;
	std::string filteredLastname;

	// animációk, vertexek kiolvasása, beállítása
	for (int i = 0; i < numFrames; i++)
	{
		frame = (FrameMd2*) &buffer[ header->offsetFrames + i * header->frameSize ];

		// az azonos nevű frameket egy animációba pakoljuk (pl. stand_1, stand_2 ...)
		if (lastname == "" || strncmp(lastname.c_str(), frame->name, lastname.size()) > 0)
		{
			lastname = frame->name;

			if (lastname[lastname.size() - 2] == '0')			// eg. stand01
			{
				lastname.resize(lastname.size() - 2);
			}
			else
			{
				lastname.resize(lastname.size() - 1);
			}

			filteredLastname = lastname;
			filteredLastname.resize( std::remove_if(filteredLastname.begin(), filteredLastname.end(), filter) - filteredLastname.begin() );

			m_numAnims++;
			m_animations[filteredLastname].start = i;
			m_animations[filteredLastname].end   = -1;
		}
		m_animations[filteredLastname].end++;

		for (int j = 0; j < m_numVertices; j++)
		{
			m_pVertices[ i * m_numVertices + j ].x = (frame->verts[j].v[0] * frame->scale[0]) + frame->translate[0];
			m_pVertices[ i * m_numVertices + j ].y = (frame->verts[j].v[1] * frame->scale[1]) + frame->translate[1];
			m_pVertices[ i * m_numVertices + j ].z = (frame->verts[j].v[2] * frame->scale[2]) + frame->translate[2];

			m_pVertices[ i * m_numVertices + j ] = transformVector(m_pVertices[ i * m_numVertices + j ], vec3(0.0f), vec3(90, 90, 0));
			m_pNormals[ i * m_numVertices + j ].set( anorms[frame->verts[j].normalIndex] );
		}


		// calculate tangent vectors
		vec3 tangent, bitangent;
		for (int j = 0; j < m_numTriangles; j++)
		{
			calculateTangent(m_pVertices[ i * m_numVertices + m_pTriangles[j].vertIdx[2] ], m_pVertices[ i * m_numVertices + m_pTriangles[j].vertIdx[1] ], m_pVertices[ i * m_numVertices + m_pTriangles[j].vertIdx[0] ],
			                 m_pTexcoords[ m_pTriangles[j].texIdx[2] ], m_pTexcoords[ m_pTriangles[j].texIdx[1] ], m_pTexcoords[ m_pTriangles[j].texIdx[0] ],
			                 tangent, bitangent);

			m_pTangents[ i * m_numVertices + m_pTriangles[j].vertIdx[0] ] = m_pTangents[ i * m_numVertices + m_pTriangles[j].vertIdx[1] ] = m_pTangents[ i * m_numVertices + m_pTriangles[j].vertIdx[2] ] = tangent;
		}
	}

	int v_i = 0;
	for (int j = 0; j < m_numTriangles; j++)
	{
		m_pNormalArray[v_i] = m_pNormals[ m_pTriangles[j].vertIdx[2] ];
		m_pNormalArray[v_i + 1] = m_pNormals[ m_pTriangles[j].vertIdx[1] ];
		m_pNormalArray[v_i + 2] = m_pNormals[ m_pTriangles[j].vertIdx[0] ];

		m_pTexcoordArray[v_i] = m_pTexcoords[ m_pTriangles[j].texIdx[2] ];
		m_pTexcoordArray[v_i + 1] = m_pTexcoords[ m_pTriangles[j].texIdx[1] ];
		m_pTexcoordArray[v_i + 2] = m_pTexcoords[ m_pTriangles[j].texIdx[0] ];

		v_i += 3;
	}

	// load textures
	if (!justData)
	{
		// the name of the texture is usually not stored n the file
		char textureNameTmp[64];
		memcpy(textureNameTmp, &buffer[header->offsetSkins], 64 * sizeof(char));


		std::string locationtemp = utils::file::getDir(std::string(filename));
		std::string textureName(textureNameTmp);
		utils::toLowerCase(textureName);

		std::string textemp = locationtemp + textureName;

		if (textemp.length() > locationtemp.length())
		{
			// texture map
			loadTexture((char*) textemp.c_str(), m_decalMap, textureDirectory);

			// normalheight map
			std::string nhtemp = locationtemp + utils::file::getFileName(textemp) + "_nh.png";

			if (!loadTexture((char*) nhtemp.c_str(), m_normalHeightMap, textureDirectory))
			{
				// no nh texture -> use the blank texture
				m_normalHeightMap = 0;
			}
		}
	}

	delete[] buffer;

	return true;
}

void ModelMd2::animate(const float dt)
{
	if (m_isAnimationOn)
	{
		m_time += dt * m_animationSpeed;
	}

	float fframe = m_animations[m_currentAnimationName].start + m_time * 45;
	int f1 = (int) (fframe);
	int f2 = f1 + 1;
	float int_time = fframe - f1;

	if (f2 > m_animations[m_currentAnimationName].start + m_animations[m_currentAnimationName].end)
	{
		if (f1 > m_animations[m_currentAnimationName].start + m_animations[m_currentAnimationName].end)
		{
			f1 = m_animations[m_currentAnimationName].start + m_animations[m_currentAnimationName].end;

			if (m_isAnimationLooped)
			{
				m_time = 0;
			}
			else
			{
				m_isAnimationOn = false;
			}
		}
		f2 = m_animations[m_currentAnimationName].start;
	}


	int v_i = 0;
	for (int p_i = 0; p_i < m_numTriangles; p_i++)
	{
		// Vertex 1
		m_pVertexArray[v_i] = m_pVertices[ f1 * m_numVertices + m_pTriangles[p_i].vertIdx[2] ].interpolate(m_pVertices[f2 * m_numVertices + m_pTriangles[p_i].vertIdx[2]], int_time);

		// Vertex 2
		m_pVertexArray[v_i + 1] = m_pVertices[ f1 * m_numVertices + m_pTriangles[p_i].vertIdx[1] ].interpolate(m_pVertices[f2 * m_numVertices + m_pTriangles[p_i].vertIdx[1]], int_time);

		// Vertex 3
		m_pVertexArray[v_i + 2] = m_pVertices[ f1 * m_numVertices + m_pTriangles[p_i].vertIdx[0] ].interpolate(m_pVertices[f2 * m_numVertices + m_pTriangles[p_i].vertIdx[0]], int_time);

		//tangentArray[v_i] = tangentArray[v_i + 1] = tangentArray[v_i + 2] = tangents[ f1 * numVertices + triangles[p_i].vertIdx[0] ].interpolate(tangents[f2 * numVertices + triangles[p_i].vertIdx[0]], int_time);
		vec3 tangent, bitangent;
		calculateTangent(m_pVertexArray[v_i], m_pVertexArray[v_i + 1], m_pVertexArray[v_i + 2], m_pTexcoordArray[v_i], m_pTexcoordArray[v_i + 1], m_pTexcoordArray[v_i + 2], tangent, bitangent);
		m_pTangentArray[v_i] = m_pTangentArray[v_i + 1] = m_pTangentArray[v_i + 2] = tangent;

		v_i += 3;
	}
}

void ModelMd2::render()
{
	glPushMatrix();


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);


	// decal map
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_decalMap);

	// texcoords
	glEnable(GL_TEXTURE_2D);
	glTexCoordPointer(2, GL_FLOAT, 0, m_pTexcoordArray);

	// normal height map
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalHeightMap);

	// tangents
	glEnable(GL_TEXTURE_2D);
	glTexCoordPointer(3, GL_FLOAT, 0, m_pTangentArray);

	// normals, vertices, indices
	glNormalPointer(GL_FLOAT, 0, m_pNormalArray);
	glVertexPointer(3, GL_FLOAT, 0, m_pVertexArray);

	// render the model
	//glDrawElements(GL_TRIANGLES, numTriangles * 3, GL_UNSIGNED_INT, triangleArray);
	glDrawArrays(GL_TRIANGLES, 0, m_numTriangles * 3);


	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);




	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);


	//// decal map
	//glClientActiveTexture(GL_TEXTURE0);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, decalMap);

	//// texcoords
	//glEnable(GL_TEXTURE_2D);
	//glTexCoordPointer(2, GL_FLOAT, 0, texcoordArray);

	//// normal height map
	//glClientActiveTexture(GL_TEXTURE1);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, normalHeightMap);

	//// tangents
	//glEnable(GL_TEXTURE_2D);
	//glTexCoordPointer(3, GL_FLOAT, 0, tangentArray);

	//// normals, vertices, indices
	//glNormalPointer(GL_FLOAT, 0, normalArray);
	//glVertexPointer(3, GL_FLOAT, 0, vertexArray);

	//// render the model
	//glDrawElements(GL_TRIANGLES, numTriangles * 3, GL_UNSIGNED_INT, triangleArray);


	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, 0);



	//// decal tex
	//glEnable(GL_TEXTURE_2D);
	//glClientActiveTexture(GL_TEXTURE0);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, decalMap);

	//// normal height map
	//glClientActiveTexture(GL_TEXTURE1);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, normalHeightMap);
	//// tangents
	//glEnable(GL_TEXTURE_2D);
	//glTexCoordPointer(3, GL_FLOAT, 0, tangentArray);

	//glBegin(GL_TRIANGLES);
	//for (int i = 0; i < numTriangles; i++) {
	//	glTexCoord2f( texcoords[triangles[i].texIdx[2]].u, texcoords[triangles[i].texIdx[2]].v );
	//	glNormal3f( VEC3_TO_F3(normalArray[triangles[i].vertIdx[2]]) );
	//	glVertex3f( VEC3_TO_F3(vertexArray[triangles[i].vertIdx[2]]) );

	//	glTexCoord2f( texcoords[triangles[i].texIdx[1]].u, texcoords[triangles[i].texIdx[1]].v );
	//	glNormal3f( VEC3_TO_F3(normalArray[triangles[i].vertIdx[1]]) );
	//	glVertex3f( VEC3_TO_F3(vertexArray[triangles[i].vertIdx[1]]) );

	//	glTexCoord2f( texcoords[triangles[i].texIdx[0]].u, texcoords[triangles[i].texIdx[0]].v );
	//	glNormal3f( VEC3_TO_F3(normalArray[triangles[i].vertIdx[0]]) );
	//	glVertex3f( VEC3_TO_F3(vertexArray[triangles[i].vertIdx[0]]) );
	//}
	//glEnd();

	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();
}

void ModelMd2::renderSubset(const uint subset)
{
	render();
}

// getters-setters

// TODO: az animaciokon belül legyenek frame és frameTime attribútumok
float ModelMd2::getFrame() const
{
	return m_time;
}

void ModelMd2::setFrame(float time)
{
	setFrameTime(time);
}

float ModelMd2::getFrameTime() const
{
	return m_time;
}

void ModelMd2::setFrameTime(float time)
{
	m_time = time;
}

} // namespace models
