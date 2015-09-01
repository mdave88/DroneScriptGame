#include "GameStdAfx.h"
#include "Skeleton.h"


namespace models
{

JointMd5::JointMd5()
	:	parent(0),
	    height(0),
	    height2(0)
{
}

JointMd5::~JointMd5()
{
}




FrameMd5::FrameMd5(int numJoints)
{
	if (numJoints != -1)
	{
		m_joints.resize(numJoints);
	}
}

} // namespace models
