
#ifndef CONFIGS_H
#define CONFIGS_H

class Configs
{
public:
	int		width0, height0;
	int		width, height;

	bool	m_fullscreen;
	bool	m_lightOn;
	bool	m_shadowsOn;

	float	m_mouseSensitivity;

public:
	Configs();

	bool loadConfigs(const std::string& filename = "settings");
};

#endif // CONFIGS_H
