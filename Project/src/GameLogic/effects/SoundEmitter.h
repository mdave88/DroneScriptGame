#pragma once

#include "GameLogic/Node.h"
#include "Sound/SoundSource.h"

class SoundSource;

namespace effects
{

class SoundEmitter : public Node
{
public:
	SoundEmitter(const std::string& filename = "", const vec3& pos = vec3(0.0f), const vec3& vel = vec3(0.0f), bool loop = false);
	virtual ~SoundEmitter();

	virtual void render(const graphics::RenderContext& context);
	virtual void animate(const float dt);

	// wrapper functions for SoundSource
	void play();
	void stop();

	// register to lua
	static void registerMethodsToLua();

private:
	bool			m_isLooped;
	SoundSourcePtr	m_soundSource;
};

typedef std::shared_ptr<SoundEmitter> SoundEmitterPtr;

} //namespace effects
