#include "GameStdAfx.h"
#include "Common/ClientConfigs.h"


ClientConfigs::ClientConfigs()
	: width0(400)
	, height0(400)
	, width(400)
	, height(400)
	, m_fullscreen(false)
	, m_lightOn(false)
	, m_shadowsOn(false)
	, m_mouseSensitivity(1.0f)
{
}

//void removeEndlineChar(std::string& str) {
//	int pos = str.find('\n', 0);
//	if (pos > 0) {
//		str.erase(pos, 1);
//	}
//}

/**
 * Loading the configurations from the settings file.
 * Using old C style file reading because it works the same way on linux/windows.
 */
bool ClientConfigs::loadConfigs(const std::string& filename)
{
	m_lightOn = false;
	m_shadowsOn = false;

	std::ifstream file((getDataDir() + "settings/" + filename).c_str(), std::ios::in);
	if (!file.is_open())
	{
		TRACE_ERROR("Error: Cannot open config file.", 0);
		return false;
	}

	std::string str;

	getline(file, str);
	sscanf(str.c_str(), "width0: %d", &width0);
	getline(file, str);
	sscanf(str.c_str(), "height0: %d", &height0);

	getline(file, str);
	sscanf(str.c_str(), "width: %d", &width);
	getline(file, str);
	sscanf(str.c_str(), "height: %d", &height);
	getline(file, str);
	sscanf(str.c_str(), "mouse sen: %f", &m_mouseSensitivity);

	getline(file, str);
	m_fullscreen = !str.compare(0, str.size(), "fullScreenOn");

	getline(file, str);
	m_lightOn = !str.compare(0, str.size(), "lightOn");

	getline(file, str);
	m_shadowsOn = !str.compare(0, str.size(), "shadowsOn");

	return true;
}
