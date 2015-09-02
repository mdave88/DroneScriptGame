#include "GameStdAfx.h"
#include "Models/md5/ModelMd5.h"

namespace models
{

bool ModelMd5::loadAnim(const char* filename)
{
	FILE* fp = nullptr;
	char buff[512];

	JointInfoMd5* pJointInfos = nullptr;
	std::vector<JointMd5> baseFrameJoints;
	float* pAnimFrameData = nullptr;

	int version;
	int numAnimatedComponents;
	int frameIndex;

	uint i;

	fp = fopen(filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "error: couldn't open \"%s\"!\n", filename);
		return 0;
	}

	m_animations[utils::file::getFileName(filename)] = AnimMd5();
	AnimMd5* pAnim = &m_animations[utils::file::getFileName(filename)];

	while (!feof(fp))
	{
		fgets(buff, sizeof(buff), fp);

		if (sscanf(buff, " MD5Version %d", &version) == 1)
		{
			if (version != 10)
			{
				fprintf(stderr, "Error: bad animation version\n");
				fclose(fp);
				return 0;
			}
		}
		else if (sscanf(buff, " numFrames %d", &pAnim->numFrames) == 1)
		{
			// Allocate memory for skeleton frames and bounding boxes
			if (pAnim->numFrames > 0)
			{
				pAnim->frames.resize(pAnim->numFrames);
			}
		}
		else if (sscanf(buff, " numJoints %d", &m_numJoints) == 1)
		{
			if (m_numJoints > 0)
			{
				// Allocate temporary memory for building skeleton frames
				pJointInfos = new JointInfoMd5[m_numJoints];
				baseFrameJoints.resize(m_numJoints);
			}
		}
		else if (sscanf(buff, " frameRate %d", &pAnim->frameRate) == 1)
		{
			pAnim->animInfo.maxTime = 1.0 / pAnim->frameRate;
			// printf ("md5anim: animation's frame rate is %d\n", anim->frameRate);
		}
		else if (sscanf(buff, " numAnimatedComponents %d", &numAnimatedComponents) == 1)
		{
			if (numAnimatedComponents > 0)
			{
				// Allocate memory for animation frame data
				pAnimFrameData = new float[numAnimatedComponents];//(float *) malloc(sizeof(float) * numAnimatedComponents);
			}
		}
		else if (strncmp(buff, "hierarchy {", 11) == 0)
		{
			for (i = 0; i < m_numJoints; ++i)
			{
				// Read whole line
				fgets(buff, sizeof(buff), fp);
				char name[64];
				// Read joint info
				sscanf(buff, " %63s %d %d %d", name, &pJointInfos[i].parent, &pJointInfos[i].flags, &pJointInfos[i].startIndex);

				pJointInfos[i].name = name;

				// If the name consist spaces
				if (pJointInfos[i].name.at(pJointInfos[i].name.length() - 1) != '\"')
				{
					const std::string str(buff);
					const size_t first = str.find_first_of('"');
					const size_t last = str.find_last_of('"') + 1;

					const size_t length = str.copy(name, last - first, first);
					name[length] = '\0';
					pJointInfos[i].name = name;
					sscanf(buff + last, " %d %d %d", &pJointInfos[i].parent, &pJointInfos[i].flags, &pJointInfos[i].startIndex);
				}
			}
		}
		else if (strncmp(buff, "bounds {", 8) == 0)
		{
			for (i = 0; i < pAnim->numFrames; ++i)
			{
				// Read whole line
				fgets(buff, sizeof(buff), fp);

				pAnim->frames[i] = FrameMd5(m_numJoints);
				// Read bounding box

				FrameMd5& frame = pAnim->frames[i];

				sscanf(buff, " ( %f %f %f ) ( %f %f %f )", &frame.m_bbox.minp.x, &frame.m_bbox.minp.y, &frame.m_bbox.minp.z, &frame.m_bbox.maxp.x, &frame.m_bbox.maxp.y, &frame.m_bbox.maxp.z);
			}
		}
		else if (strncmp(buff, "baseframe {", 10) == 0)
		{
			for (i = 0; i < m_numJoints; ++i)
			{
				// Read whole line
				fgets(buff, sizeof(buff), fp);

				// Read base frame joint
				JointMd5& baseFrameJoint = baseFrameJoints[i];

				if (sscanf(buff, " ( %f %f %f ) ( %f %f %f )", &baseFrameJoint.pos.x, &baseFrameJoint.pos.y, &baseFrameJoint.pos.z, &baseFrameJoint.orientation.x, &baseFrameJoint.orientation.y, &baseFrameJoint.orientation.z) == 6)
				{
					baseFrameJoint.orientation.calculateS();
				}
			}
		}
		else if (sscanf(buff, " frame %d", &frameIndex) == 1)
		{
			// Read frame data
			for (i = 0; i < (uint) numAnimatedComponents; ++i)
			{
				fscanf(fp, "%f", &pAnimFrameData[i]);
			}

			// Build frame skeleton from the collected data
			buildFrameSkeleton(*pAnim, pJointInfos, baseFrameJoints, pAnimFrameData, frameIndex);
		}
	}
	fclose(fp);

