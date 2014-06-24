#ifndef UNIMPLEMENTED_HPP
#define UNIMPLEMENTED_HPP

#include <stdexcept>

class unimplemented_exception : public std::exception
{
public:
	unimplemented_exception() : std::exception("function not implemented")
	{}
};

#endif