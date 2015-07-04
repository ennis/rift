#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include <functional>
#include <memory>
#include <Windows.h>

class Coroutine;
class YieldOperation;

class Coroutine
{
public:
	enum Status
	{
		Suspended,
		Terminated
	};

	class Anonymous;

	Coroutine()
	{
		pImpl = std::make_unique<Coroutine::Impl>();
	}

	template <typename Fn, typename... Args>
	Coroutine(Fn &&fn, Args&&... args) 
	{
		// magic
		pImpl = std::make_unique<Coroutine::Impl>();
		pImpl->entry = std::bind(std::forward<Fn>(fn), std::forward<Args...>(args...)); 
		pImpl->status = Status::Suspended;
		init();
	}

	template <typename Fn, typename... Args>
	static Coroutine start(Fn &&fn, Args&&... args)
	{
		Coroutine c(std::forward<Fn>(fn), std::forward<Args...>(args...));
		c.resume();
		return std::move(c);
	}

	void resume();

	Status getStatus() const
	{
		return pImpl->status;
	}

private:
	struct Impl
	{
		std::function<void()> entry;
		YieldOperation *yieldOperation = nullptr;
		Status status = Status::Terminated;
		// windows-specific
		LPVOID pReturnFiber = nullptr;
		LPVOID pFiber = nullptr;
	};

	void init();

	struct Impl;
	std::unique_ptr<Impl> pImpl;

};

class YieldOperation
{
public:
	YieldOperation() = default;

	// Checks if the operation has completed
	// If yes, returns true and the coroutine can continue
	// If not, returns false
	virtual bool poll() = 0;

protected:
};

class WaitForSeconds : public YieldOperation
{
public:
	WaitForSeconds(double duration_);
	// checks that currentTime > startTime + time
	bool poll() override;

protected:
	double startTime;
	double duration;
};

class Resume : public YieldOperation
{
public:
	Resume()
	{}
	// always returns true
	bool poll() override;

protected:
};

void yield(YieldOperation &&yieldOperation = Resume());

#endif