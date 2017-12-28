#pragma once
template<typename T>
class Singleton
{
public:
	static T& getInstance();

private:
	Singleton();
	~Singleton();

	static T m_instance;
};

template<typename T>
T Singleton<T>::m_instance;

template<typename T>
Singleton<T>::Singleton() {
}

template<typename T>
Singleton<T>::~Singleton() {
}

template<typename T>
T& Singleton<T>::getInstance() {
	return m_instance;
}


