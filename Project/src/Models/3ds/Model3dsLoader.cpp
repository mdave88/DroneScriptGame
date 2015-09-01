#include "GameStdAfx.h"
#include "Model3ds.h"


namespace models
{

bool Model3ds::load(const char* meshFile, MeshDirectory& meshDirectory, TextureDirectory& textureDirectory, const char* name, const bool justData)
{
	Model3ds* model = new Model3ds();

	if (model->loadFile(meshFile, meshDirectory, textureDirectory, justData))
	{
		meshDirectory[name] = model;


#ifdef GX_DEBUG_INFO
		model->m_debug_meshFileName = meshFile;
#endif
		return true;
	}

	return false;
}

//--------------- EDIT_OBJECT ---------------
// Description: Object block, info for each object
// Chunk ID: 4000(hex)
// Chunk Lenght: len(object name) + sub chunks
//-------------------------------------------
void Model3ds::loadObjectBlock(std::ifstream* file)
{
	uint8_t* pChar = new uint8_t;

	addObject(new Object());
	m_objectTextureCache.push_back(ObjectTextureCache());

	int i = 0;

	do
	{
		file->read((char*)pChar, sizeof(uint8_t));
		m_objectTextureCache[m_numObjects - 1].m_name[i] = *pChar;
		i++;
	}
	while (*pChar != '\0' && i < 20);

	m_objects[m_numObjects - 1]->m_name = std::string( m_objectTextureCache[m_numObjects - 1].m_name );

	delete pChar;
}

//--------------- TRI_VERTEXL ---------------
// Description: Vertices list
// Chunk ID: 4110(hex)
// Chunk Lenght: 1 x unsigned short(number of vertices)
//			+ 3 x float(vertex coordinates) x(number of vertices)
//			+ sub chunks
//-------------------------------------------
void Model3ds::loadVertexBlock(std::ifstream* file)
{
	unsigned short* pQty = new unsigned short;

	file->read((char*) pQty, sizeof(unsigned short));

	Object* pCurrentObject = m_objects[m_numObjects - 1];

	pCurrentObject->m_numVertices = *pQty;
	pCurrentObject->m_pVertices = new vec3[*pQty];

	for (int i = 0; i < *pQty; i++)
	{
		pCurrentObject->m_pVertices[i].set(0.0f);
		file->read((char*) &pCurrentObject->m_pVertices[i], sizeof(vec3));
	}

	delete pQty;
}

//--------------- TRI_FACEL1 ----------------
// Description: Polygons(faces) list
// Chunk ID: 4120(hex)
// Chunk Lenght: 1 x unsigned short(number of polygons)
//			+ 3 x unsigned short(polygon points) x(number of polygons)
//			+ sub chunks
//-------------------------------------------
void Model3ds::loadPolygonBlock(std::ifstream* file)
{
	unsigned short* pQty = new unsigned short;
	unsigned short* pShort = new unsigned short;

	file->read((char*) pQty, sizeof(unsigned short));

	Object* pCurrentObject = m_objects[m_numObjects - 1];

	pCurrentObject->m_numTriangles = *pQty;
	pCurrentObject->m_pTriangles = new poly3[*pQty];

	for (int i = 0; i < *pQty; i++)
	{
		pCurrentObject->m_pTriangles[i].a = 0;
		pCurrentObject->m_pTriangles[i].b = 0;
		pCurrentObject->m_pTriangles[i].c = 0;

		file->read((char*) pShort, sizeof(unsigned short));
		pCurrentObject->m_pTriangles[i].a = *pShort;
		file->read((char*) pShort, sizeof(unsigned short));
		pCurrentObject->m_pTriangles[i].b = *pShort;
		file->read((char*) pShort, sizeof(unsigned short));
		pCurrentObject->m_pTriangles[i].c = *pShort;

		//Face flags
		file->read((char*) pShort, sizeof(unsigned short));
	}

	delete pQty;
	delete pShort;
}

//--------------- MAPPING FILENAME ----------------
// Description: Texture names
// Chunk ID: A300(hex)
// Chunk Lenght: KITOLTENI
// Desc : dealing with only one texture per material
//-------------------------------------------
void Model3ds::loadTextureBlock(std::ifstream* file, std::vector<Material3ds>& materialsv)
{
	uint8_t* pChar = new uint8_t;

	int i = 0;
	char tmpTextureName[20];
	do
	{
		file->read((char*)pChar, sizeof(uint8_t));
		tmpTextureName[i] = *pChar;
		i++;
	}
	while (*pChar != '\0' && i < 20);
	materialsv.back().setTextureName(tmpTextureName);

	delete pChar;
}

//------------- TRI_MAPPINGCOORS ------------
// Description: Map coords list
// Chunk ID: 4140(hex)
// Chunk Lenght: 1 x unsigned short(number of mapping points)
//			+ 2 x float(mapping coordinates) x(number of mapping points)
//			+ sub chunks
//-------------------------------------------
void Model3ds::loadTexCoordsBlock(std::ifstream* file)
{
	unsigned short* pQty = new unsigned short;

	file->read((char*) pQty, sizeof(unsigned short));

	m_numTexcoords = *pQty;

	Object* pCurrentObject = m_objects[m_numObjects - 1];
	pCurrentObject->m_pTexcoords = new texCoord[*pQty];

	for (int i = 0; i < *pQty; i++)
	{
		file->read((char*) &pCurrentObject->m_pTexcoords[i], sizeof(texCoord));
	}

	delete pQty;
}

//--------------- MATERIAL ID ----------------
// Description: Material ids
// Chunk ID: A000(hex)
// Chunk Lenght: KITOLTENI
//-------------------------------------------
void Model3ds::loadMaterialIdsBlock(std::ifstream* file, std::vector<Material3ds>& materialsv)
{
	uint8_t* pChar = new uint8_t;

	int i = 0;
	char tmpMaterialId[20];
	do
	{
		file->read((char*)pChar, sizeof(uint8_t));
		tmpMaterialId[i] = *pChar;
		i++;
	}
	while (*pChar != '\0' && i < 20);
	materialsv.push_back(Material3ds(tmpMaterialId));

	delete pChar;
}

//--------------- MESH MATERIAL ID ----------------
// Description: Material names
// Chunk ID: 4130(hex)
// Chunk Lenght: KITOLTENI
//-------------------------------------------
void Model3ds::loadMaterialNamesBlock(std::ifstream* file)
{
	unsigned short* pQty = new unsigned short;
	uint8_t* pChar = new uint8_t;

	int i = 0;
	do
	{
		file->read((char*)pChar, sizeof(uint8_t));
		m_objectTextureCache[m_numObjects - 1].m_materialId[i] = *pChar;
		i++;
	}
	while (*pChar != '\0' && i < 20);

	file->read((char*) pQty, sizeof(unsigned short));

	int tmpSize = *pQty;
	for (i = 0; i < tmpSize; i++)
	{
		file->read((char*) pQty, sizeof(unsigned short));
	}

	delete pQty;
	delete pChar;
}

} // namespace models
