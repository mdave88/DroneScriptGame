
#ifndef CONSTANT_MANAGER_H
#define CONSTANT_MANAGER_H

#include "Common/Directory.h"
#include "Common/Singleton.h"

#include <rapidjson/document.h>


class ConstantManager : public Singleton<ConstantManager>
{
public:
	ConstantManager();
	~ConstantManager();

	bool loadConstants(const std::string& filename = "contants.xml");

	inline void setIntConstant(const std::string& name, int value)
	{
		m_intDirectory[name] = value;
	}

	inline void setFloatConstant(const std::string& name, float value)
	{
		m_floatDirectory[name] = value;
	}

	inline void setBoolConstant(const std::string& name, bool value)
	{
		m_boolDirectory[name] = value;
	}

	inline void setStringConstant(const std::string& name, const std::string& value)
	{
		m_stringDirectory[name] = value;
	}

	inline void setVectorConstant(const std::string& name, const vec3& value)
	{
		m_vectorDirectory[name] = value;
	}

	inline int getIntConstant(const std::string& name)
	{
		return m_intDirectory[name];
	}

	inline bool getBoolConstant(const std::string& name)
	{
		return m_boolDirectory[name];
	}

	inline float getFloatConstant(const std::string& name)
	{
		return m_floatDirectory[name];
	}

	inline std::string& getStringConstant(const std::string& name)
	{
		return m_stringDirectory[name];
	}

	inline vec3 getVectorConstant(const std::string& name)
	{
		return m_vectorDirectory[name];
	}


	// register methods to lua
	static void registerMethodsToLua();

private:
	void parseObject(const rapidjson::Value& object, const std::string& path = "");

private:
	BoolDirectory			m_boolDirectory;
	IntDirectory			m_intDirectory;
	FloatDirectory			m_floatDirectory;
	StringDirectory			m_stringDirectory;
	VectorDirectory			m_vectorDirectory;
};

// short helper methods
inline int CONST_INT(const std::string& name)
{
	return ConstantManager::getInstance()->getIntConstant(name);
}

inline bool CONST_BOOL(const std::string& name)
{
	return ConstantManager::getInstance()->getBoolConstant(name);
}

inline float CONST_FLOAT(const std::string& name)
{
	return ConstantManager::getInstance()->getFloatConstant(name);
}

inline vec3 CONST_VEC3(const std::string& name)
{
	return ConstantManager::getInstance()->getVectorConstant(name);
}

inline const std::string& CONST_STR(const std::string& name)
{
	return ConstantManager::getInstance()->getStringConstant(name);
}

#endif // CONSTANT_MANAGER_H
