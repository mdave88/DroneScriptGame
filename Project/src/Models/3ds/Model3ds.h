/*
 thanks to www.spacesimulator.net
 */
#pragma once

#include <imageLoad.h>

#include "Models/mesh/Mesh.h"

namespace models
{

#define MAIN3DS 0x4d4d
#define EDIT3DS 0x3d3d
#define OBJ_TRIMESH 0x4100
#define MATERIAL_BLOCK 0xAFFF
#define TEXTURE_BLOCK 0xA200

#define OBJECT_CHK 0x4000
#define VERTEX_CHK 0x4110
#define POLYIGON_CHK 0x4120
#define TEXTURE_CHK 0xA300
#define TEXCOORD_CHK 0x4140
#define MATERIALID_CHK 0xA000
#define MATERIALNAME_CHK 0x4130

class Model3ds : public Mesh
{
	class Material3ds
	{
	public:
		char materialId[20];
		char textureName[20];

		Material3ds()
		{
			materialId[0] = 0;
			textureName[0] = 0;
		}

		Material3ds(char matId[20])
		{
			textureName[0] = 0;
			strcpy(materialId, matId);
		}

		Material3ds(char matId[20], char texName[20])
		{
			strcpy(materialId, matId);
			strcpy(textureName, texName);
		}

		void setMaterialId(char matId[20])
		{
			strcpy(materialId, matId);
		}

		void setTextureName(char texName[20])
		{
			strcpy(textureName, texName);
		}
	};

	class ObjectTextureCache
	{
	public:
		char m_name[20];
		char m_textureName[20];
		char m_materialId[20];
	};


public:
	static bool load(const char* meshFile, MeshDirectory& meshDirectory, TextureDirectory& textureDirectory, const char* name, const bool justData = false);

	Model3ds();
	~Model3ds();

	bool loadFile(const char* filename, MeshDirectory& meshDirectory, TextureDirectory& textureDirectory, const bool justData = false);

	virtual void render();
	virtual void renderSubset(const uint subset);

	// register to lua
	static void registerMethodsToLua();


private:
	void loadObjectBlock(std::ifstream* file);
	void loadVertexBlock(std::ifstream* file);
	void loadPolygonBlock(std::ifstream* file);
	void loadTextureBlock(std::ifstream* file, std::vector<Material3ds>& materialsv);
	void loadTexCoordsBlock(std::ifstream* file);
	void loadMaterialIdsBlock(std::ifstream* file, std::vector<Material3ds>& materialsv);
	void loadMaterialNamesBlock(std::ifstream* file);


	bool loadTextures(const char* location, TextureDirectory& textureDirectory);
	void calculateTangentArray();
	void buildVBOBuffers();


protected:
	uint	m_numTexcoords;
	int*	m_pVertexOffsets;
	Vertex* m_pVerticesArray;

	std::vector<ObjectTextureCache> m_objectTextureCache;
};

} // namespace models
