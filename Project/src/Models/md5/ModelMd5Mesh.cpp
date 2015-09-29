#include "GameStdAfx.h"
#include "Models/md5/ModelMd5.h"

#include "Common/LuaManager.h"

namespace models
{

bool ModelMd5::load(const char* meshFile, const char* animFile, MeshDirectory& meshDirectory, TextureDirectory& textureDirectory, const char* name, const bool justData)
{
	ModelMd5* md5 = new ModelMd5();

	if (md5->loadMesh(meshFile, textureDirectory, justData))
	{
		if (animFile)
		{
			md5->loadAnim(animFile);
		}
		meshDirectory[name] = md5;

		md5->m_currentAnimationName = utils::file::getFileName(animFile);

#ifdef GX_DEBUG_INFO
		md5->m_debug_meshFileName = meshFile;
		md5->m_debug_animFileName = meshFile;
#endif
		return true;
	}

	return false;
}

bool ModelMd5::loadMesh(const char* filename, TextureDirectory& textureDirectory, bool justSkeleton)
{
	char buff[512];
	uint curr_mesh = 0;
	uint i;

	int maxWeightCount = 0;

	FILE* fp;
	fp = fopen(filename, "rb");
	if (!fp)
	{
		TRACE_ERROR("Error: couldn't open " << filename, 0);
		return 0;
	}

	while (!feof(fp))
	{
		// Read whole line
		fgets(buff, sizeof(buff), fp);

		int version;
		if (sscanf(buff, " MD5Version %d", &version) == 1)
		{
			if (version != 10)
			{
				// Bad version
				fprintf(stderr, "Error: bad model version\n");
				fclose(fp);
				return 0;
			}
		}
		else if (sscanf(buff, " numJoints %d", &m_numJoints) == 1)
		{
			if (m_numJoints > 0)
			{
				// Allocate memory for base skeleton joints
				m_joints.resize(m_numJoints);
			}
		}
		else if (sscanf(buff, " numMeshes %d", &m_numMeshes) == 1)
		{
			if (m_numMeshes > 0)
			{
				// Allocate memory for meshes
				m_meshes.resize(m_numMeshes);
			}

			m_numObjects = m_numMeshes;
		}
		else if (strncmp(buff, "joints {", 8) == 0)
		{
			// Read each joint
			for (i = 0; i < m_numJoints; ++i)
			{
				char name[64];
				// Read whole line
				fgets(buff, sizeof(buff), fp);

				/*
				 if (sscanf(buff, "%s %d ( %f %f %f ) ( %f %f %f )", name, &joint->parent, &joint->pos.x, &joint->pos.y, &joint->pos.z, &joint->orientation.x, &joint->orientation.y,
				 &joint->orientation.z) == 8) {
				 // Compute the w component
				 joint->name = name;
				 }
				 */

				// If the name contains spaces
				const std::string str(buff);
				const size_t first = str.find_first_of('"');
				const size_t last = str.find_last_of('"') + 1;

				const size_t length = str.copy(name, last - first, first);
				name[length] = '\0';
				m_joints[i].name = name;

				vec3& currentJointPos = m_joints[i].pos;
				Quat& currentJointOrientation = m_joints[i].orientation;

				sscanf(buff + last, " %d ( %f %f %f ) ( %f %f %f )", &m_joints[i].parent, &currentJointPos.x, &currentJointPos.y, &currentJointPos.z, &currentJointOrientation.x, &currentJointOrientation.y, &currentJointOrientation.z);

				// delete
				std::string currentJointName = m_joints[i].name;
				currentJointName.erase(currentJointName.begin());
				currentJointName.erase(currentJointName.end() - 1);

				currentJointOrientation.calculateS();
			}
		}
		else if (strncmp(buff, "mesh {", 6) == 0)
		{
			MeshMd5* mesh = &m_meshes[curr_mesh];
			int vert_index = 0;
			int tri_index = 0;
			int weight_index = 0;
			float fdata[4];
			int idata[3];

			while ((buff[0] != '}') && !feof(fp))
			{
				// Read whole line
				fgets(buff, sizeof(buff), fp);

				if (strstr(buff, "shader "))
				{
					// we dont need the shader

					/*
					int quote = 0, j = 0;

					// Copy the shader name without the quote marks /
					for (i = 0; i < sizeof(buff) && (quote < 2); ++i) {
						if (buff[i] == '\"')
							quote++;

						if ((quote == 1) && (buff[i] != '\"')) {
							//mesh->shader[j] = buff[i];
							j++;
						}
					}
					*/

				}
				else if (sscanf(buff, " numverts %d", &mesh->numVertices) == 1)
				{
					if (mesh->numVertices > 0)
					{
						mesh->pVertices = new VertexMd5[mesh->numVertices];
					}

					if (mesh->numVertices > m_maxVertices)
					{
						m_maxVertices = mesh->numVertices;
					}

				}
				else if (sscanf(buff, " numtris %d", &mesh->numTriangles) == 1)
				{
					if (mesh->numTriangles > 0)
					{
						mesh->pTriangles = new poly3[mesh->numTriangles];
					}

					if (mesh->numTriangles > m_maxTriangles)
					{
						m_maxTriangles = mesh->numTriangles;
					}

				}
				else if (sscanf(buff, " numweights %d", &mesh->numWeights) == 1)
				{
					if (mesh->numWeights > 0)
					{
						mesh->pWeights = new Weight[mesh->numWeights];
					}
				}
				else if (sscanf(buff, " vert %d ( %f %f ) %d %d", &vert_index, &fdata[0], &fdata[1], &idata[0], &idata[1]) == 5)
				{
					// Copy vertex data
					mesh->pVertices[vert_index].texCoord.u = fdata[0];
					mesh->pVertices[vert_index].texCoord.v = 1.0f - fdata[1];
					mesh->pVertices[vert_index].weightIndex = idata[0];
					mesh->pVertices[vert_index].weightCount = idata[1];

					if (idata[1] > maxWeightCount)
					{
						maxWeightCount = idata[1];
					}

				}
				else if (sscanf(buff, " tri %d %d %d %d", &tri_index, &idata[0], &idata[1], &idata[2]) == 4)
				{
					// Copy triangle data
					mesh->pTriangles[tri_index].a = idata[2];
					mesh->pTriangles[tri_index].b = idata[1];
					mesh->pTriangles[tri_index].c = idata[0];

				}
				else if (sscanf(buff, " weight %d %d %f ( %f %f %f )", &weight_index, &idata[0], &fdata[3], &fdata[0], &fdata[1], &fdata[2]) == 6)
				{
					// Copy vertex data
					mesh->pWeights[weight_index].joint = idata[0];
					mesh->pWeights[weight_index].w = fdata[3];
					mesh->pWeights[weight_index].pos.x = fdata[0];
					mesh->pWeights[weight_index].pos.y = fdata[1];
					mesh->pWeights[weight_index].pos.z = fdata[2];
				}
			}

			curr_mesh++;
		}
	}
	fclose(fp);

	if (!justSkeleton)
	{
		m_pVertexIndices = new GLuint[m_maxTriangles * 3];
		m_pVertexArrayDynamic = new Vertex[m_maxVertices];

		normalizeVertexGroups();

		glGenBuffers(1, &m_indicesVboId);
		glGenBuffers(1, &m_verticesVboId);
	}

	return true;
}

//#define SERIALIZE_FIXED_DATA

void ModelMd5::normalizeVertexGroups()
{
	float epsilon = 0.3;

	VertexMd5* vertices = 0;
	Weight* weights = 0;

#ifdef SERIALIZE_FIXED_DATA
	std::ofstream out;
	out.open("fixin.txt");
#endif

	for (MeshMd5& mesh : m_meshes)
	{
		uint numWeights = mesh.numWeights;

		for (uint wi = 0; wi < mesh.numWeights; wi++)
		{
			if (mesh.pWeights[wi].w <= epsilon)
			{
				numWeights--;
			}
		}

		// are there any isolated vertices?
		for (uint vi = 0; vi < mesh.numVertices; vi++)
		{
			VertexMd5* vert = &mesh.pVertices[vi];

			bool allZero = true;
			int wi = mesh.pVertices[vi].weightIndex;

			while (wi < mesh.pVertices[vi].weightIndex + mesh.pVertices[vi].weightCount)
			{
				if (mesh.pWeights[wi].w > epsilon)
				{
					allZero = false;
				}
				wi++;
			}

			if (allZero)
			{
				numWeights++;
				int wi = mesh.pVertices[vi].weightIndex;
				while (wi < mesh.pVertices[vi].weightIndex + mesh.pVertices[vi].weightCount)
				{
					mesh.pWeights[wi].w = 0.0f;
					wi++;
				}

				mesh.pWeights[mesh.pVertices[vi].weightIndex].w = 1.0f;
				wi = mesh.pVertices[vi].weightIndex + 1;
				while (wi < mesh.pVertices[vi].weightIndex + mesh.pVertices[vi].weightCount)
				{
					++wi;
				}
			}
		}

		//SAFEDEL2(weights);
		weights = new Weight[numWeights];

		//SAFEDEL2(vertices);
		vertices = new VertexMd5[mesh.numVertices];

		TRACE_INFO("old numWeights: " << mesh.numWeights, 0);
		TRACE_INFO("new numWeights: " << numWeights, 0);


		// fixin it
		int weightIdx = 0;

		for (uint vi = 0; vi < mesh.numVertices; vi++)
		{
			float sum = 0.0f;
			int badWeights = 0;
			int wi = mesh.pVertices[vi].weightIndex;

			vertices[vi].weightIndex = weightIdx;
			while (wi < mesh.pVertices[vi].weightIndex + mesh.pVertices[vi].weightCount)
			{
				if (mesh.pWeights[wi].w <= epsilon)
				{
					badWeights++;
				}
				else
				{
					weights[weightIdx] = mesh.pWeights[wi];
					sum += mesh.pWeights[wi].w;
					weightIdx++;
				}
				++wi;
			}

			vertices[vi].texCoord = mesh.pVertices[vi].texCoord;
			vertices[vi].weightCount = mesh.pVertices[vi].weightCount - badWeights;

			//float sum2 = 0.0;
			// normalizing the weights
			for (int wi = vertices[vi].weightIndex; wi < vertices[vi].weightIndex + vertices[vi].weightCount; wi++)
			{
				weights[wi].w /= sum;
				//sum2 += weights[wi].w;
			}
		}

#ifdef SERIALIZE_FIXED_DATA
		out << "\tnumverts " << mesh.numVertices << std::endl;
		for (int vi = 0; vi < mesh.numVertices; vi++)
		{
			// vert [vertIndex] ( [texU] [texV] ) [weightIndex] [weightElem]
			out << "\tvert " << vi << " ( " << vertices[vi].texCoord.u << " " << vertices[vi].texCoord.v << " ) " << vertices[vi].weightIndex << " " << vertices[vi].weightCount << std::endl;
		}

		out << std::endl;
		out << "\tnumtris " << mesh.numTriangles << std::endl;
		for (int ti = 0; ti < mesh.numTriangles; ti++)
		{
			// tri [triIndex] [vertIndex1] [vertIndex2] [vertIndex3]
			out << "\ttri " << ti << " " << mesh.pTriangles[ti].a << " " << mesh.pTriangles[ti].b << " " << mesh.pTriangles[ti].c << std::endl;
		}

		out << std::endl;
		out << "\tnumweights " << numWeights << std::endl;
		for (int i = 0; i < numWeights; i++)
		{
			// weight [weightIndex] [jointIndex] [weightValue] ( [xPos] [yPos] [zPos] )
			out << "\tweight " << i << " " << weights[i].joint << " " << weights[i].w << " ( " << weights[i].pos.x << " " << weights[i].pos.y << " " << weights[i].pos.z << " )" << std::endl;
		}
#endif
		SAFEDEL2(mesh.pVertices);
		SAFEDEL2(mesh.pWeights);

		mesh.numWeights = numWeights;
		mesh.pVertices = vertices;
		mesh.pWeights = weights;
	}

#ifdef SERIALIZE_FIXED_DATA
	out.close();
#endif
}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

ModelMd5::ModelMd5()
	: m_pVertexIndices(nullptr)
	, m_pVertexArrayDynamic(nullptr)

