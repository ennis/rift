#ifndef ENTITY_HPP
#define ENTITY_HPP

// C++11 entity system
// inspired by https://github.com/alecthomas/entityx

#include <array>	// std::array
#include <vector>	// std::vector
#include <algorithm>	// std::remove_if
#include <memory>	// std::unique_ptr

namespace util
{
namespace ecs
{

struct component_base
{
protected:
	static int family_counter;
};

template <typename T>
struct component : public component_base
{
	static int family()
	{
		static int family_index = family_counter++;
		return family_index;
	}
};

class entity
{
public:

	// Component map
	std::array<component_base*, kMaxComponents> components;
};

struct component_deleter_base
{
	virtual void delete_component(component_base *component) = 0;
};

template <typename T>
struct component_deleter : public component_deleter_base
{
	void delete_component(T *component) override
	{
		// TODO pools
		delete component;
	}
};

struct world
{
	entity *create_entity()
	{
		entities.push_back(std::make_unique<entity>());
		return entities.back().get();
	}

	void delete_entity(entity *ent)
	{
		// XXX ...
		entities.erase(
			std::remove_if(
				entities.begin(), 
				entities.end(), 
				[ent](const std::unique_ptr<entity> &e) {
					return e.get() == ent;
				}), 
			entites.end());
	}

	template <typename T, typename... Args>
	T &add_component(entity *ent, Args&&... args)
	{
		remove_component<T>(ent);
		// bwaaaaah
		auto ptr = new T(std::forward<Args>(args)...);
		ent->components[T::family()] = ptr;
		return ptr;
	}

	template <typename T>
	T &get_component(entity *ent)
	{
		return *ent->components[T::family()];
	}

	template <typename T>
	void remove_component(entity *ent)
	{
		assert(component_deleters[T::family()] != nullptr);
		if (ent->components[T::family()] != nullptr)
			component_deleters[T::family()]->delete_component(ent->components[T::family()]);
	}

	std::vector<std::unique_ptr<entity> > entities;
	std::array<std::unique_ptr<component_deleter_base>, kMaxComponents> component_deleters; 
};


} }

 
#endif /* end of include guard: ENTITY_HPP */