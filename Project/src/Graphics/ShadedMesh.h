#pragma once


namespace models
{
	class Mesh;
}

namespace graphics
{

class Role;
class RenderContext;


class ShadedMesh
{
public:
	ShadedMesh(models::Mesh* mesh = nullptr);
	virtual ~ShadedMesh(void);

	ShadedMesh& operator=(const ShadedMesh& s);

	void addRole(const std::string& name, Role* role);
	void render(const graphics::RenderContext& context/*, const Entity* pOwner = nullptr*/);	// TODO: remove entity


	// getters-setters
	models::Mesh* getMesh() const;
	void setMesh(models::Mesh* mesh);
	
protected:
	models::Mesh* m_pMesh;
	std::map<const std::string, Role*> m_roles;
};

} // namespace graphics
