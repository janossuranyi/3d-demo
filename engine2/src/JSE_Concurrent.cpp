#include "JSE.h"

JseMutex::JseMutex()
{
	m_pMutex = SDL_CreateMutex();
}

JseMutex::~JseMutex()
{
	if (m_pMutex) SDL_DestroyMutex(m_pMutex);
}

int JseMutex::lock()
{
	int r = SDL_LockMutex(m_pMutex);
	++m_lockCount;

	return r;
}

int JseMutex::tryLock()
{
	int r = SDL_TryLockMutex(m_pMutex);
	if (r != SDL_MUTEX_TIMEDOUT) {
		++m_lockCount;
	}

	return r;
}

int JseMutex::unlock()
{
	int r = -1;
	assert(m_lockCount > 0);

	r = SDL_UnlockMutex(m_pMutex);
	--m_lockCount;
	
	return r;
}

JseMutex::operator bool() const noexcept
{
	return m_pMutex != nullptr;
}

JseLockGuard::JseLockGuard(JseMutex& mutex) : m_mutex(mutex)
{
	m_mutex.lock();
}

JseLockGuard::~JseLockGuard()
{
	m_mutex.unlock();
}

JseUniqueLock::~JseUniqueLock()
{
	unlock();
	release();
}

JseUniqueLock::JseUniqueLock(JseMutex& mutex, bool defer_lock)
{
	m_pMutex = &mutex;
	if (!defer_lock) m_pMutex->lock();
}

void JseUniqueLock::swap(JseUniqueLock& other)
{
	std::swap(m_pMutex, other.m_pMutex);
}

void JseUniqueLock::release()
{
	m_pMutex = nullptr;
}

JseUniqueLock& JseUniqueLock::operator=(JseUniqueLock&& other) noexcept
{
	release();
	swap(other);

	return *this;
}

JseMutex* JseUniqueLock::mutex()
{
	return m_pMutex;
}

JseUniqueLock::operator bool() const noexcept
{
	return m_pMutex != nullptr;
}

int JseUniqueLock::lock()
{
	int r = -1;
	r = m_pMutex->lock();
	return r;
}

int JseUniqueLock::tryLock()
{
	int r = -1;
	r = m_pMutex->tryLock();
	return r;
}

int JseUniqueLock::unlock()
{
	int r = -1;
	r = m_pMutex->unlock();
	return r;
}

JseSemaphore::JseSemaphore(uint32_t aInitValue)
{
	m_pSem = SDL_CreateSemaphore(aInitValue);
}

JseSemaphore::~JseSemaphore() noexcept
{
	while(SDL_SemValue(m_pSem) == 0) SDL_SemPost(m_pSem);
	SDL_DestroySemaphore(m_pSem);
}

int JseSemaphore::wait()
{
	return SDL_SemWait(m_pSem);
}

int JseSemaphore::waitTimeout(uint32_t ms)
{
	return SDL_SemWaitTimeout(m_pSem, ms);
}

int JseSemaphore::tryWait()
{
	return SDL_SemTryWait(m_pSem);
}

void JseSemaphore::post()
{
	SDL_SemPost(m_pSem);
}

JseConditionVariable::JseConditionVariable()
{
	m_pCond = SDL_CreateCond();
}

JseConditionVariable::~JseConditionVariable() noexcept
{
	SDL_DestroyCond(m_pCond);
}

void JseConditionVariable::wait(JseUniqueLock& lck, std::function<bool()> predicate)
{
	while (!predicate()) {
		SDL_CondWait(m_pCond, lck.mutex()->m_pMutex);
	}
}

void JseConditionVariable::wait_for(JseUniqueLock& lck, uint32_t ms, std::function<bool()> predicate)
{
	while (!predicate()) {
		SDL_CondWaitTimeout(m_pCond, lck.mutex()->m_pMutex, ms);
	}
}

void JseConditionVariable::notify_one() noexcept
{
	SDL_CondSignal(m_pCond);
}

void JseConditionVariable::notify_all() noexcept
{
	SDL_CondBroadcast(m_pCond);
}
