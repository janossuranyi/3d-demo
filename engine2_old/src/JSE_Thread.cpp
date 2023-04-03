#include "JSE.h"

namespace js {
	Thread::Thread(ThreadFunction func, const char* name, void* data)
	{
		joinable_ = true;
		pThread_ = SDL_CreateThread(func, name, data);
	}

	Thread::Thread()
	{
		pThread_ = nullptr;
		joinable_ = false;
	}

	Thread::Thread(Thread&& other) noexcept
	{
		pThread_ = other.pThread_;
		joinable_ = other.joinable_;
		other.pThread_ = nullptr;
		other.joinable_ = false;
	}

	Thread& Thread::operator=(Thread&& other) noexcept
	{
		pThread_ = other.pThread_;
		joinable_ = other.joinable_;
		other.pThread_ = nullptr;
		other.joinable_ = false;

		return *this;
	}

	Thread::~Thread()
	{
		if (joinable_) {
			SDL_WaitThread(pThread_, nullptr);
		}
		pThread_ = nullptr;
	}

	void Thread::swap(Thread& other)
	{
		std::swap(pThread_, other.pThread_);
		std::swap(joinable_, other.joinable_);
	}

	void Thread::detach() noexcept
	{
		SDL_DetachThread(pThread_);
		pThread_ = nullptr;
		joinable_ = false;
	}

	bool Thread::joinable() const
	{
		return joinable_;
	}

	int Thread::join()
	{
		int result;
		SDL_WaitThread(pThread_, &result);
		pThread_ = nullptr;
		joinable_ = false;

		return result;
	}

	uint32_t Thread::native_id() const
	{
		return SDL_static_cast(uint32_t, SDL_GetThreadID(pThread_));
	}

	const char* Thread::name() const
	{
		return SDL_GetThreadName(pThread_);
	}

	uint32_t Thread::current_id()
	{
		return SDL_static_cast(uint32_t, SDL_GetThreadID(nullptr));
	}
}