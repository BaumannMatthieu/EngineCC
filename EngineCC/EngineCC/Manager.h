#pragma once
#include <map>
#include <memory>

#include "EntityHierarchy.h"

template<typename K, typename T>
class Manager {
public:
	static Manager<K, T>& getInstance();

	void insert(const K& key, const T& ressource);
	void insertCopy(K key, T ressource);

	void remove(const K& key);

	bool isThereRessource(const K& key);

	const T& get(const K& key) const;
	T& get(const K& key);

	const std::map<K, T>& getRessources() const;
	std::map<K, T>& getRessources();
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
void Manager<K, T>::remove(const K& key) {
	if (!isThereRessource(key))
		return;
	m_ressources.erase(key);
}

template<typename K, typename T>
Manager<K, T>& Manager<K, T>::getInstance() {
	return m_manager;
}

template<typename K, typename T>
void Manager<K, T>::insert(const K& key, const T& ressource) {
	m_ressources.insert(std::make_pair(key, ressource));
}

template<typename K, typename T>
void Manager<K, T>::insertCopy(K key, T ressource) {
	m_ressources.insert(std::make_pair(key, std::move(ressource)));
}

template<typename K, typename T>
const T& Manager<K, T>::get(const K& key) const {
	if (m_ressources.find(key) == m_ressources.end()) {
		std::cout << "Element " << key << " not found in the map." << std::endl;
	}
	return m_ressources.at(key);
}

template<typename K, typename T>
T& Manager<K, T>::get(const K& key) {
	if (m_ressources.find(key) == m_ressources.end()) {
		std::cout << "Element " << key << " not found in the map." << std::endl;
	}
	return m_ressources.at(key);
}
template<typename K, typename T>
bool Manager<K, T>::isThereRessource(const K& key) {
	return (m_ressources.find(key) != m_ressources.end());
}

template<typename K, typename T>
const std::map<K, T>& Manager<K, T>::getRessources() const {
	return m_ressources;
}

template<typename K, typename T>
std::map<K, T>& Manager<K, T>::getRessources() {
	return m_ressources;
}

// A static manager of all the finite state machine in the game
using ScriptManager = Manager<std::string, FiniteStateMachinePtr>;
// A static manager of all the entity hierarchies in the game
using EntityHierarchyManager = Manager<entityx::Entity, EntityHierarchyPtr>;