	// Free temporary data allocated
	SAFEDEL2(pAnimFrameData);
	SAFEDEL2(pJointInfos);


	if (!m_pBaseFrame)
	{
		m_pBaseFrame = new FrameMd5();
		m_pBaseFrame->m_joints = m_joints;

		computeWeightNormals();
		computeWeightTangents();

		m_pCurrentFrame = new FrameMd5();
		*m_pCurrentFrame = *m_pBaseFrame;
	}

	return true;
}

bool ModelMd5::buildFrameSkeleton(AnimMd5& anim, JointInfoMd5* pJointInfos, std::vector<JointMd5>& baseFrameJoints, const float* pAnimFrameData, int frameIndex)
{
	static const vec3 scale(1.2);

	for (uint i = 0; i < m_numJoints; i++)
	{
		vec3 animatedPos;
		Quat animatedOrient;
		int j = 0;

		animatedPos = baseFrameJoints[i].pos;
		animatedOrient = baseFrameJoints[i].orientation;

		// Tx
		if (pJointInfos[i].flags & 1)
		{
			animatedPos.x = pAnimFrameData[pJointInfos[i].startIndex + j];
			++j;
		}

		// Ty
		if (pJointInfos[i].flags & 2)
		{
			animatedPos.y = pAnimFrameData[pJointInfos[i].startIndex + j];
			++j;
		}

		// Tz
		if (pJointInfos[i].flags & 4)
		{
			animatedPos.z = pAnimFrameData[pJointInfos[i].startIndex + j];
			++j;
		}

		// Qx
		if (pJointInfos[i].flags & 8)
		{
			animatedOrient.x = pAnimFrameData[pJointInfos[i].startIndex + j];
			++j;
		}

		// Qy
		if (pJointInfos[i].flags & 16)
		{
			animatedOrient.y = pAnimFrameData[pJointInfos[i].startIndex + j];
			++j;
		}

		// Qz
		if (pJointInfos[i].flags & 32)
		{
			animatedOrient.z = pAnimFrameData[pJointInfos[i].startIndex + j];
			++j;
		}

		// Compute orient quaternion's w value
		animatedOrient.calculateS();

		// NOTE: we assume that this joint's parent has
		// already been calculated, i.e. joint's ID should
		// never be smaller than its parent ID.
		JointMd5* joint = &anim.frames[frameIndex].m_joints[i];

		int parent = pJointInfos[i].parent;
		joint->parent = parent;
		joint->name = pJointInfos[i].name;

		// Has parent?
		if (joint->parent < 0)
		{
			joint->pos = animatedPos;
			joint->orientation = animatedOrient;

			// rotating the model with 90 degrees over 1,0,0
			Quat rotQuat = Quat::fromAxis(-90.0f, vec3(1, 0, 0));

			joint->pos = rotQuat.rotateVec(joint->pos);
			joint->pos *= scale;

			joint->orientation = rotQuat * joint->orientation;
			joint->orientation.normalize();
		}
		else
		{
			JointMd5* parentJoint = &anim.frames[frameIndex].m_joints[parent];

			vec3 rpos;
			animatedPos *= scale;
			rpos = parentJoint->orientation.rotateVec(animatedPos);
			joint->pos = rpos + parentJoint->pos;

			joint->orientation = parentJoint->orientation * animatedOrient;
			joint->orientation.normalize();
		}
	}

	return true;
}

