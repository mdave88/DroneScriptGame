#ifndef ROLE_H
#define ROLE_H

#include <vector>

#include "Graphics/Material.h"


class Entity;

namespace graphics
{

class RenderContext;

class Role
{
public:
	Role();
	~Role();

	Role& operator=(const Role& s);

	void addMaterial(Material* pMaterial);

	void render(const graphics::RenderContext& context, models::Mesh* pMesh, const Entity* entity = nullptr);

	static void updateCommonUniforms(const graphics::RenderContext& context, const Entity* entity, const graphics::Shader* customEffect = nullptr);

	// register to lua
	static void registerMethodsToLua();


private:
	std::vector<Material*> m_materials;
};

} // namespace graphics

#endif // ROLE_H