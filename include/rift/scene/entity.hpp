#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <cstdint>
#include <unordered_map>

// cannot do simpler
using Entity = uint64_t;

template <typename T>
using EntityMap = std::unordered_map<Entity, T>;

template <typename T>
T *CreateComponent(EntityMap<T> &entityMap, Entity id)
{
	auto ret = entityMap.insert(std::make_pair(id, T()));
	auto &comp = ret.first->second;
	return &comp;
}

 
#endif /* end of include guard: ENTITY_HPP */