inline vec3 computeTriangleNormal(const vec3& p1, const vec3& p2, const vec3& p3)
{
	vec3 vec1(p2 - p1);
	vec3 vec2(p3 - p1);

	vec3 result = vec1.cross(vec2);
	result.normalize();

	return result;
}

/**
 * Computes the normals of the weights from the bind pose.
 */
void ModelMd5::computeWeightNormals()
{
	for (uint m = 0; m < m_numMeshes; m++)
	{
		std::vector<vec3> bindposeVerts(m_meshes[m].numVertices);
		std::vector<vec3> bindposeNorms(m_meshes[m].numVertices);

		for (uint i = 0; i < m_meshes[m].numVertices; ++i)
		{
			// Zero out final vertex position and final vertex normal
			bindposeVerts[i] = vec3(0.0f);
			bindposeNorms[i] = vec3(0.0f);

			VertexMd5& currentVertex = m_meshes[m].pVertices[i];

			for (int j = 0; j < currentVertex.weightCount; ++j)
			{
				const Weight* pWeight = &m_meshes[m].pWeights[currentVertex.weightIndex + j];
				JointMd5* pJoint = &m_pBaseFrame->m_joints[pWeight->joint];

				vec3 wv = pWeight->pos;

				// Calculate transformed vertex for this weight
				wv = pJoint->orientation.rotateVec(wv);

				bindposeVerts[i] += (pJoint->pos + wv) * pWeight->w;
			}
		}

		// Compute triangle normals
		for (uint i = 0; i < m_meshes[m].numTriangles; ++i)
		{
			const poly3* pTri = &m_meshes[m].pTriangles[i];
			vec3 triNorm = -computeTriangleNormal(bindposeVerts[pTri->a], bindposeVerts[pTri->b], bindposeVerts[pTri->c]);

			for (int j = 0; j < 3; j++)
			{
				bindposeNorms[pTri->at(j)] += triNorm;
			}
		}

		// "Average" the surface normals, by normalizing them
		for (uint i = 0; i < m_meshes[m].numVertices; ++i)
		{
			bindposeNorms[i].normalize();
		}

		// Zero out all weight normals
		for (uint i = 0; i < m_meshes[m].numWeights; ++i)
		{
			m_meshes[m].pWeights[i].normal = vec3(0.0f);
		}

		// Compute weight normals by invert-transforming the normal by the bone-space matrix
		for (uint i = 0; i < m_meshes[m].numVertices; ++i)
		{
			VertexMd5& currentVertex = m_meshes[m].pVertices[i];

			for (int j = 0; j < currentVertex.weightCount; ++j)
			{
				Weight* pWeight = &m_meshes[m].pWeights[currentVertex.weightIndex + j];
				const JointMd5* pJoint = &m_pBaseFrame->m_joints[pWeight->joint];

				vec3 wn = bindposeNorms[i];

				// Compute inverse quaternion rotation
				Quat invRot = pJoint->orientation.conjugate();
				invRot.normalize();
				wn = invRot.rotateVec(wn);

				//pWeight->normal += wn;
				pWeight->normal += wn;
			}
		}

		// Normalize all weight normals
		for (uint i = 0; i < m_meshes[m].numWeights; ++i)
		{
			m_meshes[m].pWeights[i].normal.normalize();
		}
	}
}

/**
 * Computes the tangents of the weights from the bind pose.
 */
