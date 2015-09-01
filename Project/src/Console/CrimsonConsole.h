
#ifndef GAME_CONSOLE_H
#define GAME_CONSOLE_H

#include <string>

class GameConsole
{
public:
	GameConsole();
	~GameConsole();

	void init(float width, float height);
	void releaseKeys();

	void addKeyword(const std::string& keyword);

	void render();
	void print(const std::string& str);

	bool keyDown(uint8_t key, int x, int y);
	bool keyUp(uint8_t key, int x, int y);
	bool special(int key, int x, int y);
	bool specialUp(int key, int x, int y, bool isGamePaused);

	void mouseMove(int x, int y);
	//void mouseDrag(int x, int y);
	void mouseAction(int button, int state, int x, int y);

	bool isOpen() const;
	std::string getCommand() const;
};

void addKeywordToConsole(const std::string& keyword);

#endif
