
#include "imageLoad.h"

#include <IL/il.h>
#include <IL/ilut.h>

#include <iostream>
#include <string>

bool enableDebugMessages = false;

void devilInit(bool enableDebugMessages0) {
	ilInit();
	iluInit();

	// this + iluFlipImage() causes crashes when loading tga images
	//ilEnable(IL_ORIGIN_SET);
	//ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	enableDebugMessages = enableDebugMessages0;
}

std::string getExt(const std::string& path) {
	return path.substr(path.find_last_of('.') + 1, path.size());
}

bool loadTexture(const std::string& filename, GLuint& texid, bool resize, int resolutionDiv) {
	ilLoadImage(filename.c_str());
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	if (!resize) glGenTextures(1, &texid);

	glBindTexture(GL_TEXTURE_2D, texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	std::string fileExtension = getExt(filename);

	if (resize) {
		int width  = ilGetInteger(IL_IMAGE_WIDTH) / resolutionDiv;
		int height = ilGetInteger(IL_IMAGE_HEIGHT) / resolutionDiv;

		iluScale(width, height, ilGetInteger(IL_IMAGE_DEPTH));
	}

	if (fileExtension == "bmp" || fileExtension == "tga") {
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, IL_BGR, GL_UNSIGNED_BYTE, ilGetData());
		//iluFlipImage();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
	} else if (fileExtension == "jpg" || fileExtension == "png") {
		iluFlipImage();
		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
	}

	ILenum error;
	if ((error = ilGetError()) != IL_NO_ERROR) {
		if (enableDebugMessages) {
			std::cout << "\tDevIL error <" << filename << ">:" << std::endl;
			std::cout << "\t\t " << error << ": " << iluErrorString(error) << std::endl;
		}

		return false;
	}

	return true;
}

bool loadTexture(const std::string& filename, GLuint& texid, TextureDirectory& textureDirectory, bool resize, int resolutionDiv) {
	if (enableDebugMessages)
		std::cout << "Loading texture <" << filename << ">." << std::endl;

	TextureDirectory::iterator it = textureDirectory.find(filename);
	if (it != textureDirectory.end()) {
		if (enableDebugMessages)
			std::cout << "\tTexture already loaded <" << filename << ">." << std::endl;

		texid = it->second;

		if (!resize) return true;
	}

	if (!loadTexture(filename, texid, resize, resolutionDiv))
		return false;

	textureDirectory[filename] = texid;

	return true;
}

/**
 * Creates a cube map using the given mapname.
 * The namimg convention of the the six texture files are [mapname]_p_[dim].[ext] for positive dimensions and [mapname]_n_[dim].[ext] for negative dimensions.
 *
 * @param mapname			The location and the first part of the name of the texture files.
 * @param extension			The extension of the texture files.
 * @param texid				The texture id in which the generated texture will be stored.
 * @param textureDirectory	The texture collection of the program instance.
 */
bool loadCubeMap(const std::string& mapname, const std::string& extension, GLuint& texid, TextureDirectory& textureDirectory) {
	if (enableDebugMessages)
		std::cout << "Loading cubemap <" << mapname << ">." << std::endl;

	TextureDirectory::iterator it = textureDirectory.find(mapname);
	if (it != textureDirectory.end()) {
		if (enableDebugMessages)
			std::cout << "\tCubemap already loaded." << std::endl;

		texid = it->second;

		return true;
	}

	std::string faceFileName(mapname);

	glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texid);

	GLenum faceTarget[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT };

	for (int i = 0; i < 6; i++) {
		std::string coord;
		switch (i) {
		case 0:
			coord = "_p_x.";
			break;
		case 1:
			coord = "_n_x.";
			break;
		case 2:
			coord = "_p_y.";
			break;
		case 3:
			coord = "_n_y.";
			break;
		case 4:
			coord = "_p_z.";
			break;
		case 5:
			coord = "_n_z.";
			break;
		}

		ilLoadImage((faceFileName + coord + extension).c_str());

		ILenum error;
		if ((error = ilGetError()) != IL_NO_ERROR) {
			if (enableDebugMessages) {
				std::cout << "\tDevIL error:" << std::endl;
				std::cout << "\t\t " << error << ": " << iluErrorString(error) << std::endl;
			}

			return false;
		}

		glTexImage2D(faceTarget[i], 0, 3, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	textureDirectory[mapname] = texid;

	return true;
}

// debug
bool isDevILDebugEnabled() {
	return enableDebugMessages;
}

void setDevILDebug(bool debugEnabled) {
	enableDebugMessages = debugEnabled;
}