void ModelMd5::computeWeightTangents()
{
	for (uint m = 0; m < m_numMeshes; m++)
	{
		std::vector<vec3> bindposeVerts(m_meshes[m].numVertices);
		std::vector<vec3> bindposeNorms(m_meshes[m].numVertices);
		std::vector<vec3> bindposeTangents(m_meshes[m].numVertices);
		std::vector<vec3> bindposeBitangents(m_meshes[m].numVertices);

		for (uint i = 0; i < m_meshes[m].numVertices; ++i)
		{
			// Zero out final vertex position and final vertex normal
			bindposeVerts[i] = vec3(0.0f);
			bindposeNorms[i] = vec3(0.0f);
			bindposeTangents[i] = vec3(0.0f);
			bindposeBitangents[i] = vec3(0.0f);

			const VertexMd5& currentVertex = m_meshes[m].pVertices[i];

			for (int j = 0; j < currentVertex.weightCount; ++j)
			{
				const Weight* pWeight = &m_meshes[m].pWeights[currentVertex.weightIndex + j];
				const JointMd5* pJoint = &m_pBaseFrame->m_joints[pWeight->joint];

				vec3 wv = pWeight->pos;
				wv = pJoint->orientation.rotateVec(wv);
				bindposeVerts[i] += (pJoint->pos + wv) * pWeight->w;

				vec3 wn = pWeight->normal;
				wn = pJoint->orientation.rotateVec(wn);
				bindposeNorms[i] += wn * pWeight->w;
			}
		}

		for (uint i = 0; i < m_meshes[m].numTriangles; ++i)
		{
			const poly3* pTri = &m_meshes[m].pTriangles[i];

			vec3 triTang, triBitang;

			texCoord t1 = m_meshes[m].pVertices[pTri->a].texCoord;
			texCoord t2 = m_meshes[m].pVertices[pTri->b].texCoord;
			texCoord t3 = m_meshes[m].pVertices[pTri->c].texCoord;

			calculateTangent(bindposeVerts[pTri->a], bindposeVerts[pTri->b], bindposeVerts[pTri->c], t1, t2, t3, triTang, triBitang);

			//if (!calculateTangent(bindposeVerts[pTri->a], bindposeVerts[pTri->b], bindposeVerts[pTri->c], t1, t2, t3, triTang, triBitang)) {
			//	if (t1.u == t2.u && t1.v == t2.v)
			//		TRACE_ERROR("NAN: 1 <" << pTri->a << ">  -  2 <" << pTri->b << ">", 0);
			//	if (t1.u == t3.u && t1.v == t3.v)
			//		TRACE_ERROR("NAN: 1 <" << pTri->a << ">  -  3 <" << pTri->c << ">", 0);
			//	if (t2.u == t3.u && t2.v == t3.v)
			//		TRACE_ERROR("NAN: 2 <" << pTri->b << ">  -  3 <" << pTri->c << ">", 0);
			//}

			for (uint j = 0; j < 3; j++)
			{
				bindposeTangents[pTri->at(j)] += triTang;
				bindposeBitangents[pTri->at(j)] += triBitang;
			}
		}

		for (uint i = 0; i < m_meshes[m].numVertices; ++i)
		{
			const vec3& n = bindposeNorms[i];
			const vec3& t = bindposeTangents[i];

			bindposeTangents[i] = (t - n * n.dot(t));
			bindposeTangents[i].normalize();

			if ((n.cross(t)).dot(bindposeBitangents[i]) < 0.0f)
			{
				bindposeTangents[i] = -bindposeTangents[i];
			}
		}

		// Zero out all weight normals
		for (uint i = 0; i < m_meshes[m].numWeights; ++i)
		{
			m_meshes[m].pWeights[i].tangent = vec3(0.0f);
		}

		// Compute weight normals by invert-transforming the normal
		// by the bone-space matrix
		for (uint i = 0; i < m_meshes[m].numVertices; ++i)
		{
			const VertexMd5& currentVertex = m_meshes[m].pVertices[i];

			for (int j = 0; j < currentVertex.weightCount; ++j)
			{
				Weight* pWeight = &m_meshes[m].pWeights[currentVertex.weightIndex + j];
				const JointMd5* pJoint = &m_pBaseFrame->m_joints[pWeight->joint];

				vec3 wt = bindposeTangents[i];

				// Compute inverse quaternion rotation
				Quat invRot = pJoint->orientation.conjugate();
				invRot.normalize();
				wt = invRot.rotateVec(wt);

				pWeight->tangent += wt;
			}
		}

		// Normalize all weight normals
		for (uint j = 0; j < m_meshes[m].numWeights; ++j)
		{
			m_meshes[m].pWeights[j].tangent.normalize();
		}
	}
}

