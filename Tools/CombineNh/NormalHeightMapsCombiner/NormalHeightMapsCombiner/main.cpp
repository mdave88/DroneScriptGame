
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <IL/il.h>
#include <IL/ilut.h>

#include "helpers.h"

string filename;

ILubyte* heightMapData;
ILubyte* normalMapData;

int width = 0, height = 0;
ILubyte* finalData = NULL;



void devilInit() {
	ilInit();
	iluInit();
	ilutInit();
}

bool loadImages(const string& filename, const string& ext) {
	string heightMapFile = string(filename) + "_HEIGHT." + ext;
	string normalMapFile = string(filename) + "_Normal." + ext;

	// heightmap
	ilLoadImage(heightMapFile.c_str());
	ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);

	ILenum error;
	if ((error = ilGetError()) != IL_NO_ERROR) {
		cout << "\t Missing height map for " << filename << endl;
		return false;
	}

	if (ext != "jpg")
		iluFlipImage();
	heightMapData = ilGetData();

	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	finalData = new ILubyte[width * height * 4];


	cout << "\t dimensons: " << width << ", " << height << endl;

	// normalmap
	ilBindImage(0);
	ilLoadImage(normalMapFile.c_str());
	ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);

	if ((error = ilGetError()) != IL_NO_ERROR) {
		cout << "\t Missing normal map for " << filename << endl;
		return false;
	}

	if (ext != "jpg")
		iluFlipImage();
	normalMapData = ilGetData();

	return true;
}

bool buidNormalHeightMap() {
	int finalIndex = 0;
	for (int i = 0; i < width * height * 3; i += 3) {
		finalData[finalIndex++] = normalMapData[i    ];
		finalData[finalIndex++] = normalMapData[i + 1];
		finalData[finalIndex++] = normalMapData[i + 2];

		// alpha from heightmap
		finalData[finalIndex++] = heightMapData[i];
	}

	if (finalIndex != width * height * 4) {
		cout << "hiba" << endl;
		return false;
	}

	return true;
}

bool saveImage(string filename, ILubyte* data) {
	ILuint image;
	ilGenImages(1, &image);
	ilBindImage(image);

	ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, data);

	iluFlipImage();

	ilEnable(IL_FILE_OVERWRITE); 
	ilSaveImage(filename.c_str());
		
	ILenum error;
	if ((error = ilGetError()) != IL_NO_ERROR) {
		cout << "\tDevIL error" << endl;
		cout << "\t\t " << error << ": " << iluErrorString(error) << endl;
		return false;
	}

	ilDeleteImages(1, &image);

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
	string ext = getExtension(filename);
	filename = filename.substr(0, filename.find_last_of('_'));

	string filename_nh = getFileName(filename) + "_nh.png";

	if (loadImages(filename, ext))
		if (buidNormalHeightMap())
			saveImage(filename_nh, finalData);
	

	if (finalData)
		delete[] finalData;

	cout <<  endl;

	return 0;
}
