#include "JSE.h"

JSE_Thread::JSE_Thread(JSE_ThreadFunction func, const char* name, void* data)
{
	m_bJoinable = true;
	m_pThread = SDL_CreateThread(func, name, data);
}

JSE_Thread::JSE_Thread()
{
	m_pThread = nullptr;
	m_bJoinable = false;
}

JSE_Thread::JSE_Thread(JSE_Thread&& other) noexcept
{
	m_pThread = other.m_pThread;
	m_bJoinable = other.m_bJoinable;
	other.m_pThread = nullptr;
	other.m_bJoinable = false;
}

JSE_Thread& JSE_Thread::operator=(JSE_Thread&& other) noexcept
{
	m_pThread = other.m_pThread;
	m_bJoinable = other.m_bJoinable;
	other.m_pThread = nullptr;
	other.m_bJoinable = false;

	return *this;
}

JSE_Thread::~JSE_Thread()
{
	if (m_bJoinable) {
		SDL_WaitThread(m_pThread, nullptr);
	}

	m_pThread = nullptr;
}

void JSE_Thread::swap(JSE_Thread& other)
{
	std::swap(m_pThread, other.m_pThread);
	std::swap(m_bJoinable, other.m_bJoinable);
}

void JSE_Thread::detach() noexcept
{
	SDL_DetachThread(m_pThread);
	m_pThread = nullptr;
	m_bJoinable = false;
}

bool JSE_Thread::joinable() const
{
	return m_bJoinable;
}

int JSE_Thread::join()
{
	int result;
	SDL_WaitThread(m_pThread, &result);
	m_pThread = nullptr;
	m_bJoinable = false;

	return result;
}

uint32_t JSE_Thread::native_id() const
{
	return SDL_static_cast(uint32_t, SDL_GetThreadID(m_pThread));
}

const char* JSE_Thread::name() const
{
	return SDL_GetThreadName(m_pThread);
}

uint32_t JSE_Thread::current_id()
{
	return SDL_static_cast(uint32_t, SDL_GetThreadID(nullptr));
}
