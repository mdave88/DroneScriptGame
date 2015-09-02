#include "GameStdAfx.h"
#include "Models/mesh/AnimatedMesh.h"


namespace models
{

AnimatedMesh::AnimatedMesh()
	: m_isAnimationOn(true)
	, m_animationSpeed(1.0f)
	, m_isAnimationLooped(true)
	, m_hasStanceChanged(true)
	, m_currentAnimationName("idle")
{
}

AnimatedMesh::~AnimatedMesh()
{
}

void AnimatedMesh::play()
{
	if (!m_isAnimationOn)
	{
		restart();
	}
	m_isAnimationOn = true;
}

void AnimatedMesh::stop()
{
	m_isAnimationOn = false;
	restart();
}

void AnimatedMesh::pause()
{
	m_isAnimationOn = false;
}

void AnimatedMesh::restart()
{
	setFrame(0);
	setFrameTime(0);
}

float AnimatedMesh::getFrame() const
{
	if (m_animations.find(m_currentAnimationName) != m_animations.end())
	{
		return m_animations.at(m_currentAnimationName).animInfo.currentFrame;
	}

	return 0;
}

void AnimatedMesh::setFrame(float time)
{
	AnimData* anim = &m_animations.at(m_currentAnimationName);

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

float AnimatedMesh::getFrameTime() const
{
	if (m_animations.find(m_currentAnimationName) != m_animations.end())
	{
		return m_animations.at(m_currentAnimationName).animInfo.lastTime;
	}

	return 0;
}

void AnimatedMesh::setFrameTime(float time)
{
	m_animations.at(m_currentAnimationName).animInfo.lastTime = time;
}

bool AnimatedMesh::isAnimationLooping() const
{
	return m_isAnimationLooped;
}

float AnimatedMesh::getAnimationSpeed() const
{
	return m_animationSpeed;
}

void AnimatedMesh::setAnimationSpeed(float animationSpeed)
{
	m_animationSpeed = animationSpeed;
}

void AnimatedMesh::setAnimationLooping(bool loopAnimation)
{
	m_isAnimationLooped = loopAnimation;
}

std::string AnimatedMesh::getCurrentAnimationName() const
{
	return m_currentAnimationName;
}

void AnimatedMesh::setCurrentAnimationName(const std::string& animationName)
{
	m_currentAnimationName = animationName;
}

bool AnimatedMesh::isPlaying() const
{
	return m_isAnimationOn;
}

} // namespace models
