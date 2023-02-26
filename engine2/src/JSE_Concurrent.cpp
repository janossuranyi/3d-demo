#include "JSE.h"

namespace js {
	Mutex::Mutex()
	{
		m_pMutex = SDL_CreateMutex();
	}

	Mutex::~Mutex()
	{
		if (m_pMutex) SDL_DestroyMutex(m_pMutex);
	}

	int Mutex::lock()
	{
		int r = SDL_LockMutex(m_pMutex);

		return r;
	}

	int Mutex::tryLock()
	{
		int r = SDL_TryLockMutex(m_pMutex);

		return r;
	}

	int Mutex::unlock()
	{
		int r = -1;

		r = SDL_UnlockMutex(m_pMutex);

		return r;
	}

	Mutex::operator bool() const noexcept
	{
		return m_pMutex != nullptr;
	}

	JseLockGuard::JseLockGuard(Mutex& mutex) : m_mutex(mutex)
	{
		m_mutex.lock();
	}

	JseLockGuard::~JseLockGuard()
	{
		m_mutex.unlock();
	}

	UniqueLock::~UniqueLock()
	{
		unlock();
		release();
	}

	UniqueLock::UniqueLock(Mutex& mutex, bool defer_lock)
	{
		m_pMutex = &mutex;
		if (!defer_lock) m_pMutex->lock();
	}

	void UniqueLock::swap(UniqueLock& other)
	{
		std::swap(m_pMutex, other.m_pMutex);
	}

	void UniqueLock::release()
	{
		m_pMutex = nullptr;
	}

	UniqueLock& UniqueLock::operator=(UniqueLock&& other) noexcept
	{
		release();
		swap(other);

		return *this;
	}

	Mutex* UniqueLock::mutex()
	{
		return m_pMutex;
	}

	UniqueLock::operator bool() const noexcept
	{
		return m_pMutex != nullptr;
	}

	int UniqueLock::lock()
	{
		int r = -1;
		r = m_pMutex->lock();
		return r;
	}

	int UniqueLock::tryLock()
	{
		int r = -1;
		r = m_pMutex->tryLock();
		return r;
	}

	int UniqueLock::unlock()
	{
		int r = -1;
		r = m_pMutex->unlock();
		return r;
	}

	Semaphore::Semaphore(uint32_t aInitValue)
	{
		m_pSem = SDL_CreateSemaphore(aInitValue);
	}

	Semaphore::~Semaphore() noexcept
	{
		while (SDL_SemValue(m_pSem) == 0) SDL_SemPost(m_pSem);
		SDL_DestroySemaphore(m_pSem);
	}

	int Semaphore::wait()
	{
		return SDL_SemWait(m_pSem);
	}

	int Semaphore::waitTimeout(uint32_t ms)
	{
		return SDL_SemWaitTimeout(m_pSem, ms);
	}

	int Semaphore::tryWait()
	{
		return SDL_SemTryWait(m_pSem);
	}

	void Semaphore::post()
	{
		SDL_SemPost(m_pSem);
	}

	ConditionVariable::ConditionVariable()
	{
		m_pCond = SDL_CreateCond();
	}

	ConditionVariable::~ConditionVariable() noexcept
	{
		SDL_DestroyCond(m_pCond);
	}

	void ConditionVariable::wait(UniqueLock& lck, std::function<bool()> predicate)
	{
		while (!predicate()) {
			SDL_CondWait(m_pCond, lck.mutex()->m_pMutex);
		}
	}

	void ConditionVariable::wait_for(UniqueLock& lck, uint32_t ms, std::function<bool()> predicate)
	{
		while (!predicate()) {
			SDL_CondWaitTimeout(m_pCond, lck.mutex()->m_pMutex, ms);
		}
	}

	void ConditionVariable::notify_one() noexcept
	{
		SDL_CondSignal(m_pCond);
	}

	void ConditionVariable::notify_all() noexcept
	{
		SDL_CondBroadcast(m_pCond);
	}
}