	, m_numJoints(0)
	, m_numMeshes(0)

	, m_pBaseFrame(nullptr)
	, m_pCurrentFrame(nullptr)

	, m_maxVertices(0)
	, m_maxTriangles(0)
{
}

ModelMd5* ModelMd5::copy()
{
	ModelMd5* copy = new ModelMd5();

	copy->m_numJoints = m_numJoints;
	copy->m_numMeshes = m_numMeshes;
	copy->m_maxVertices = m_maxVertices;
	copy->m_maxTriangles = m_maxTriangles;

	copy->m_meshes = m_meshes;
	copy->m_joints = m_joints;
	copy->m_animations = m_animations;

	copy->m_pVertexIndices = new GLuint[m_maxTriangles * 3];
	copy->m_pVertexArrayDynamic = new Vertex[m_maxVertices];

	copy->m_pBaseFrame = new FrameMd5();
	copy->m_pBaseFrame->m_joints.resize(copy->m_joints.size());
	copy->m_pBaseFrame->m_joints = copy->m_joints;

	AnimMd5& currentAnimation = copy->m_animations[m_currentAnimationName];
	currentAnimation.animInfo.currentFrame = 0;
	currentAnimation.animInfo.nextFrame = 1;
	currentAnimation.animInfo.lastTime = 0;
	currentAnimation.animInfo.maxTime = 1.0 / copy->m_animations[m_currentAnimationName].frameRate;

	copy->m_isAnimationOn = true;
	copy->m_hasStanceChanged = true;

	copy->m_pCurrentFrame = new FrameMd5();
	*copy->m_pCurrentFrame = *copy->m_pBaseFrame;

	//copy->ragdoll->setupRagdoll(copy->currentFrame);

	return copy;
}

ModelMd5::~ModelMd5()
{
	SAFEDEL2(m_pVertexIndices);
	SAFEDEL2(m_pVertexArrayDynamic);

	SAFEDEL(m_pBaseFrame)
	SAFEDEL(m_pCurrentFrame)
}


// register to lua
void ModelMd5::registerMethodsToLua()
{
	using namespace luabind;

	class_<ModelMd5, std::shared_ptr<ModelMd5>> thisClass("ModelMd5");
	thisClass.def(constructor<>());

	thisClass.scope [
		def("load", &ModelMd5::load)
	];

	module(LuaManager::getInstance()->getState()) [ thisClass ];
}

} // namespace models
