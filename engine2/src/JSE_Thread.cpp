#include "JSE.h"

JseThread::JseThread(JseThreadFunction func, const char* name, void* data)
{
	m_bJoinable = true;
	m_pThread = SDL_CreateThread(func, name, data);
}

JseThread::JseThread()
{
	m_pThread = nullptr;
	m_bJoinable = false;
}

JseThread::JseThread(JseThread&& other) noexcept
{
	m_pThread = other.m_pThread;
	m_bJoinable = other.m_bJoinable;
	other.m_pThread = nullptr;
	other.m_bJoinable = false;
}

JseThread& JseThread::operator=(JseThread&& other) noexcept
{
	m_pThread = other.m_pThread;
	m_bJoinable = other.m_bJoinable;
	other.m_pThread = nullptr;
	other.m_bJoinable = false;

	return *this;
}

JseThread::~JseThread()
{
	if (m_bJoinable) {
		SDL_WaitThread(m_pThread, nullptr);
	}

	m_pThread = nullptr;
}

void JseThread::swap(JseThread& other)
{
	std::swap(m_pThread, other.m_pThread);
	std::swap(m_bJoinable, other.m_bJoinable);
}

void JseThread::detach() noexcept
{
	SDL_DetachThread(m_pThread);
	m_pThread = nullptr;
	m_bJoinable = false;
}

bool JseThread::joinable() const
{
	return m_bJoinable;
}

int JseThread::join()
{
	int result;
	SDL_WaitThread(m_pThread, &result);
	m_pThread = nullptr;
	m_bJoinable = false;

	return result;
}

uint32_t JseThread::native_id() const
{
	return SDL_static_cast(uint32_t, SDL_GetThreadID(m_pThread));
}

const char* JseThread::name() const
{
	return SDL_GetThreadName(m_pThread);
}

uint32_t JseThread::current_id()
{
	return SDL_static_cast(uint32_t, SDL_GetThreadID(nullptr));
}
