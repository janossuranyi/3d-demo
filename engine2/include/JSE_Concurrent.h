#ifndef JSE_CONCURRENT_H
#define JSE_CONCURRENT_H

class JSE_Mutex : public JSE_NonMovable {
    friend class JSE_ConditionVariable;
    public:
        /// <summary>
        /// Creates a mutex
        /// </summary>
        JSE_Mutex();
        ~JSE_Mutex();

        /// <summary>
        /// Lock the mutex.
        /// </summary>
        /// <returns>Return 0, or -1 on error.</returns>
        int lock();
        
        /// <summary>
        /// Try to lock a mutex without blocking.
        /// </summary>
        /// <returns>Returns 0, SDL_MUTEX_TIMEDOUT, or -1 on error</returns>
        int tryLock();

        /// <summary>
        /// Unlock the mutex.
        /// </summary>
        /// <returns>0, or -1 on error.</returns>
        int unlock();

        explicit operator bool() const noexcept;
private:
        SDL_mutex* m_pMutex;
};

class JSE_LockGuard : public JSE_NonMovable {
public:
    JSE_LockGuard() = delete;
    JSE_LockGuard(JSE_Mutex& mutex);
    ~JSE_LockGuard();
private:
    JSE_Mutex& m_mutex;
};

class JSE_UniqueLock : public JSE_NonCopyable {
public:
    JSE_UniqueLock() = delete;
    JSE_UniqueLock(JSE_Mutex& mutex, bool defer_lock = false);
    void swap(JSE_UniqueLock& other);
    void release();
    JSE_UniqueLock& operator=(JSE_UniqueLock&& other) noexcept;
    JSE_Mutex* mutex();
    explicit operator bool() const noexcept;
    /// <summary>
    /// Lock the mutex.
    /// </summary>
    /// <returns>Return 0, or -1 on error.</returns>
    int lock();

    /// <summary>
    /// Try to lock a mutex without blocking.
    /// </summary>
    /// <returns>Returns 0, SDL_MUTEX_TIMEDOUT, or -1 on error</returns>
    int tryLock();

    /// <summary>
    /// Unlock the mutex.
    /// </summary>
    /// <returns>0, or -1 on error.</returns>
    int unlock();

private:
    JSE_Mutex* m_pMutex;
};

class JSE_Semaphore : public JSE_NonMovable {
public:
    JSE_Semaphore() : JSE_Semaphore(0) {}
    JSE_Semaphore(uint32_t aInitValue);
    ~JSE_Semaphore() noexcept;
    int wait();
    int waitTimeout(uint32_t ms);
    int tryWait();
    void post();
private:
    SDL_sem* m_pSem;
};

class JSE_ConditionVariable : public JSE_NonMovable {
public:
    JSE_ConditionVariable();
    ~JSE_ConditionVariable() noexcept;
    void wait(JSE_UniqueLock& lck, std::function<bool()> predicate);
    void wait_for(JSE_UniqueLock& lck, uint32_t ms, std::function<bool()> predicate);
    void notify_one() noexcept;
    void notify_all() noexcept;
private:
    SDL_cond* m_pCond;
};

#endif