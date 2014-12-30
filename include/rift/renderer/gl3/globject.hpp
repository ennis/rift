#ifndef GLOBJECT_HPP
#define GLOBJECT_HPP

#include <utility>
#include <renderer_common.hpp>
#include <gl_common.hpp>
#include <gl_core_4_4.hpp>

// define move ctors
#define GL_MOVEABLE_OBJECT_IMPL(ty) \
	friend class Renderer;\
	ty() = default; \
	ty(const ty &) = delete; \
	ty &operator=(const ty &) = delete; \
	ty(ty &&rhs) {\
		*this = std::move(rhs);\
	}\
	ty &operator=(ty &&rhs) {\
		swap(std::move(rhs));\
		return *this;\
	}

// null obj check
#define GL_IS_NULL_IMPL(expr) \
	bool isNull() const {\
		return (expr) == 0;\
	}

class GLAPIObject
{

};

 
#endif /* end of include guard: GLOBJECT_HPP */