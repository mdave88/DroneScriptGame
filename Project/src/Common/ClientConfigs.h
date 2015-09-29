#pragma once

class ClientConfigs
{
public:
	int		width0, height0;
	int		width, height;

	bool	m_fullscreen;
	bool	m_lightOn;
	bool	m_shadowsOn;

	float	m_mouseSensitivity;

public:
	ClientConfigs();

	bool loadConfigs(const std::string& filename = "settings");
};

