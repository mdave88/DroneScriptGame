#include "GameStdAfx.h"
#include "Sound/SoundSource.h"
#include "Common/LoggerSystem.h"

#define vec3ToFloatA6(f, v1, v2)	{ f[0]=v1.x; f[1]=v1.y; f[2]=v1.z; f[3]=v2.x; f[4]=v2.y; f[5]=v2.z; }
#define SET_AF3(fa,x,y,z)			{ fa[0]=x; fa[1]=y; fa[2]=z; }
#define SET_AF4(fa,x,y,z,w)			{ fa[0]=x; fa[1]=y; fa[2]=z; fa[3]=w; }


SoundSource::SoundSource(const std::string& filename)
	: m_source(0)
	, m_buffer(0)
	, m_isLooped(false)
	, m_isOpen(false)
{
	load(filename);
}

SoundSource::~SoundSource()
{
	if (m_source)
	{
		alDeleteSources(1, &m_source);
	}

	if (m_buffer)
	{
		alDeleteBuffers(1, &m_buffer);
	}
}

void SoundSource::audioInit()
{
	alutInit(nullptr, 0);

	checkError("Error: failed to initialize alut.", true);

	//std::cout << alcGetString(alcGetContextsDevice(alcGetCurrentContext()), ALC_DEVICE_SPECIFIER) << std::endl;
}

bool SoundSource::load(const std::string& filename)
{
	if (filename.empty())
	{
		return false;
	}

	m_buffer = alutCreateBufferFromFile(filename.c_str());

	ALenum error = alutGetError();
	if (error != ALUT_ERROR_NO_ERROR)
	{
		TRACE_ERROR("Error: failed to load sound file <" << filename.c_str() << ">." << std::endl, 1);
		TRACE_ERROR(alutGetErrorString(error) << std::endl, -1);

		return false;
	}

	alGenSources(1, &m_source);
	alSourcei(m_source, AL_BUFFER, m_buffer);
	alSourcef(m_source, AL_PITCH, 1.0f);
	alSourcef(m_source, AL_GAIN, 1.0f); //volume 0.0 - 1.0

	if (!checkError("Error: failed to load sound file." + filename + ">."))
	{
		return false;
	}

	m_isOpen = true;

#ifdef GX_DEBUG_INFO
	m_debug_fileName = filename;
#endif

	return true;
}

void SoundSource::set(const vec3& pos, const vec3& vel, bool loop)
{
	m_pos = pos;
	m_vel = vel;
	m_isLooped = loop;
}

void SoundSource::play()
{
	ALfloat sourcePos[3];
	ALfloat sourceVel[3];

	SET_AF3(sourcePos, m_pos.x, m_pos.y, m_pos.z);
	SET_AF3(sourceVel, m_vel.x, m_vel.y, m_vel.z);

	alSourcefv(m_source, AL_POSITION, sourcePos);
	alSourcefv(m_source, AL_VELOCITY, sourceVel);
	alSourcei(m_source, AL_LOOPING, m_isLooped);

	alSourcePlay(m_source);

	checkError("Error: failed to play sound file.");
}

void SoundSource::stop()
{
	alSourceStop(m_source);
}

void SoundSource::update()
{
	ALfloat sourcePos[3];
	ALfloat sourceVel[3];

	SET_AF3(sourcePos, m_pos.x, m_pos.y, m_pos.z);
	SET_AF3(sourceVel, m_vel.x, m_vel.y, m_vel.z);

	alSourcefv(m_source, AL_POSITION, sourcePos);
	alSourcefv(m_source, AL_VELOCITY, sourceVel);
	alSourcei(m_source, AL_LOOPING, m_isLooped);

	checkError("Error: update().");
}

void SoundSource::release()
{
	if (m_source)
	{
		alDeleteSources(1, &m_source);
	}

	if (m_buffer)
	{
		alDeleteBuffers(1, &m_buffer);
	}
}

void SoundSource::setListener(const vec3& pos, const vec3& vel, const vec3& ori1, const vec3& ori2)
{
	ALfloat listenerPos[3];
	ALfloat listenerVel[3];
	ALfloat listenerOrientation[6];

	SET_AF3(listenerPos, pos.x, pos.y, pos.z);
	SET_AF3(listenerVel, vel.x, vel.y, vel.z);
	vec3ToFloatA6(listenerOrientation, ori1, ori2);

	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOrientation);

	checkError("Error: setListener().");
}

void SoundSource::audioExit()
{
	ALCcontext* context = alcGetCurrentContext();
	ALCdevice* device = alcGetContextsDevice(context);

	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);

	alutExit();
}

bool SoundSource::isOpen() const
{
	return m_isOpen;
}

bool SoundSource::isPlaying() const
{
	ALenum state;
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);

	checkError("Error: isPlaying().");

	return (state == AL_PLAYING);
}

int SoundSource::getState() const
{
	ALenum state;
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);

	checkError("Error: getState().");

	return state;
}

bool SoundSource::checkError(const std::string& message, const bool isAlError)
{
	ALenum error = isAlError ? alGetError() : alutGetError();
	if (error != AL_NO_ERROR)
	{
		TRACE_ERROR(message, 1);
		TRACE_ERROR((isAlError ? alGetString(error) : alutGetErrorString(error)), -1);

		return false;
	}

	return true;
}
