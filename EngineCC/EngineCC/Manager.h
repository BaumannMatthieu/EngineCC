#pragma once
#include <map>
#include <memory>

template<typename K, typename T>
class Manager {
public:
	static Manager<K, T>& getInstance();

	void insert(const K& key, const T& ressource);

	const T& get(const K& key) const;

	const std::map<K, T>& getRessources() const;
private:
	Manager();
	~Manager();

	static Manager<K, T> m_manager;

	std::map<K, T> m_ressources;
};
template<typename K, typename T>
Manager<K, T> Manager<K, T>::m_manager = Manager<K, T>();

template<typename K, typename T>
Manager<K, T>::Manager() {}
template<typename K, typename T>
Manager<K, T>::~Manager() {}


template<typename K, typename T>
Manager<K, T>& Manager<K, T>::getInstance() {
	return m_manager;
}

template<typename K, typename T>
void Manager<K, T>::insert(const K& key, const T& ressource) {
	m_ressources.insert(std::make_pair(key, ressource));
}

template<typename K, typename T>
const T& Manager<K, T>::get(const K& key) const {
	if (m_ressources.find(key) == m_ressources.end()) {
		std::cout << "Element " << key << " not found in the map." << std::endl;
		return nullptr;
	}
	return m_ressources.at(key);
}

template<typename K, typename T>
const std::map<K, T>& Manager<K, T>::getRessources() const {
	return m_ressources;
}
