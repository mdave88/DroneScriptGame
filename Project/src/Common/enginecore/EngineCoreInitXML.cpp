#include "GameStdAfx.h"
#include "EngineCore.h"

#ifdef CLIENT_SIDE
#include "Models/3ds/Model3ds.h"
#include "Models/md2/ModelMd2.h"
#include "Models/md5/ModelMd5.h"
#endif

#include "GameLogic/Door.h"
#include "GameLogic/NodeGroup.h"

#include <RapidXml/rapidxml_print.hpp>
#include <RapidXml/rapidxml_utils.hpp>


using namespace rapidxml;

typedef std::vector<const xml_node<>*> XmlNodeVector;

bool getXmlBoolVariable(const xml_node<>* node, const char* variableName, bool defaultValue = false)
{
	if (node->first_attribute(variableName))
	{
		const char* value = node->first_attribute(variableName)->value();
		return (strcmp(value, "true") == 0);
	}
	else
	{
		return defaultValue;
	}
}

float getXmlFloatVariable(const xml_node<>* node, const char* variableName, float defaultValue = 0.0f)
{
	if (node->first_attribute(variableName))
	{
		float value = 0;
		const char* tmp = node->first_attribute(variableName)->value();
		sscanf(tmp, "%3f", &value);

		return value;
	}
	else
	{
		return defaultValue;
	}
}

vec3 getXmlVec(const xml_node<>* node, const char* vectorNameC, const vec3& defaultValue = vec3(0, 0, 0))
{
	bool returnDef = true;

	vec3 v;
	const std::string vectorName(vectorNameC);
	std::string vectorTmp;

	vectorTmp = vectorName + ".x";
	if (node->first_attribute(vectorTmp.c_str()))
	{
		std::string tmp = node->first_attribute(vectorTmp.c_str())->value();
		sscanf(tmp.c_str(), "%f", &v.x);
		returnDef = false;
	}

	vectorTmp = vectorName + ".y";
	if (node->first_attribute(vectorTmp.c_str()))
	{
		std::string tmp = node->first_attribute(vectorTmp.c_str())->value();
		sscanf(tmp.c_str(), "%f", &v.y);
		returnDef = false;
	}

	vectorTmp = vectorName + ".z";
	if (node->first_attribute(vectorTmp.c_str()))
	{
		std::string tmp = node->first_attribute(vectorTmp.c_str())->value();
		sscanf(tmp.c_str(), "%f", &v.z);
		returnDef = false;
	}

	if (returnDef)
	{
		return defaultValue;
	}

	return v;
}

XmlNodeVector parseXmlSubnodes(const xml_node<>* xmlNode)
{
	XmlNodeVector subXmlNodes;

	xmlNode = xmlNode->first_node();
	while (xmlNode)
	{
		subXmlNodes.push_back(xmlNode);
		xmlNode = xmlNode->next_sibling();
	}

	return subXmlNodes;
}

