#pragma once

#include "Models/mesh/Mesh.h"

namespace models
{

struct AnimInfo
{
	int		currentFrame;
	int		nextFrame;
	double	lastTime;
	double	maxTime;

	AnimInfo()
		: currentFrame(0)
		, nextFrame(1)
		, lastTime(0)
		, maxTime(0.0)
	{
	}
};

struct AnimData
{
	int			numFrames;
	int			frameRate;
	AnimInfo	animInfo;

	AnimData() : numFrames(0), frameRate(0)
	{
	}

	~AnimData()
	{
	}
};

typedef std::map<const std::string, AnimData> AnimDataMap;

class AnimatedMesh : public Mesh
{
public:
	AnimatedMesh();
	virtual ~AnimatedMesh();

	virtual void play();
	virtual void stop();
	virtual void pause();
	virtual void restart();

	virtual float getFrame() const;
	virtual void setFrame(float time);

	virtual float getFrameTime() const;
	virtual void setFrameTime(float time);

	virtual float getAnimationSpeed() const;
	virtual void setAnimationSpeed(float animationSpeed);

	virtual std::string getCurrentAnimationName() const;
	virtual void setCurrentAnimationName(const std::string& animationName);

	virtual bool isAnimationLooping() const;
	virtual void setAnimationLooping(bool loopAnimation);

	virtual bool isPlaying() const;


protected:
	bool			m_isAnimationOn;
	bool			m_isAnimationLooped;
	float			m_animationSpeed;
	std::string		m_currentAnimationName;
	AnimDataMap		m_animations;

	bool			m_hasStanceChanged;
};

} //namespace models
