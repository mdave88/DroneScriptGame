
#ifndef SKELETON_H
#define SKELETON_H

namespace models
{

struct JointMd5
{
	std::string	name;
	int			parent;
	vec3		pos;
	Quat		orientation;

	float		height;
	float		height2;

	JointMd5();
	~JointMd5();
};

class FrameMd5
{
public:
	FrameMd5(int numJoints = -1);

	~FrameMd5()
	{
		//TRACE_INFO("FrameMd5_" << this << "destroyed")
	}

public:
	struct BBox
	{
		vec3 minp, maxp;
	};

	BBox					m_bbox;
	std::vector<JointMd5>	m_joints;
};

} // namespace models

#endif // SKELETON_H