#ifdef CLIENT_SIDE
void EngineCore::loadMeshes(const rapidxml::xml_node<>* rootXmlNode, const bool justData)
{
	//xml_node<>* currentXmlNode;

	//// load md5 meshes
	//currentXmlNode = rootXmlNode->first_node("MeshMd5");
	//while (currentXmlNode)
	//{
	//	const char* name = currentXmlNode->first_attribute("name")->value();
	//	const char* modelName = currentXmlNode->first_attribute("model")->value();

	//	models::ModelMd5* model = new models::ModelMd5();
	//	const bool justSkeleton = (getApplicationSide() == APPSIDE_SERVER);
	//	model->loadMesh(modelName, graphics::getTextureDirectory(), justSkeleton);

	//	m_meshDirectory[name] = model;

	//	//ModelMd5::load(modelName.c_str(), nullptr, meshDirectory, textureDirectory, name.c_str());

	//	XmlNodeVector animXmlNodes = parseXmlSubnodes(currentXmlNode);
	//	ForEach (XmlNodeVector, animXmlNodes, iAnim)
	//	{
	//		const char* animName = (*iAnim)->first_attribute("file")->value();
	//		((models::ModelMd5*) m_meshDirectory[name])->loadAnim(animName);

	//		if (getXmlBoolVariable(*iAnim, "default", false))
	//		{
	//			((models::ModelMd5*) m_meshDirectory[name])->setCurrentAnimationName(utils::file::getFileName(animName));
	//		}
	//	}
	//	currentXmlNode = currentXmlNode->next_sibling("MeshMd5");
	//}

	//// load 3ds meshes
	//currentXmlNode = rootXmlNode->first_node("Mesh3ds");
	//while (currentXmlNode)
	//{
	//	const char* name = currentXmlNode->first_attribute("name")->value();
	//	const char* model = currentXmlNode->first_attribute("model")->value();

	//	models::Model3ds::load(model, m_meshDirectory, graphics::getTextureDirectory(), name, justData);

	//	currentXmlNode = currentXmlNode->next_sibling("Mesh3ds");
	//}


	//// load md2 meshes
	//currentXmlNode = rootXmlNode->first_node("MeshMd2");
	//while (currentXmlNode)
	//{
	//	const char* name = currentXmlNode->first_attribute("name")->value();
	//	const char* model = currentXmlNode->first_attribute("model")->value();

	//	models::ModelMd2::load(model, m_meshDirectory, graphics::getTextureDirectory(), name, justData);

	//	currentXmlNode = currentXmlNode->next_sibling("MeshMd2");
	//}
}

void EngineCore::loadShadedMeshes(const rapidxml::xml_node<>* rootXmlNode)
{
	//xml_node<>* currentXmlNode;

	//currentXmlNode = rootXmlNode->first_node("ShadedMesh");
	//while (currentXmlNode)
	//{
	//	const char* name = currentXmlNode->first_attribute("name")->value();
	//	const char* meshName = currentXmlNode->first_attribute("mesh")->value();

	//	if (m_meshDirectory.find(meshName) == m_meshDirectory.end())
	//	{
	//		TRACE_ERROR("Error: Mesh definition <" << meshName << "> missing in " << name << " ShadedMesh.", 0);
	//		exit(EXIT_FAILURE);
	//	}
	//	m_shadedMeshDirectory[name] = graphics::ShadedMeshPtr(new graphics::ShadedMesh(m_meshDirectory[meshName]));

	//	// load the shading data (skip this on the server side)
	//	if (getApplicationSide() == APPSIDE_CLIENT || getApplicationSide() == APPSIDE_BOTH)
	//	{
	//		// roles
	//		const XmlNodeVector roleXmlNodes = parseXmlSubnodes(currentXmlNode);
	//		ForEachC (XmlNodeVector, roleXmlNodes, iRole)
	//		{
	//			const char* roleName = (*iRole)->first_attribute("name")->value();

	//			graphics::Role* role = new graphics::Role();

	//			// materials
	//			const XmlNodeVector materialXmlNodes = parseXmlSubnodes(*iRole);
	//			ForEachC (XmlNodeVector, materialXmlNodes, iMaterial)
	//			{
	//				const std::string techniqueName = (*iMaterial)->first_attribute("technique")->value();
	//				const std::string techniqueGlslVersionStr = (*iMaterial)->first_attribute("glslVersion") ? (*iMaterial)->first_attribute("glslVersion")->value() : "0";
	//				const int techniqueGlslVersion = atoi(techniqueGlslVersionStr.c_str());

	//				if (graphics::getEffectDirectory()[techniqueName] == nullptr)
	//				{
	//					const std::string shaderPath = getResourcesDir() + "shaders/" + techniqueName;
	//					graphics::getEffectDirectory()[techniqueName] = graphics::ShaderPtr(new graphics::Shader(shaderPath.c_str(), techniqueGlslVersion));
	//				}

	//				graphics::Material* material = new graphics::Material(graphics::getEffectDirectory()[techniqueName].get());

	//				// material parameters
	//				const XmlNodeVector materialParameterXmlNodes = parseXmlSubnodes(*iMaterial);
	//				ForEachC (XmlNodeVector, materialParameterXmlNodes, iMaterialParameter)
	//				{
	//					if (strcmp((*iMaterialParameter)->name(), "Texture") == 0)
	//					{
	//						const char* name = (*iMaterialParameter)->first_attribute("name")->value();
	//						const std::string fileName = std::string(getDataDir() + "textures/") + std::string((*iMaterialParameter)->first_attribute("file")->value());

	//						GLuint tex;
	//						loadTexture(fileName, tex, graphics::getTextureDirectory());
	//						material->setTexture(name, tex);

	//						// material vectors
	//					}
	//					if (strcmp((*iMaterialParameter)->name(), "Vector") == 0)
	//					{
	//						const char* name = (*iMaterialParameter)->first_attribute("name")->value();
	//						const vec3 value = getXmlVec(*iMaterialParameter, "value");

	//						material->setVector(name, value);

	//						// material variables
	//					}
	//					else if (strcmp((*iMaterialParameter)->name(), "IntVariable") == 0)
	//					{
	//						const char* name = (*iMaterialParameter)->first_attribute("name")->value();
	//						const int value = (int) getXmlFloatVariable((*iMaterialParameter), "value");

	//						material->setIntVariable(name, value);
	//					}
	//					else if (strcmp((*iMaterialParameter)->name(), "FloatVariable") == 0)
	//					{
	//						const char* name = (*iMaterialParameter)->first_attribute("name")->value();
	//						const float value = getXmlFloatVariable((*iMaterialParameter), "value");

	//						material->setFloatVariable(name, value);
	//					}
	//				}
	//				role->addMaterial(material);
	//			}
	//			m_shadedMeshDirectory[name]->addRole(roleName, role);
	//		}

	//	}

	//	currentXmlNode = currentXmlNode->next_sibling("ShadedMesh");
	//}
}
#endif

