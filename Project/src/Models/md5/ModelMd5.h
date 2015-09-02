
#ifndef MODELMD5_H
#define MODELMD5_H

#include <imageLoad.h>

#include "Models/md5/Skeleton.h"
#include "Models/mesh/AnimatedMesh.h"


namespace models
{

// if the fps is too low, there might be too much weights per vertex -> in blender/weight paint mode/paint/clean weights
class ModelMd5 : public AnimatedMesh
{

	// member structure declarations
private:
	struct JointInfoMd5;
	struct MeshMd5;
	struct AnimMd5;


public:
	static bool load(const char* meshFile, const char* animFile, MeshDirectory& meshDirectory, TextureDirectory& textureDirectory, const char* name = "", const bool justData = false);

	ModelMd5();
	~ModelMd5();

	ModelMd5* copy();

	// mesh
	bool loadMesh(const char* filename, TextureDirectory& textureDirectory, bool justSkeleton = false);	// justSkeleton for the server
	// anim
	bool loadAnim(const char* filename);

	virtual void render();
	virtual void renderSubset(const uint subset);
	virtual void renderSkeleton();

	virtual void animate(const float dt);


	virtual void kill();


	// getters-setters

	FrameMd5* getSkeleton() const
	{
		return m_pCurrentFrame;
	}

	virtual float getFrame() const;
	virtual void setFrame(float time);

	virtual float getFrameTime() const;
	virtual void setFrameTime(float time);


private:
	void normalizeVertexGroups();

	void computeWeightNormals();
	void computeWeightTangents();

	bool buildFrameSkeleton(AnimMd5& anim, JointInfoMd5* pJointInfos, std::vector<JointMd5>& baseFrameJoints, const float* pAnimFrameData, int frameIndex);

	void prepareMesh(const MeshMd5& mesh, std::vector<JointMd5>& skeleton);
	void interpolateSkeletons(AnimMd5& anim, int frame1, int frame2, float interp);


private:
	uint									m_numJoints;
	uint									m_numMeshes;

	std::vector<JointMd5>					m_joints;
	std::vector<MeshMd5>					m_meshes;

	FrameMd5*								m_pBaseFrame;
	FrameMd5*								m_pCurrentFrame;

	uint									m_maxVertices;
	uint									m_maxTriangles;

	// TODO
	GLuint*									m_pVertexIndices;
	Vertex*									m_pVertexArrayDynamic;


	// animations and ragdoll
	std::map<const std::string, AnimMd5>	m_animations;


#ifdef GX_DEBUG_INFO
	std::string				m_debug_animFileName;
#endif


// member structure definitions
private:
	// structs
	struct VertexMd5
	{
		//vert vertIndex ( s t ) startWeight countWeight
		int			weightIndex;
		int			weightCount;

		texCoord	texCoord;
	};

	struct JointInfoMd5
	{
		//"name" parent flags startIndex
		std::string	name;
		int			parent;
		int			flags;
		int			startIndex;
	};

	struct Weight
	{
		int			joint;
		float		w;
		vec3		pos;
		vec3		normal;
		vec3		tangent;
		vec3		bitangent;
	};

	struct MeshMd5
	{
		GLuint		texid;
		GLuint		normalMap;
		GLuint		heightMap;
		GLuint		normalHeightMap;

		uint		numVertices;
		uint		numTriangles;
		uint		numWeights;

		VertexMd5*	pVertices;
		poly3*		pTriangles;
		Weight*		pWeights;

		MeshMd5()
			: texid(0)
			, normalMap(0)
			, heightMap(0)
			, normalHeightMap(0)

			, numVertices(0)
			, numTriangles(0)
			, numWeights(0)

			, pVertices(nullptr)
			, pTriangles(nullptr)
			, pWeights(nullptr)
		{
		}

		MeshMd5(const MeshMd5& o)
		{
			texid = o.texid;
			normalMap = o.normalMap;
			heightMap = o.heightMap;
			normalHeightMap = o.normalHeightMap;

			numVertices = o.numVertices;
			numTriangles = o.numTriangles;
			numWeights = o.numWeights;

			if (numVertices > 0)
			{
				pVertices = new VertexMd5[numVertices];
			}

			for (uint i = 0; i < numVertices; i++)
			{
				pVertices[i] = o.pVertices[i];
			}

			if (numTriangles > 0)
			{
				pTriangles = new poly3[numTriangles];
			}

			for (uint i = 0; i < numTriangles; i++)
			{
				pTriangles[i] = o.pTriangles[i];
			}

			if (numWeights > 0)
			{
				pWeights = new Weight[numWeights];
			}

			for (uint i = 0; i < numWeights; i++)
			{
				pWeights[i] = o.pWeights[i];
			}
		}

		MeshMd5& operator=(const MeshMd5& o)
		{
			if (this != &o)
			{
				texid = o.texid;
				normalMap = o.normalMap;
				heightMap = o.heightMap;
				normalHeightMap = o.normalHeightMap;

				numVertices = o.numVertices;
				numTriangles = o.numTriangles;
				numWeights = o.numWeights;

				if (numVertices > 0)
				{
					pVertices = new VertexMd5[numVertices];
				}

				for (uint i = 0; i < numVertices; i++)
				{
					pVertices[i] = o.pVertices[i];
				}

				if (numTriangles > 0)
				{
					pTriangles = new poly3[numTriangles];
				}

				for (uint i = 0; i < numTriangles; i++)
				{
					pTriangles[i] = o.pTriangles[i];
				}

				if (numWeights > 0)
				{
					pWeights = new Weight[numWeights];
				}

				for (uint i = 0; i < numWeights; i++)
				{
					pWeights[i] = o.pWeights[i];
				}
			}
			return *this;
		}

		~MeshMd5()
		{
			SAFEDEL2(pVertices);
			SAFEDEL2(pTriangles);
			SAFEDEL2(pWeights);
		}
	};

	struct AnimMd5Info
	{
		int		currentFrame;
		int		nextFrame;
		double	lastTime;
		double	maxTime;

		AnimMd5Info()
			: currentFrame(0)
			, nextFrame(1)
			, lastTime(0)
			, maxTime(0)
		{
		}
	};

	struct AnimMd5
	{
		uint					numFrames;
		uint					frameRate;
		AnimMd5Info				animInfo;
		std::vector<FrameMd5>	frames;

		AnimMd5()
			: numFrames(0)
			, frameRate(0)
		{
		}

		~AnimMd5()
		{
		}
	};



public:

	// register to lua
	static void registerMethodsToLua();
};

} // namespace models

#endif // MODELMD5_H
