#ifndef RESOURCE_HPP
#define RESOURCE_HPP

class CResourceBase
{
public:
	virtual void addRef();
	virtual void release();

protected:
	int mRefCount;
};

#endif