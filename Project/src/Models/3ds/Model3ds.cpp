#include "GameStdAfx.h"
#include "Models/3ds/Model3ds.h"

#include "Common/LuaManager.h"
#include "Common/LoggerSystem.h"


namespace models
{

#define debugging 0


Model3ds::Model3ds() : m_pVertexOffsets(nullptr), m_pVerticesArray(nullptr)
{
}

Model3ds::~Model3ds()
{
	SAFEDEL2(m_pVertexOffsets);
	SAFEDEL2(m_pVerticesArray);
	//MQLOG("dest Model3ds", 0);
}

bool Model3ds::loadFile(const char* filename, MeshDirectory& meshDirectory, TextureDirectory& textureDirectory, const bool justData)
{
	uint numMaterials = 0;
	std::vector<Material3ds> materialsv;

	std::ifstream file;

	file.open(filename, std::ifstream::in | std::ifstream::binary);
	if (!file.is_open())
	{
		TRACE_ERROR("\tError: Cannot open 3ds file " << filename << ".", 0);
		return false;
	}

	// get file length
	file.seekg(0, std::ios::end);
	int fileLength = file.tellg();
	file.seekg(0, std::ios::beg);

	class ChunkInfo
	{
	public:
		unsigned short id;
		uint lenght;
	};

	ChunkInfo chunkInfo;

	while (file.tellg() < fileLength)
	{
		file.read((char*) &chunkInfo.id, sizeof(unsigned short));
		file.read((char*) &chunkInfo.lenght, sizeof(uint));

		switch (chunkInfo.id)
		{

			//----------------- MAIN3DS -----------------
			case MAIN3DS:
				break;

			//----------------- EDIT3DS -----------------
			case EDIT3DS:
				break;

			case OBJECT_CHK:
				loadObjectBlock(&file);

				break;

			//--------------- OBJ_TRIMESH ---------------
			case OBJ_TRIMESH:
				break;

			case VERTEX_CHK:
				loadVertexBlock(&file);

				break;

			case POLYIGON_CHK:
				loadPolygonBlock(&file);

				break;

			//--------------- MATERIAL_BLOCK ----------------
			case MATERIAL_BLOCK:
				break;

			case MATERIALID_CHK:
				loadMaterialIdsBlock(&file, materialsv);
				numMaterials++;

				break;

			case MATERIALNAME_CHK:
				loadMaterialNamesBlock(&file);

				break;

			//--------------- TEXTURE_BLOCK ----------------
			case TEXTURE_BLOCK:
				break;

			case TEXTURE_CHK:
				loadTextureBlock(&file, materialsv);

				break;

			case TEXCOORD_CHK:
				loadTexCoordsBlock(&file);

				break;

			//----------- Skip unknow chunks ------------
			default:
				file.seekg(chunkInfo.lenght - 6, std::ios::cur);
		}
	}
	file.close();

	for (uint o_i = 0; o_i < m_numObjects; o_i++)
	{
		for (uint v_i = 0; v_i < m_objects[o_i]->m_numVertices; v_i++)
		{
			m_objects[o_i]->m_pVertices[v_i] = transformVector(m_objects[o_i]->m_pVertices[v_i], vec3(0.0f), vec3(90, 0, 0));
		}
	}

	//
	for (uint o_i = 0; o_i < m_numObjects; o_i++)
	{
		for (uint i = 0; i < numMaterials; i++)
		{
			const std::string tmpMatname(m_objectTextureCache[m_numObjects - 1].m_materialId);
			const std::string tmpMatId(materialsv[i].materialId);

			if (tmpMatname == tmpMatId)
			{
				strcpy(m_objectTextureCache[m_numObjects - 1].m_textureName, materialsv[i].textureName); // memória hibát okozott
			}

			if (tmpMatname == "Default")
			{
				m_objectTextureCache[m_numObjects - 1].m_textureName[0] = 0;
			}
		}
	}


	if (justData)
	{
		return true;
	}

	loadTextures(filename, textureDirectory);
	buildVBOBuffers();

	m_objectTextureCache.clear();

	return true;
}

void Model3ds::buildVBOBuffers()
{
	vec3* tmpPolyNormals	= new vec3[getNumPolygons()];
	vec3* tmpVertNormals	= new vec3[getNumVertices()];
	m_pVerticesArray		= new Vertex[getNumPolygons() * 3];

	m_pVertexOffsets = new int[m_numObjects];


	uint v_i = 0;
	uint n_i = 0;
	uint tp_i = 0;

	for (uint o_i = 0; o_i < m_numObjects; o_i++)
	{
		if (o_i == 0)
		{
			m_pVertexOffsets[o_i] = 0;
		}
		else
		{
			m_pVertexOffsets[o_i] = m_pVertexOffsets[o_i - 1] + m_objects[o_i]->m_numTriangles * 3;
		}
	}

	// filling up the vertex and texcoord arrays
	for (uint o_i = 0; o_i < m_numObjects; o_i++)
	{
		for (uint p_i = 0; p_i < m_objects[o_i]->m_numTriangles; p_i++)
		{
			// vertex 1
			const poly3& currentTriangle = m_objects[o_i]->m_pTriangles[p_i];

			m_pVerticesArray[v_i].pos = m_objects[o_i]->m_pVertices[currentTriangle.a];

			m_pVerticesArray[v_i].texcoord.u = m_objects[o_i]->m_pTexcoords[currentTriangle.a].u;
			m_pVerticesArray[v_i].texcoord.v = m_objects[o_i]->m_pTexcoords[currentTriangle.a].v;

			// vertex 2
			m_pVerticesArray[v_i + 1].pos = m_objects[o_i]->m_pVertices[currentTriangle.b];

			m_pVerticesArray[v_i + 1].texcoord.u = m_objects[o_i]->m_pTexcoords[currentTriangle.b].u;
			m_pVerticesArray[v_i + 1].texcoord.v = m_objects[o_i]->m_pTexcoords[currentTriangle.b].v;

			// vertex 3
			m_pVerticesArray[v_i + 2].pos = m_objects[o_i]->m_pVertices[currentTriangle.c];

			m_pVerticesArray[v_i + 2].texcoord.u = m_objects[o_i]->m_pTexcoords[currentTriangle.c].u;
			m_pVerticesArray[v_i + 2].texcoord.v = m_objects[o_i]->m_pTexcoords[currentTriangle.c].v;

			// computing the polygon's tangent and bitangent vector
			vec3 tangent, bitangent;
			calculateTangent(m_pVerticesArray[v_i].pos, m_pVerticesArray[v_i + 1].pos, m_pVerticesArray[v_i + 2].pos, m_pVerticesArray[v_i].texcoord, m_pVerticesArray[v_i + 1].texcoord, m_pVerticesArray[v_i + 2].texcoord, tangent, bitangent);
			m_pVerticesArray[v_i].tangent = m_pVerticesArray[v_i + 1].tangent = m_pVerticesArray[v_i + 2].tangent = tangent;
			m_pVerticesArray[v_i].bitangent = m_pVerticesArray[v_i + 1].bitangent = m_pVerticesArray[v_i + 2].bitangent = bitangent;

			// computing the polygon's normal
			const vec3 v1 = m_pVerticesArray[v_i + 1].pos - m_pVerticesArray[v_i].pos;
			const vec3 v2 = m_pVerticesArray[v_i + 2].pos - m_pVerticesArray[v_i].pos;

			tmpPolyNormals[tp_i] = cross(v1, v2);
			tmpPolyNormals[tp_i] = normalize(tmpPolyNormals[tp_i]);
			tp_i++;

			v_i += 3;
		}

		// computing the vertex normals
		vec3 vSum(0.0f);
		const vec3 vZero = vSum;
		float shared = 0;

		for (uint v_i = 0; v_i < m_objects[o_i]->m_numVertices; v_i++)
		{
			for (uint p_i = 0; p_i < m_objects[o_i]->m_numTriangles; p_i++)
			{
				const poly3& currentTriangle = m_objects[o_i]->m_pTriangles[p_i];

				if (currentTriangle.a == v_i || currentTriangle.b == v_i || currentTriangle.c == v_i)
				{
					vSum += tmpPolyNormals[tp_i - m_objects[o_i]->m_numTriangles + p_i];
					shared++;
				}
			}

			tmpVertNormals[n_i] = vSum / shared;
			tmpVertNormals[n_i] = normalize(tmpVertNormals[n_i]);
			n_i++;

			vSum = vZero;
			shared = 0;
		}
	}

	// filling up the normal array
	n_i = 0;
	tp_i = 0;

	for (uint o_i = 0; o_i < m_numObjects; o_i++)
	{
		for (uint p_i = 0; p_i < m_objects[o_i]->m_numTriangles; p_i++)
		{
			const poly3& currentTriangle = m_objects[o_i]->m_pTriangles[p_i];

			m_pVerticesArray[n_i].normal		= tmpVertNormals[tp_i + currentTriangle.a];
			m_pVerticesArray[n_i + 1].normal	= tmpVertNormals[tp_i + currentTriangle.b];
			m_pVerticesArray[n_i + 2].normal	= tmpVertNormals[tp_i + currentTriangle.c];

			n_i += 3;
		}
		tp_i += m_objects[o_i]->m_numVertices;
	}

	glGenBuffers(1, &m_verticesVboId);
	glBindBuffer(GL_ARRAY_BUFFER, m_verticesVboId);
	glBufferData(GL_ARRAY_BUFFER, getNumPolygons() * 3 * sizeof(Vertex), m_pVerticesArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] tmpPolyNormals;
	delete[] tmpVertNormals;
}

bool Model3ds::loadTextures(const char* location, TextureDirectory& textureDirectory)
{
	for (uint o_i = 0; o_i < m_numObjects; o_i++)
	{
		if (m_objectTextureCache[m_numObjects - 1].m_textureName[0] == 0)
		{
			continue;
		}

		std::string texturename(m_objectTextureCache[m_numObjects - 1].m_textureName);
		const std::string locationtemp = utils::file::getDir(location);
		utils::toLowerCase(texturename);
		//strupr((char*)texturename.c_str());

		/*	##
		 for (int i = 0; i < texturename.length(); i++) {
		 // Max stores the texture names in upper case letters
		 if (isalpha(texturename[i]) && isupper(texturename[i])) {
		 texturename[i] += 32; // lower -> upper
		 }
		 }
		 */

		std::string textemp = locationtemp + texturename;

		if (textemp.length() > locationtemp.length())
		{
			// texture map
			loadTexture((char*) textemp.c_str(), m_objects[o_i]->m_decalMap, textureDirectory);

			// normalheight map
			std::string nhtemp = locationtemp + utils::file::getFileName(textemp) + "_nh.png";

			if (!loadTexture((char*) nhtemp.c_str(), m_objects[o_i]->m_normalHeightMap, textureDirectory))
			{
				// no nh texture -> use the blank texture
				m_objects[o_i]->m_normalHeightMap = 0;
			}
		}


	}

	return true;
}

void Model3ds::render()
{
	uint v_i = 0;

	std::vector<Object*>::iterator it = m_objects.begin();
	while (it != m_objects.end())
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (*it)->m_decalMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, (*it)->m_normalHeightMap);

		glDrawArrays(GL_TRIANGLES, v_i, (*it)->m_numTriangles * 3);
		v_i += (*it)->m_numTriangles * 3;

		++it;
	}
}

void Model3ds::renderSubset(const uint subset)
{
	// drawing
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_objects[subset]->m_decalMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_objects[subset]->m_normalHeightMap);

	int v_i = 0;
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


// register to lua
void Model3ds::registerMethodsToLua()
{
	using namespace luabind;

	class_<Model3ds, std::shared_ptr<Model3ds>> thisClass("Model3ds");
	thisClass.def(constructor<>());

	REG_FUNC("load", &Model3ds::load);

	module(LuaManager::getInstance()->getState()) [ thisClass ];
}

} // namespace models