void EngineCore::parseLuaSettings(const xml_node<>* rootXmlNode, bool justDefinitions)
{
	const std::string rootDirectoryName = rootXmlNode->first_attribute("rootDirectory")->value();

	// definition scripts
	const xml_node<>* currentXmlNode = rootXmlNode->first_node("DefinitonsScript");
	while (currentXmlNode)
	{
		const char* filename = currentXmlNode->first_attribute("file")->value();
		m_luaDefinitonScripts.push_back(rootDirectoryName + filename);

		currentXmlNode = currentXmlNode->next_sibling("DefinitonsScript");
	}

	// initialized scripts
	currentXmlNode = rootXmlNode->first_node("InitScript");
	while (currentXmlNode)
	{
		const char* filename = currentXmlNode->first_attribute("file")->value();
		m_luaInitializerScripts.push_back(rootDirectoryName + filename);

		currentXmlNode = currentXmlNode->next_sibling("InitScript");
	}
}

void EngineCore::parseXml(const std::string& filename, const bool justData)
{
	char* buffer = utils::file::readFile(filename.c_str());
	const std::string xmlData(buffer);
	delete[] buffer;

	std::vector<char> xmlCopy(xmlData.begin(), xmlData.end());
	xmlCopy.push_back('\0');

	xml_document<> doc;
	doc.parse < parse_declaration_node | parse_no_data_nodes > (&xmlCopy[0]);
	xml_node<>* rootXmlNode = doc.first_node("Scene");


	// setup lua
#ifndef SERVER_SIDE
	const bool justDefinitions = true;
#else
	const bool justDefinitions = false;
#endif
	parseLuaSettings(rootXmlNode->first_node("LuaSettings"), justDefinitions);

	resetLuaScripts();
	LuaManager::getInstance()->registerObject("rootNode", m_pRootNode);

#ifdef CLIENT_SIDE
	loadMeshes(rootXmlNode->first_node("Models"), justData);
	loadShadedMeshes(rootXmlNode->first_node("ShadedMeshes"));
#endif

	//loadLevel(rootXmlNode->first_node("Level"), justData);
}
