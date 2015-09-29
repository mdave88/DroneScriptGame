#pragma once
#include <vector>

#include "Graphics/Material.h"


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

	void render(const graphics::RenderContext& context, models::Mesh* pMesh);

	static void updateCommonUniforms(const graphics::RenderContext& context, const Matrix& modelMatrix = Matrix(), const graphics::Shader* customEffect = nullptr);

private:
	std::vector<Material*> m_materials;
};

} // namespace graphics
