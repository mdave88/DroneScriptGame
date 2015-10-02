#pragma once

namespace utils
{
	const char* int2str(const int32_t num);
	const char* formatStr(const char* fmt, ...);

	std::string intToStr(int i);
	std::string floatToStr(float f, int precision = 4);
	std::string vec3ToStr(const vec3& vec);

	float strToFloat(const std::string& str);
	vec3 strToVec3(const std::string& str);

	// float compression/decompression
	short float32Tofloat16(float num);
	float float16Tofloat32(int n);

	vec3 vec3F16_to_vec3F32(int x, int y, int z);

	template<typename T>
	bool is_ieee_nan(const T& v)
	{
		return !(v == v);
	}

	void toLowerCase(std::string& str);
	std::vector<std::string> tokenize(const std::string& str, const std::string& delim);

	template<typename T>
	std::string getClassName(const T& t)
	{
		std::string name = typeid(t).name();
		name = name.substr(name.find_last_of(' ') + 1, name.size());
		return name;
	}
	
	
	namespace file
	{
		std::string getDir(const std::string& path);
		std::string getParentDir(const std::string& path);
		std::string getFileName(const std::string& path);
		std::string getFileNameWithExtension(const std::string& path);
		std::string getExtension(const std::string& path);


		std::vector<std::string> parseLine(std::ifstream& file);
		std::vector<std::string> findKeyword(std::ifstream& file, const std::string& keyword);
		void skipLine(std::ifstream& file);

		bool existFile(const std::string& filename);
		char* readFile(const std::string& filename);

		int getInt(std::ifstream& file);
		float getFloat(std::ifstream& file);
	}



#ifdef CLIENT_SIDE
	namespace gfx
	{
		void blankTexture(GLuint* textureId, int size, int channels, int type);

		void orthoMode(int left, int top, int right, int bottom);
		void perspectiveMode();

		void drawCrosshair();

		vec3 printGL(const std::string& text, const vec3& textPos, bool posInPixels = false, int id = 0, int timeToStay = 0);
		void flushScreenTextBuffer();
		vec3 printToScreen(const std::string& text, const vec3& textPos, bool posInPixels = false);
	}
#endif // CLIENT_SIDE
} // namespace utils
