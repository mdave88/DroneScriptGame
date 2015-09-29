#pragma once

#include <map>
#include <memory>
#include <string>

#include <GL/gl.h>

#include "Math/vec3.h"


typedef std::map<const std::string, bool>										BoolDirectory;
typedef std::map<const std::string, float>										FloatDirectory;
typedef std::map<const std::string, int>										IntDirectory;
typedef std::map<const std::string, vec3>										VectorDirectory;
typedef std::map<const std::string, std::string>								StringDirectory;

typedef std::map<unsigned short, const std::string>								StringMap;

// Textures
typedef std::map<const std::string, GLuint>										TextureDirectory;
typedef std::map<const std::string, GLuint>										CubeTextureDirectory;


// Graphics
namespace graphics
{
class Shader;
typedef std::shared_ptr<graphics::Shader> ShaderPtr;
}

typedef std::map<const std::string, graphics::ShaderPtr>						ShaderDirectory;

namespace graphics
{
class ShadedMesh;
typedef std::shared_ptr<graphics::ShadedMesh> ShadedMeshPtr;
}
typedef std::map<const std::string, graphics::ShadedMeshPtr>					ShadedMeshDirectory;

namespace models
{
class Mesh;
}
typedef std::map<const std::string, models::Mesh*>								MeshDirectory;

class SoundSource;
typedef std::map<const std::string, std::shared_ptr<SoundSource>>				SoundDirectory;
