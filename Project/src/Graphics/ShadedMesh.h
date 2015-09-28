#pragma once


namespace models
{
	class Mesh;
}

namespace graphics
{

class Role;
typedef std::map<const std::string, Role*> RoleDirectory;

class ShadedMesh
{
public:
	ShadedMesh(models::Mesh* mesh = nullptr);
	virtual ~ShadedMesh(void);

	ShadedMesh& operator=(const ShadedMesh& s);

	void addRole(const std::string& name, Role* role);
	void render(const graphics::RenderContext& context, const Entity* pOwner = nullptr);


	// getters-setters
	models::Mesh* getMesh() const;
	void setMesh(models::Mesh* mesh);


	// register to lua
	static void registerMethodsToLua();

protected:
	models::Mesh*	m_pMesh;
	RoleDirectory	m_roles;
};

TextureDirectory& getTextureDirectory();
GLuint getTexture(const std::string& name);
} // namespace graphics
