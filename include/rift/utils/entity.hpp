#ifndef ENTITY_HPP
#define ENTITY_HPP

// C++11 entity system
// inspired by https://github.com/alecthomas/entityx

#include <array>	// std::array
#include <vector>	// std::vector
#include <algorithm>	// std::remove_if
#include <memory>	// std::unique_ptr

namespace util { namespace ecs {

constexpr auto kMaxComponents = 16u;

struct component_base
{
protected:
	static unsigned family_counter;
};

template <typename T>
struct component : public component_base
{
	static unsigned family()
	{
		static unsigned family_index = family_counter++;
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
	void delete_component(component_base *component) override
	{
		// TODO pools
		delete static_cast<T*>(component);
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
		for (auto i = 0u; i < kMaxComponents; ++i) {
			remove_component_by_family(ent, i);
		}
		// XXX ...
		entities.erase(
			std::remove_if(
				entities.begin(), 
				entities.end(), 
				[ent](const std::unique_ptr<entity> &e) {
					return e.get() == ent;
				}), 
			entities.end());
	}

	template <typename T, typename... Args>
	T &add_component(entity *ent, Args&&... args)
	{
		if (component_deleters[T::family()] == nullptr) {
			component_deleters[T::family()] = std::make_unique<component_deleter<T> >();
		}
		remove_component<T>(ent);
		// bwaaaaah
		auto ptr = new T(std::forward<Args>(args)...);
		ent->components[T::family()] = ptr;
		return *ptr;
	}

	template <typename T>
	T &get_component(entity *ent)
	{
		return static_cast<T&>(*ent->components[T::family()]);
	}

	template <typename T>
	void remove_component(entity *ent)
	{
		remove_component_by_family(ent, T::family());
	}

	void remove_component_by_family(entity *ent, int family)
	{
		if (ent->components[family] != nullptr)
		{
			assert(component_deleters[family] != nullptr);
			component_deleters[family]->delete_component(ent->components[family]);
			ent->components[family] = nullptr;
		}
	}

	size_t size()
	{
		return entities.size();
	}

	std::vector<std::unique_ptr<entity> > entities;
	std::array<std::unique_ptr<component_deleter_base>, kMaxComponents> component_deleters; 


	struct entity_iterator
	{
		world &w;
		unsigned family;
		unsigned current;

		entity_iterator(world &w_, unsigned family_, unsigned current_ = 0) :
			w(w_), family(family_), current(current_)
		{
			skip();
		}

		entity_iterator &operator++()
		{
			++current;
			skip();
		}

		entity *operator*()
		{
			return w.entities[current].get();
		}

	private:
		void skip()
		{
			while (current < w.size() && w.entities[current]->components[family] == nullptr)
				++current;
		}
	};
	
	struct entity_view
	{
		world &w;
		unsigned family;

		entity_view(world &world_, unsigned family_) : w(world_), family(family_)
		{}

		entity_iterator begin() const {
			return entity_iterator(w, family, 0);
		}

		entity_iterator end() const {
			return entity_iterator(w, family, w.size());
		}
	};

	template <typename T> 
	entity_view entities_with_component() const
	{
		return entity_view(*this, T::family());
	}

};

static inline bool operator==(const world::entity_iterator &it1, const world::entity_iterator &it2)
{
	return (it1.family == it2.family) && (it1.current == it2.current);
}


} }

 
#endif /* end of include guard: ENTITY_HPP */