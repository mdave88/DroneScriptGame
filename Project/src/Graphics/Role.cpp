#include "GameStdAfx.h"
#include "Graphics/Role.h"
#include "Graphics/Camera.h"
#include "Graphics/RenderContext.h"
#include "Models/mesh/Mesh.h"

#include <algorithm>


namespace graphics
{

Role::Role()
{
}

Role& Role::operator=(const Role& s)
{
	if (this != &s)
	{
		for (const Material* material : s.m_materials)
		{
			Material* pMaterial = new Material();
			*pMaterial = *material;
			m_materials.push_back(pMaterial);
		}
	}
	return *this;
}

Role::~Role()
{
	for (Material* material : m_materials)
	{
		SAFEDEL(material);
	}
}

void Role::updateCommonUniforms(const graphics::RenderContext& context, const Matrix& modelMatrix, const graphics::Shader* customShader)
{
	const graphics::Shader* shader = (customShader != nullptr) ? customShader : context.m_pShader;
	shader->updateCommonUniforms(context.m_pCamera, modelMatrix);

	// update lights
	const GLuint program = shader->getProgram();
	///LightSource::updateLightUniforms(shader);

	// Shadow
	shader->setUniform1i("u_enableShadows", context.getEnableBit("shadow"));
}

void Role::render(const graphics::RenderContext& context, models::Mesh* pMesh)
{
	const Shader* pEffect = context.m_pShader;

	const uint numSubmeshes = std::max(1u, pMesh->getNumObjects());
	for (uint i = 0; i < numSubmeshes; i++)
	{
		if (i < m_materials.size())
		{
			pEffect = m_materials[i]->apply(context.m_pShader);
		}


		/// TODO? pOwner modelMatrix
		Matrix modelMatrix;
		Role::updateCommonUniforms(context, modelMatrix, pEffect);

		const GLuint program = pEffect->getProgram();
		pMesh->preRender(program);
		pMesh->renderSubset(i);
		pMesh->postRender();


		//// reset common textures
		//if (pEffect) {
		//	pEffect->setTexture("tex", 0, 0);
		//	pEffect->setTexture("normalHeightMap", 1, 0);

		//	m_materials[i]->resetTextures(pEffect);
		//}
	}
}

void Role::addMaterial(Material* pMaterial)
{
	m_materials.push_back(pMaterial);
}

} // namespace graphics