void ModelMd5::prepareMesh(const MeshMd5& mesh, std::vector<JointMd5>& skeleton)
{
	static const vec3 scale(1.2);

	// TMP
	if (m_hasStanceChanged)
	{
		//if (1) {
		vec3 wv, wn, wt;
		// Setup vertices
		for (uint i = 0; i < mesh.numVertices; i++)
		{
			vec3 finalVertex;
			vec3 finalNormal;
			vec3 finalTangent;

			// Calculate final vertex to draw with weights
			for (int j = 0; j < mesh.pVertices[i].weightCount; j++)
			{
				const Weight* pWeight = &mesh.pWeights[mesh.pVertices[i].weightIndex + j];
				const JointMd5* pJoint = &skeleton[pWeight->joint];

				//if (pJoint->updated) {

				// Calculate transformed vertex for this weight
				// The sum of all weight->bias should be 1.0
				wv = pJoint->orientation.rotateVec(pWeight->pos) * scale;
				finalVertex += (pJoint->pos + wv) * pWeight->w;

				// Calculate transformed normal for this weight
				wn = pJoint->orientation.rotateVec(pWeight->normal);
				finalNormal += wn * pWeight->w;

				wt = pJoint->orientation.rotateVec(pWeight->tangent);
				finalTangent += wt * pWeight->w;

				//}
			}

			m_pVertexArrayDynamic[i] = Vertex(finalVertex, -finalNormal, finalTangent, (-finalNormal).cross(finalTangent), mesh.pVertices[i].texCoord);
		}
		m_hasStanceChanged = false;

		glBindBuffer(GL_ARRAY_BUFFER, m_verticesVboId);
		glBufferData(GL_ARRAY_BUFFER, mesh.numVertices * sizeof(Vertex), m_pVertexArrayDynamic, GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.numTriangles * 3 * sizeof(GLuint), mesh.pTriangles, GL_STREAM_DRAW);
	}
}

void ModelMd5::interpolateSkeletons(AnimMd5& anim, int frame1, int frame2, float interp)
{
	for (uint i = 0; i < m_numJoints; i++)
	{
		const JointMd5& jointFrame1 = anim.frames[frame1].m_joints[i];
		const JointMd5& jointFrame2 = anim.frames[frame2].m_joints[i];

		JointMd5& currentJoint = m_pCurrentFrame->m_joints[i];
		currentJoint.pos = jointFrame1.pos + (jointFrame2.pos - jointFrame1.pos) * interp;
		currentJoint.orientation = Quat::slerp(jointFrame1.orientation, jointFrame2.orientation, interp);
		currentJoint.orientation.normalize();
	}
	m_hasStanceChanged = true;
}

void ModelMd5::renderSkeleton()
{
#ifdef DEBUG_TBN
	glBegin(GL_LINES);
	for (int i = 0; i < m_numMeshes; i++)
	{
		prepareMesh(m_meshes[i], m_pCurrentFrame->m_joints);
		for (int j = 0; j < m_meshes[i].numVertices; j++)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(VEC3_TO_F3(m_pVertexArray[j]));
			glVertex3f(VEC3_TO_F3((m_pVertexArray[j] + m_pNormalArray[j] / 2.0f)));

			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(VEC3_TO_F3(m_pVertexArray[j]));
			glVertex3f(VEC3_TO_F3((m_pVertexArray[j] + m_pTangentArray[j] / 2.0f)));

			glColor3f(0.0f, 1.0f, 0.0f);
			vec3 b = m_pNormalArray[j].cross(m_pTangentArray[j]);
			glVertex3f(VEC3_TO_F3(m_pVertexArray[j]));
			glVertex3f(VEC3_TO_F3((m_pVertexArray[j] + b / 2.0f)));
		}
	}
	glEnd();
