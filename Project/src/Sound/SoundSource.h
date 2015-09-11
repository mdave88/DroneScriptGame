#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>


class SoundSource
{
public:
	static void audioInit();
	static void audioExit();
	static void setListener(const vec3& pos, const vec3& vel, const vec3& ori1, const vec3& ori2);

	SoundSource(const std::string& filename = "");
	~SoundSource();

	bool load(const std::string& filename);
	void play();
	void stop();
	void update();
	void release();

	// getters-setters
	void set(const vec3& pos, const vec3& vel, bool loop = false);

	bool isOpen() const;
	bool isPlaying() const;

	int getState() const;

private:
	static bool checkError(const std::string& message, const bool isAlError = true);

protected:
	ALuint		m_buffer;
	ALuint		m_source;

	bool		m_isOpen;
	bool		m_isLooped;

	vec3		m_pos;
	vec3		m_vel;

#ifdef GX_DEBUG_INFO
	std::string	m_debug_fileName;
#endif
};

typedef std::shared_ptr<SoundSource> SoundSourcePtr;
