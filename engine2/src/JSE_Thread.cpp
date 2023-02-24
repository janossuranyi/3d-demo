#include "JSE.h"

JseThread::JseThread(JseThreadFunction func, const char* name, void* data)
{
	joinable_ = true;
	pThread_ = SDL_CreateThread(func, name, data);
}

JseThread::JseThread()
{
	pThread_ = nullptr;
	joinable_ = false;
}

JseThread::JseThread(JseThread&& other) noexcept
{
	pThread_ = other.pThread_;
	joinable_ = other.joinable_;
	other.pThread_ = nullptr;
	other.joinable_ = false;
}

JseThread& JseThread::operator=(JseThread&& other) noexcept
{
	pThread_ = other.pThread_;
	joinable_ = other.joinable_;
	other.pThread_ = nullptr;
	other.joinable_ = false;

	return *this;
}

JseThread::~JseThread()
{
	if (joinable_) {
		SDL_WaitThread(pThread_, nullptr);
	}
	pThread_ = nullptr;
}

void JseThread::swap(JseThread& other)
{
	std::swap(pThread_, other.pThread_);
	std::swap(joinable_, other.joinable_);
}

void JseThread::detach() noexcept
{
	SDL_DetachThread(pThread_);
	pThread_ = nullptr;
	joinable_ = false;
}

bool JseThread::joinable() const
{
	return joinable_;
}

int JseThread::join()
{
	int result;
	SDL_WaitThread(pThread_, &result);
	pThread_ = nullptr;
	joinable_ = false;

	return result;
}

uint32_t JseThread::native_id() const
{
	return SDL_static_cast(uint32_t, SDL_GetThreadID(pThread_));
}

const char* JseThread::name() const
{
	return SDL_GetThreadName(pThread_);
}

uint32_t JseThread::current_id()
{
	return SDL_static_cast(uint32_t, SDL_GetThreadID(nullptr));
}
