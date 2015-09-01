#include "GameStdAfx.h"
#include "SoundEmitter.h"

#include "GameLogic/Camera.h"


namespace effects
{

SoundEmitter::SoundEmitter(const std::string& filename, const vec3& pos, const vec3& vel, bool loop) : m_isLooped(loop)
{
	m_soundSource = SoundSourcePtr(new SoundSource());

	const std::string soundDirPath = getDataDir() + CONST_STR("Sounds::SoundsDir");
	if (helperfuncs::file::existFile(soundDirPath + filename))
	{
		m_soundSource->load(soundDirPath + filename);
	}

	m_soundSource->set(pos, vel, loop);
}

SoundEmitter::~SoundEmitter()
{
}

/**
* Renders a low-poly sphere at the sound source's position.
*
* @param context The render context.
*/
void SoundEmitter::render(const graphics::RenderContext& context)
{
	// render
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Matrix projMatrix = context.m_pCamera->getProjMatrix();
	glMultMatrixf(projMatrix.getArray());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Matrix viewMatrix = context.m_pCamera->getViewMatrix();
	glMultMatrixf(viewMatrix.getArray());
	glTranslatef(m_pos.x, m_pos.y, m_pos.z);

	graphics::Shader::disableAll();
	glutSolidSphere(0.5, 4, 4);
}

void SoundEmitter::animate(const float dt)
{
	m_soundSource->update();
}

void SoundEmitter::play()
{
	m_soundSource->play();
}

void SoundEmitter::stop()
{
	m_soundSource->stop();
}

// register to lua
void SoundEmitter::registerMethodsToLua()
{
	using namespace luabind;

	class_<SoundEmitter, NodePtr, bases<Node>> thisClass("SoundEmitter");
	thisClass.def(constructor<>());
	thisClass.def(constructor<std::string>());
	thisClass.def(constructor<std::string, vec3>());
	thisClass.def(constructor<std::string, vec3, vec3, bool>());

	REG_FUNC("play", &SoundEmitter::play);
	REG_FUNC("stop", &SoundEmitter::stop);

	module(LuaManager::getInstance()->getState())[thisClass];
}

} // namespace effects
