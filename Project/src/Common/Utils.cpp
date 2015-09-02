#include "GameStdAfx.h"
#include "Common/Utils.h"

#include "Graphics/shaders/Shader.h"

#include <atomic>
#include <boost/filesystem.hpp>

#ifdef WIN32
#define snprintf sprintf_s
#endif


namespace utils
{

#ifdef CLIENT_SIDE
namespace gfx
{
struct ScreenText
{
	int			id;
	std::string text;
	vec3		textPos;
	bool		posInPixels;
	int			timeToStay;

	ScreenText(const std::string& text = "", const vec3& textPos = vec3(0.0f), bool posInPixels = false, int id = 0, int timeToStay = 0) : id(id), text(text), textPos(textPos), posInPixels(posInPixels)
	{
		this->timeToStay = glutGet(GLUT_ELAPSED_TIME) + timeToStay;
	}
};

typedef std::map<int, ScreenText*> ScreenTextPtrMap;

ScreenTextPtrMap timedScreenTexts;
std::vector<ScreenText> screenTextBuffer;


void blankTexture(GLuint* texid, int size, int channels, int type)
{
	GLuint* pTexture = nullptr;
	pTexture = new GLuint[size * size * channels];
	memset(pTexture, 0, size * size * channels * sizeof(GLuint));

	glGenTextures(1, texid);
	glBindTexture(GL_TEXTURE_2D, *texid);
	glTexImage2D(GL_TEXTURE_2D, 0, channels, size, size, 0, type, GL_UNSIGNED_BYTE, pTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	delete[] pTexture;
}


// viewmodes
/**
 * Setting the view mode to orthographic.
 */
void orthoMode(int left, int top, int right, int bottom)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(left, right, bottom, top, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/**
 * Setting the view mode to perspective.
 */
void perspectiveMode()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

/**
 * Draws a simple crosshair.
 */
void drawCrosshair()
{
	graphics::Shader* activeShader = graphics::Shader::getActiveShader();
	graphics::Shader::disableAll();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glLineWidth(1.0f);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	GLboolean lightOn = glIsEnabled(GL_LIGHTING);

	if (lightOn)
	{
		glDisable(GL_LIGHTING);
	}

	glPushMatrix();
	glTranslatef(-0.5f, -0.5f, -0.8f);

	glLineWidth(2.0f);

	// crosslines
	glBegin(GL_LINES);
	glVertex2f(0.49f, 0.5f);
	glVertex2f(0.511f, 0.5f);
	glVertex2f(0.5f, 0.49f);
	glVertex2f(0.5f, 0.511f);
	glEnd();

	glPopMatrix();

	glPopAttrib();

	graphics::Shader::bindShader(activeShader);
}

/**
 * Prints the given text on the screen at the given position.
 *
 * @param	text		the text to be rendered.
 * @param	textPos		the position of the text on the screen.
 * @param	posInPixels	the position of the text is given in pixels or in (row, col) format.
 *
 * @return	the position of the last written character.
 */
vec3 printToScreen(const std::string& text, const vec3& textPos, bool posInPixels)
{
	// scale the coordinates according to the value of posInPixels
	vec3 screenPos;
	if (posInPixels)
	{
		screenPos = textPos;    // pixels
	}
	else
	{
		screenPos = textPos * vec3(8, 13, 0);    // (row, col)
	}


	graphics::Shader* activeShader = graphics::Shader::getActiveShader();
	graphics::Shader::disableAll();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_CUBE_MAP);

	if (glIsEnabled(GL_LIGHTING))
	{
		glDisable(GL_LIGHTING);
	}

	// switch to ortho mode
	glPushMatrix();
	orthoMode(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

	GLint rasterPosf[4];
	// print the characters on the screen
	void* font = GLUT_BITMAP_8_BY_13;
	vec3 color(1, 1, 1);

	glColor3f(color.x, color.y, color.z);
	glRasterPos2f(screenPos.x, screenPos.y);
	for (size_t i = 0; i < text.length(); i++)
	{
		switch (text[i])
		{
			case '\n':
				screenPos.y += 13;
				glColor3f(color.x, color.y, color.z);
				glRasterPos2f(screenPos.x, screenPos.y);
				break;

			case '\t':
				glGetIntegerv(GL_CURRENT_RASTER_POSITION, rasterPosf);
				glRasterPos2f(((rasterPosf[0] / 8 / 4) * 4 + 4) * 8, screenPos.y);
				break;

			case '<':
				if (text.substr(i + 1, 5) == "color")
				{
					// get color vector
					size_t tagEnd = text.find('>', i);

					std::string colorVectorStr = text.substr(i + 6, tagEnd - (i + 6));
					color = conversion::strToVec3(colorVectorStr);

					i = tagEnd;
				}
				else if (text.substr(i + 1, 6) == "/color")
				{
					color = vec3(1, 1, 1);
					i += 7;
				}

				break;

			default:
				// update the color (have to be used with the rasterpos)
				glColor3f(color.x, color.y, color.z);
				glGetIntegerv(GL_CURRENT_RASTER_POSITION, rasterPosf);
				glRasterPos2f(rasterPosf[0], screenPos.y);

				glutBitmapCharacter(font, text[i]);
		}
	}

	// get the final raster pos
	glGetIntegerv(GL_CURRENT_RASTER_POSITION, rasterPosf);

	perspectiveMode();
	glPopMatrix();

	glPopAttrib();

	graphics::Shader::bindShader(activeShader);

	//return vec3(rasterPosf[0], glutGet(GLUT_WINDOW_HEIGHT) - rasterPosf[1], 0);
	if (posInPixels)
	{
		return vec3(rasterPosf[0], screenPos.y, 0);
	}
	else
	{
		return vec3(rasterPosf[0] / 8, screenPos.y / 13, 0);
	}
}

/**
 * Prints the given text on the screen at the given position.
 *
 * @param	screenText	the screenText structure.
 *
 * @return	the position of the last written character.
 */
vec3 printToScreen(const ScreenText& screenText)
{
	return printToScreen(screenText.text, screenText.textPos, screenText.posInPixels);
}

/**
 * Adds the given text to the screenTextBuffer. The buffer can be flushed with flushScreenTextBuffer().
 *
 * @param	text		the text to be rendered.
 * @param	textPos		the position of the text on the screen.
 * @param	posInPixels	the position of the text is given in pixels or in (row, col) format.
 * @param	id			the id of a timed event.
 * @param	timeToStay	the amount of milliseconds the text is on the screen.
 *
 * @return	the position of the last written character.
 */
vec3 printGL(const std::string& text, const vec3& textPos, bool posInPixels, int id, int timeToStay)
{
	// not a timed text
	if (id == 0)
	{
		screenTextBuffer.push_back(ScreenText(text, textPos, posInPixels, id, timeToStay));

		// see if we have it already
	}
	else
	{
		//timedScreenTexts[id] = new ScreenText(text, textPos, posInPixels, id, timeToStay);
		if (timedScreenTexts.find(id) == timedScreenTexts.end())
		{
			timedScreenTexts[id] = new ScreenText(text, textPos, posInPixels, id, timeToStay);
		}
		else
		{
			delete timedScreenTexts[id];
			timedScreenTexts[id] = new ScreenText(text, textPos, posInPixels, id, timeToStay);
		}
	}

	return textPos;//printGL(text, textPos, posInPixels);
}

void printToScreen2(const ScreenText& screenText)
{
	// scale the coordinates according to the value of posInPixels
	vec3 screenPos;
	if (screenText.posInPixels)
	{
		screenPos = screenText.textPos;    // pixels
	}
	else
	{
		screenPos = screenText.textPos * vec3(8, 13, 0);    // (row, col)
	}

	GLint rasterPosf[4];
	// print the characters on the screen
	void* font = GLUT_BITMAP_8_BY_13;
	vec3 color(1, 1, 1);

	glColor3f(color.x, color.y, color.z);
	glRasterPos2f(screenPos.x, screenPos.y);
	for (size_t i = 0; i < screenText.text.length(); i++)
	{
		switch (screenText.text[i])
		{
			case '\n':
				screenPos.y += 13;
				glColor3f(color.x, color.y, color.z);
				glRasterPos2f(screenPos.x, screenPos.y);
				break;

			case '\t':
				glGetIntegerv(GL_CURRENT_RASTER_POSITION, rasterPosf);
				glRasterPos2f(((rasterPosf[0] / 8 / 4) * 4 + 4) * 8, screenPos.y);
				break;

			case '<':
				if (screenText.text.substr(i + 1, 5) == "color")
				{
					// get color vector
					size_t tagEnd = screenText.text.find('>', i);

					std::string colorVectorStr = screenText.text.substr(i + 6, tagEnd - (i + 6));
					color = conversion::strToVec3(colorVectorStr);

					i = tagEnd;
				}
				else if (screenText.text.substr(i + 1, 6) == "/color")
				{
					color = vec3(1, 1, 1);
					i += 7;
				}

				break;

			default:
				// update the color (have to be used with the rasterpos)
				glColor3f(color.x, color.y, color.z);
				glGetIntegerv(GL_CURRENT_RASTER_POSITION, rasterPosf);
				glRasterPos2f(rasterPosf[0], screenPos.y);

				glutBitmapCharacter(font, screenText.text[i]);
		}
	}

}

/**
 * Flushes the screenTextBuffer and prints its contents to the screen.
 */
void flushScreenTextBuffer()
{
	graphics::Shader* activeShader = graphics::Shader::getActiveShader();
	graphics::Shader::disableAll();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_CUBE_MAP);

	if (glIsEnabled(GL_LIGHTING))
	{
		glDisable(GL_LIGHTING);
	}

	// switch to ortho mode
	glPushMatrix();
	orthoMode(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));


	// print non-timed texts
	for (const ScreenText& text : screenTextBuffer)
	{
		printToScreen2(text);
	}
	screenTextBuffer.clear();

	// print timed texts
	for (const ScreenText& text : screenTextBuffer)
	{
		printToScreen2(text);
	}

	// check elapsed times
	auto end = std::remove_if(screenTextBuffer.begin(), screenTextBuffer.end(), [](const ScreenText& text)
	{
		return text.timeToStay < glutGet(GLUT_ELAPSED_TIME);
	});
	screenTextBuffer.erase(end, screenTextBuffer.end());

	// reset graphics
	perspectiveMode();
	glPopMatrix();

	glPopAttrib();

	graphics::Shader::bindShader(activeShader);
}
}
#endif // CLIENT_SIDE

namespace conversion
{

static const int RING_BUFFER_SIZE = 16;
static const int SMALL_BUFFER_SIZE = 32;
static const int BIG_BUFFER_SIZE = 1024;

static char strBuffer[RING_BUFFER_SIZE][SMALL_BUFFER_SIZE];
static char strFormatBuffer[RING_BUFFER_SIZE][BIG_BUFFER_SIZE];
std::atomic<int> formatBufferIdx(0);

const char* int2str(const int32_t num)
{
	static std::atomic<int> intBufferIdx(0);
	const int idx = intBufferIdx.fetch_add(1);

	snprintf(strBuffer[idx % RING_BUFFER_SIZE], SMALL_BUFFER_SIZE, "%d", num);

	return strBuffer[idx % RING_BUFFER_SIZE];
}

const char* float2str(const float val)
{
	static std::atomic<int> floatBufferIdx(0);
	const int idx = floatBufferIdx.fetch_add(1);

	snprintf(strBuffer[idx % RING_BUFFER_SIZE], SMALL_BUFFER_SIZE, "%4.1f", val);

	return strBuffer[idx % RING_BUFFER_SIZE];
}

const char* FormatStr(const char* fmt, ...)
{
	const int idx = formatBufferIdx.fetch_add(1);

	va_list args;
	va_start(args, fmt);

#ifdef WIN32
	GX_ASSERT(_vscprintf(fmt, args) < BIG_BUFFER_SIZE);
#endif

	vsnprintf(strFormatBuffer[idx % RING_BUFFER_SIZE], BIG_BUFFER_SIZE, fmt, args);
	va_end(args);

	return strFormatBuffer[idx % RING_BUFFER_SIZE];
}

std::string intToStr(int i)
{
	char buffer[33];
	sprintf(buffer, "%d", i);

	return std::string(buffer);
}

std::string floatToStr(float f, int precision)
{
	char buffer[100];
	sprintf(buffer, "%4.1f", f);

	return std::string(buffer);
}

std::string vec3ToStr(const vec3& vec)
{
	const std::string result = floatToStr( vec.x ) + " " + floatToStr( vec.y ) + " " + floatToStr( vec.z );
	return result;
}

float strToFloat(const std::string& str)
{
	return atof(str.c_str());
}

/**
 * Vector format: x y z
 */
vec3 strToVec3(const std::string& str)
{
	const std::vector<std::string> coordinates = utils::tokenize(str, " ");
	return vec3(atof(coordinates[0].c_str()), atof(coordinates[1].c_str()), atof(coordinates[2].c_str()));
}


// float compression/decompression : http://cboard.cprogramming.com/c-programming/92250-storing-float-16-bits.html
int csh = 4;		//8;
int cand = 15;		//255;
float cdiv = 16;	//256.0f;

short float32Tofloat16(float num)
{
	short i, f;

	if (fabs(num) > 2047.999f)
	{
		printf("Error: number out of range (num=%f)\n", num);
	}

	i = (short) num;
	f = (short) (fabs(num * cdiv)) & cand;

	return (i << csh) | f;
}

float float16Tofloat32(int n)
{
	float s = 1.0f;
	if (n < 0)
	{
		s = -1.0f;
		n = -n;
	}

	return s * ((float)(n >> csh) + ((n & cand) / cdiv));
}

vec3 vec3F16_to_vec3F32(int x, int y, int z)
{
	vec3 result;
	result.x = float16Tofloat32(x);
	result.y = float16Tofloat32(y);
	result.z = float16Tofloat32(z);

	return result;
}

//int sign() {
//	return (rand() % 2 == 0) ? 1 : -1;
//}

//float MAX(const float a, const float b) {
//	return (a < b) ? b : a;
//}

//int main(int argc, char **argv) {
//	float f, g, h;
//	short a, b, c;
//	float as, bs, cs;

//	float sumError = 0, sumMaxError = 0;
//	float maxError = 0;
//	int numCrossers = 0;

//	srand (time(nullptr));

//	int size = 1000000;

//	for (int i = 0; i < size; i++) {
//		f = sign() * (rand() % 1000) / (float)(rand() % 10 + 1);
//		g = sign() * (rand() % 1000) / (float)(rand() % 10 + 1);
//		h = sign() * (rand() % 1000) / (float)(rand() % 10 + 1);

//		a = float32Tofloat16(f);
//		b = float32Tofloat16(g);
//		c = float32Tofloat16(h);

//		//printf("%04x, %04x, %04x\n", a, b, c);
//		//printf("fgh: %4.4f, %4.4f, %4.4f\n", f, g, h);
//		//printf("abc: %4.4f, %4.4f, %4.4f\n", float16Tofloat32(a), float16Tofloat32(b), float16Tofloat32(c));

//		as = float16Tofloat32(a);
//		bs = float16Tofloat32(b);
//		cs = float16Tofloat32(c);

//		float errorMargin = 1.5f;

//		if (fabs(as - f) > errorMargin ||
//			fabs(bs - g) > errorMargin ||
//			fabs(cs - h) > errorMargin
//			)
//		{
//			using namespace std;
//			float currentMax = MAX(fabs(as - f), MAX(fabs(bs - g), fabs(cs - h)));
//			maxError = MAX(maxError, currentMax);
//			sumMaxError += currentMax;

//			//printf("err: %4.4f, %4.4f, %4.4f\n", fabs(as - f), fabs(bs - g), fabs(cs - h));
//			//printf("ATTENTION %4.4f\n", currentMax);

//			numCrossers++;
//		}

//		sumError += fabs(as - f) + fabs(bs - g) + fabs(cs - h);

//		//printf("\n");
//	}

//	printf("numCrossers: %d/%d -> %4.1f%\n", numCrossers, size, (float) numCrossers / size * 100);
//	printf("sumError:    %4.2f\n", sumError);
//	printf("sumMaxError: %4.2f\n", sumMaxError);
//	printf("avgMaxError: %4.2f\n", sumMaxError / numCrossers);
//	printf("avgError:    %4.4f\n", sumError / (size * 3));
//	printf("maxError:    %4.4f\n", maxError);

//	return 0;
//}
}

namespace file
{
std::string getDir(const std::string& path)
{
	return path.substr(0, path.find_last_of('/') + 1);
}

std::string getParentDir(const std::string& path)
{
	return getDir(path.substr(0, path.size()).substr(0, path.find_last_of('/') - 1));
}

std::string getFileName(const std::string& path)
{
	std::string tmp = path.substr(path.find_last_of('/') + 1, path.size());
	return tmp.substr(0, tmp.find_first_of('.'));
}

std::string getFileNameWithExtension(const std::string& path)
{
	return path.substr(path.find_last_of('/') + 1, path.size());
}

std::string getExtension(const std::string& path)
{
	return path.substr(path.find_last_of('.') + 1, path.size());
}

int getInt(std::ifstream& file)
{
	std::string str;
	getline(file, str);

	return atoi(str.c_str());
}

float getFloat(std::ifstream& file)
{
	std::string str;
	getline(file, str);

	return atof(str.c_str());
}

std::string getString(std::ifstream& file)
{
	std::string str;
	getline(file, str);

	return str;
}

std::vector<std::string> parseLine(std::ifstream& file)
{
	std::string str;
	getline(file, str);

	return tokenize(str, " \t(){}");
}

std::vector<std::string> findKeyword(std::ifstream& file, const std::string& keyword)
{
	std::string str;
	std::vector<std::string> v;
	while (str.compare(keyword) != 0)
	{
		v = parseLine(file);
		if (!v.empty())
		{
			str = v.at(0);
		}
	}

	return v;
}

void skipLine(std::ifstream& file)
{
	std::string str;
	getline(file, str);
}

bool existFile(const std::string& filename)
{
	return boost::filesystem::exists(filename);
}

char* readFile(const std::string& filename)
{
	int length;
	char* buffer;

	std::ifstream is;
	is.open(filename, std::ios::binary);

	if (!is.is_open())
	{
		return nullptr;
	}

	// get length of file:
	is.seekg (0, std::ios::end);
	length = is.tellg();
	is.seekg (0, std::ios::beg);

	// allocate memory:
	buffer = new char[length + 1];

	// read data as a block:
	is.read(buffer, length);
	buffer[length] = '\0';
	is.close();

	return buffer;
}
}

void toLowerCase(std::string& str)
{
	for (uint i = 0; i < str.length(); i++)
	{
		str[i] = tolower(str[i]);
	}
}

std::vector<std::string> tokenize(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t p0 = 0, p1 = std::string::npos;

	while (p0 != std::string::npos)
	{
		p1 = str.find_first_of(delim, p0);
		if (p1 != p0)
		{
			std::string token = str.substr(p0, p1 - p0);
			tokens.push_back(token);
		}
		p0 = str.find_first_not_of(delim, p1);
	}

	return tokens;
}

} // namespace utils
