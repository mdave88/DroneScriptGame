#include "GameStdAfx.h"
#include "Graphics/ShadedMesh.h"
#include "Graphics/Role.h"
#include "Graphics/RenderContext.h"
#include "Models/mesh/Mesh.h"
#include "GameLogic/EngineCore.h"

#include <algorithm>


namespace graphics
{

ShadedMesh::ShadedMesh(models::Mesh* pMesh)
{
	m_pMesh = pMesh;
}

ShadedMesh::~ShadedMesh()
{
	for (auto role : m_roles)
	{
		SAFEDEL(role.second);
	}
}

ShadedMesh& ShadedMesh::operator=(const ShadedMesh& other)
{
	if (this != &other)
	{
		for (const auto role : other.m_roles)
		{
			m_roles[role.first] = new Role();
			*m_roles[role.first] = *role.second;
		}

		m_pMesh = other.m_pMesh;
	}
	return *this;
}

void ShadedMesh::render(const graphics::RenderContext& context/*, const Entity* pOwner*/)
{
	if (m_roles.empty())
	{
		// no roles defined -> simply render the mesh
		m_pMesh->render();
		return;
	}

	if (context.m_roleName == "none")
	{
		Role::updateCommonUniforms(context/*, pOwner*/);

		const GLuint program = EngineCore::getInstance()->getShader("simplest")->getProgram();

		const uint numSubmeshes = std::max(1u, m_pMesh->getNumObjects());
		for (uint i = 0; i < numSubmeshes; i++)
		{
			m_pMesh->preRender(program);
			m_pMesh->renderSubset(i);
			m_pMesh->postRender();
		}

		return;
	}

	auto iRole = m_roles.find(context.m_roleName);
	if (iRole != m_roles.end())
	{
		iRole->second->render(context, m_pMesh/*, pOwner*/);
	}
}

void ShadedMesh::addRole(const std::string& name, Role* pRole)
{
	m_roles[name] = pRole;
}

// getters-setters
models::Mesh* ShadedMesh::getMesh() const
{
	return m_pMesh;
}

void ShadedMesh::setMesh(models::Mesh* pMesh)
{
	m_pMesh = pMesh;
}

} // namespace graphics
