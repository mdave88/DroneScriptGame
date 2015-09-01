
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <map>
#include <memory>
#include <string>

#include <GL/gl.h>

#include "Math/vector.h"


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
}
typedef std::shared_ptr<graphics::Shader>	ShaderPtr;
typedef std::map<const std::string, ShaderPtr>									EffectDirectory;

namespace graphics
{
class ShadedMesh;
typedef std::shared_ptr<graphics::ShadedMesh>	ShadedMeshPtr;
}
typedef std::map<const std::string, graphics::ShadedMeshPtr>					ShadedMeshDirectory;



// Nodes
class Node;
typedef std::shared_ptr<Node>	NodePtr;
typedef std::map<const std::string, NodePtr>									NodeDirectory;

typedef std::map<uint16_t, NodePtr>												NodeIdDirectory;

// Entities
class Entity;
typedef std::map<const std::string, std::shared_ptr<Entity>>					EntityDirectory;

// Items
class Item;
typedef std::map<const std::string, std::shared_ptr<Item>>					ItemDirectory;

// Units
class Unit;
typedef std::map<const std::string, std::shared_ptr<Unit>>					UnitDirectory;

// Players
class Player;
typedef std::map<const std::string, std::shared_ptr<Player>>					PlayerDirectory;

// Weapons
class Weapon;
typedef std::shared_ptr<Weapon> WeaponPtr;

typedef std::map<const std::string, WeaponPtr>									WeaponDirectory;


// Physics
class NxMaterial;
typedef std::map<const std::string, NxMaterial*>								PhysicsMaterialDirectory;

namespace physics
{
class PhysicsModel;
}
typedef std::map<const std::string, physics::PhysicsModel*>						PhysicsModelDirectory;


// Misc
namespace models
{
class Mesh;
}
typedef std::map<const std::string, models::Mesh*>								MeshDirectory;

class LightSource;
typedef std::map<const std::string, std::shared_ptr<LightSource>>				LightSourceDirectory;

class SoundSource;
typedef std::map<const std::string, std::shared_ptr<SoundSource>>				SoundDirectory;


template <typename Type>
Type* getElement(std::map<const std::string, Type>& map, const std::string& name)
{
	if (map.find(name) != map.end())
	{
		return map[name];
	}

	return nullptr;
}

#endif