#endif

	//glPushMatrix();
	//glDisable(GL_DEPTH_TEST);

	//// Draw each joint
	//glPointSize(10.0f);
	//glColor3f(1.0f, 1.0f, 1.0f);

	//glBegin(GL_POINTS);
	//for (int i = 0; i < m_numJoints; i++) {
	//	glVertex3f(VEC3_TO_F3(m_pCurrentFrame->m_joints[i].pos));
	//}
	//glEnd();
	//glPointSize(1.0f);
	//glLineWidth(5.0f);

	//// Draw each bone
	//glColor3f(0.0f, 1.0f, 0.0f);
	//glBegin(GL_LINES);
	//for (int i = 0; i < m_numJoints; ++i) {
	//	if (m_pCurrentFrame->m_joints[i].parent != -1) {
	//		glVertex3f(VEC3_TO_F3(m_pCurrentFrame->m_joints[m_pCurrentFrame->m_joints[i].parent].pos));
	//		glVertex3f(VEC3_TO_F3(m_pCurrentFrame->m_joints[i].pos));
	//	}
	//}
	//glEnd();

	//glEnable(GL_DEPTH_TEST);
	//glPopMatrix();
}

void ModelMd5::render()
{
	// renderSkeleton();
	for (uint i = 0; i < m_numMeshes; i++)
	{
		prepareMesh(m_meshes[i], m_pCurrentFrame->m_joints);
		glDrawElements(GL_TRIANGLES, m_meshes[i].numTriangles * 3, GL_UNSIGNED_INT, 0);
	}
}

void ModelMd5::renderSubset(const uint subset)
{
	// renderSkeleton();
	prepareMesh(m_meshes[subset], m_pCurrentFrame->m_joints);
	glDrawElements(GL_TRIANGLES, m_meshes[subset].numTriangles * 3, GL_UNSIGNED_INT, 0);
}

void ModelMd5::animate(const float dt)
{
	//if (m_pRagdoll && !m_pRagdoll->isAlive())
	//{
	//	doRagdoll();
	//	return;
	//}

	AnimMd5* pAnim = &m_animations[m_currentAnimationName];
	AnimMd5Info& animInfo = pAnim->animInfo;

	int maxFrames = pAnim->numFrames - 1;

	if (m_isAnimationOn)
	{
		animInfo.lastTime += dt;
	}


	// move to next frame
	if (animInfo.lastTime >= animInfo.maxTime)
	{
		animInfo.currentFrame++;
		animInfo.nextFrame++;
		animInfo.lastTime = 0.0;

		if (m_isAnimationLooped)
		{
			if (animInfo.currentFrame > maxFrames)
			{
				animInfo.currentFrame = 0;
			}

			if (animInfo.nextFrame > maxFrames)
			{
				animInfo.nextFrame = 0;
			}
		}
		else
		{
			if (animInfo.currentFrame > maxFrames)
			{
				animInfo.currentFrame = maxFrames;
				m_isAnimationOn = false;
			}
			if (animInfo.nextFrame > maxFrames)
			{
				animInfo.nextFrame = maxFrames;
			}
		}
	}

	interpolateSkeletons(*pAnim, animInfo.currentFrame, animInfo.nextFrame, animInfo.lastTime * pAnim->frameRate);
}


void ModelMd5::kill()
{
	if (m_isAnimationOn)
	{
		m_isAnimationOn = false;
	}
}


// getters-setters

float ModelMd5::getFrame() const
{
	if (m_animations.find(m_currentAnimationName) != m_animations.end())
	{
		return m_animations.at(m_currentAnimationName).animInfo.currentFrame;
	}

	return 0;
}

void ModelMd5::setFrame(float time)
{
	AnimMd5* anim = &m_animations.at(m_currentAnimationName);

	anim->animInfo.currentFrame = time;
	anim->animInfo.nextFrame = time + 1;

	int maxFrames = anim->numFrames - 1;

	if (anim->animInfo.currentFrame > maxFrames)
	{
		anim->animInfo.currentFrame = 0;
	}

	if (anim->animInfo.nextFrame > maxFrames)
	{
		anim->animInfo.nextFrame = 0;
	}
}

float ModelMd5::getFrameTime() const
{
	if (m_animations.find(m_currentAnimationName) != m_animations.end())
	{
		return m_animations.at(m_currentAnimationName).animInfo.lastTime;
	}

	return 0;
}

void ModelMd5::setFrameTime(float time)
{
	m_animations.at(m_currentAnimationName).animInfo.lastTime = time;
}

} // namespace models
