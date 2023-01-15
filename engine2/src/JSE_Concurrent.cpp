#include "JSE.h"

JSE_Mutex::JSE_Mutex()
{
	m_pMutex = SDL_CreateMutex();
}

JSE_Mutex::~JSE_Mutex()
{
	if (m_pMutex) SDL_DestroyMutex(m_pMutex);
}

int JSE_Mutex::lock()
{
	return SDL_LockMutex(m_pMutex);
}

int JSE_Mutex::tryLock()
{
	return SDL_TryLockMutex(m_pMutex);
}

int JSE_Mutex::unlock()
{
	return SDL_UnlockMutex(m_pMutex);
}

JSE_Mutex::operator bool() const noexcept
{
	return m_pMutex != nullptr;
}

JSE_LockGuard::JSE_LockGuard(JSE_Mutex& mutex) : m_mutex(mutex)
{
	m_mutex.lock();
}

JSE_LockGuard::~JSE_LockGuard()
{
	m_mutex.unlock();
}

JSE_UniqueLock::JSE_UniqueLock(JSE_Mutex& mutex, bool defer_lock)
{
	m_pMutex = &mutex;
	if (!defer_lock) m_pMutex->lock();
}

void JSE_UniqueLock::swap(JSE_UniqueLock& other)
{
	std::swap(m_pMutex, other.m_pMutex);
}

void JSE_UniqueLock::release()
{
	m_pMutex = nullptr;
}

JSE_UniqueLock& JSE_UniqueLock::operator=(JSE_UniqueLock&& other) noexcept
{
	release();
	swap(other);

	return *this;
}

JSE_Mutex* JSE_UniqueLock::mutex()
{
	return m_pMutex;
}

JSE_UniqueLock::operator bool() const noexcept
{
	return m_pMutex != nullptr;
}

int JSE_UniqueLock::lock()
{
	int r = -1;
	r = m_pMutex->lock();

	return r;
}

int JSE_UniqueLock::tryLock()
{
	int r = -1;
	r = m_pMutex->tryLock();
	return r;
}

int JSE_UniqueLock::unlock()
{
	int r = -1;
	r = m_pMutex->unlock();
	return r;
}

JSE_Semaphore::JSE_Semaphore(uint32_t aInitValue)
{
	m_pSem = SDL_CreateSemaphore(aInitValue);
}

JSE_Semaphore::~JSE_Semaphore() noexcept
{
	while(SDL_SemValue(m_pSem) == 0) SDL_SemPost(m_pSem);
	SDL_DestroySemaphore(m_pSem);
}

int JSE_Semaphore::wait()
{
	return SDL_SemWait(m_pSem);
}

int JSE_Semaphore::waitTimeout(uint32_t ms)
{
	return SDL_SemWaitTimeout(m_pSem, ms);
}

int JSE_Semaphore::tryWait()
{
	return SDL_SemTryWait(m_pSem);
}

void JSE_Semaphore::post()
{
	SDL_SemPost(m_pSem);
}

JSE_ConditionVariable::JSE_ConditionVariable()
{
	m_pCond = SDL_CreateCond();
}

JSE_ConditionVariable::~JSE_ConditionVariable() noexcept
{
	SDL_DestroyCond(m_pCond);
}

void JSE_ConditionVariable::wait(JSE_UniqueLock& lck, std::function<bool()> predicate)
{
	while (!predicate()) {
		SDL_CondWait(m_pCond, lck.mutex()->m_pMutex);
	}
}

void JSE_ConditionVariable::wait_for(JSE_UniqueLock& lck, uint32_t ms, std::function<bool()> predicate)
{
	while (!predicate()) {
		SDL_CondWaitTimeout(m_pCond, lck.mutex()->m_pMutex, ms);
	}
}

void JSE_ConditionVariable::notify_one() noexcept
{
	SDL_CondSignal(m_pCond);
}

void JSE_ConditionVariable::notify_all() noexcept
{
	SDL_CondBroadcast(m_pCond);
}
