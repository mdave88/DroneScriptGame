#pragma once

template <class T>
class Singleton
{
public:
	static T* getInstance();
	static bool hasInstance();
	static void destroyInstance();

protected:
	Singleton();
	virtual ~Singleton();

private:
	static T* s_instance;
};


template <class T>
T* Singleton<T>::s_instance = 0;

template <class T>
T* Singleton<T>::getInstance()
{
	GX_ASSERT(s_instance);	// doesn't create the instance automatically
	//if (!s_instance)
	//	s_instance = (T*)this;

	return s_instance;
}

template <class T>
bool Singleton<T>::hasInstance()
{
	return s_instance != 0;
}

template <class T>
void Singleton<T>::destroyInstance()
{
	delete s_instance;
	s_instance = 0;
}

template <class T>
Singleton<T>::Singleton()
{
	GX_ASSERT(s_instance == 0);
	s_instance = (T*)this;
}

template <class T>
Singleton<T>::~Singleton()
{
	s_instance = 0;
}
