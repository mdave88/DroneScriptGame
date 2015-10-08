#include "Client/Client.h"


// render
void render();
void idleFunc();
void reshape(int iwidth, int iheight);

// input
void keyDown(uint8_t key, int x, int y);
void keyUp(uint8_t key, int x, int y);
void special(int key, int x, int y);
void specialUp(int key, int x, int y);

void mouseMove(int x, int y);
void mouseDrag(int x, int y);
void mouseAction(int button, int state, int x, int y);

void entryFunc(int state);


int clientMain(const int argc, char* argv[], const bool isThickClient = false);
