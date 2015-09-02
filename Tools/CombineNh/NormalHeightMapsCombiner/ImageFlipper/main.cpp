
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <IL/il.h>
#include <IL/ilut.h>

#include "helpers.h"


void devilInit() {
	ilInit();
	iluInit();
	ilutInit();
}

bool loadImage(const string& filename) {
	ilLoadImage(filename.c_str());
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	ILenum error;
	if ((error = ilGetError()) != IL_NO_ERROR) {
		cout << "\t Missing map " << filename << endl;
		return false;
	}

	iluFlipImage();

	ilEnable(IL_FILE_OVERWRITE); 
	ilSaveImage(filename.c_str());
		
	if ((error = ilGetError()) != IL_NO_ERROR) {
		cout << "\tDevIL error" << endl;
		cout << "\t\t " << error << ": " << iluErrorString(error) << endl;
		return false;
	}

	return true;
}

int main(int argc, char* argv[]) {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	if (argc > 2) {
		for (int i = 2; i < argc; i++) {
			_CrtSetBreakAlloc(atoi(argv[i]));
		}
	}
#endif
	devilInit();

	string filename = argv[1];
	cout << "processing " << filename << endl;

	loadImage(filename);

	return 0;
}
