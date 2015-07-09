#include <utils/coroutine.hpp>
#include <cassert>
#include <time.hpp>

class Coroutine::Anonymous
{
public:
	static void coroutineEntry()
	{
		Coroutine::Impl *pImpl = (Coroutine::Impl*)GetFiberData();
		pImpl->entry();
		pImpl->status = Coroutine::Status::Terminated;
		SwitchToFiber(pImpl->pReturnFiber);
	}

	static void yield(YieldOperation &&yieldOperation)
	{
		Coroutine::Impl *pImpl = (Coroutine::Impl*)GetFiberData();
		pImpl->yieldOperation = &yieldOperation;
		assert(pImpl != nullptr);
		SwitchToFiber(pImpl->pReturnFiber);
	}
};

Coroutine::Impl::~Impl()
{
	if (pFiber)
		DeleteFiber(pFiber);
}

void Coroutine::init()
{
	if (!IsThreadAFiber())
		pImpl->pReturnFiber = ConvertThreadToFiber(NULL);
	else 
		pImpl->pReturnFiber = GetCurrentFiber();
	pImpl->pFiber = CreateFiber(0, (LPFIBER_START_ROUTINE)Coroutine::Anonymous::coroutineEntry, pImpl.get());
}

void Coroutine::resume()
{
	if ((pImpl->status == Suspended) && (!pImpl->yieldOperation || pImpl->yieldOperation->poll()))
		SwitchToFiber(pImpl->pFiber);
}

void yield(YieldOperation &&yieldOperation)
{
	Coroutine::Anonymous::yield(std::move(yieldOperation));
}

WaitForSeconds::WaitForSeconds(double duration_) : startTime(timer::getTime()), duration(duration_)
{}

bool WaitForSeconds::poll() 
{
	return timer::getTime() > (startTime + duration);
}

bool Resume::poll() 
{
	return true;
}