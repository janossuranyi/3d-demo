#ifndef JSE_CONCURRENT_H
#define JSE_CONCURRENT_H

class JseMutex : public JseNonMovable {
    friend class JseConditionVariable;
    public:
        /// <summary>
        /// Creates a mutex
        /// </summary>
        JseMutex();
        ~JseMutex();

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

class JseLockGuard : public JseNonMovable {
public:
    JseLockGuard() = delete;
    JseLockGuard(JseMutex& mutex);
    ~JseLockGuard();
private:
    JseMutex& m_mutex;
};

class JseUniqueLock : public JseNonCopyable {
public:
    JseUniqueLock() = delete;
    JseUniqueLock(JseMutex& mutex, bool defer_lock = false);
    void swap(JseUniqueLock& other);
    void release();
    JseUniqueLock& operator=(JseUniqueLock&& other) noexcept;
    JseMutex* mutex();
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
    JseMutex* m_pMutex;
};

class JseSemaphore : public JseNonMovable {
public:
    JseSemaphore() : JseSemaphore(0) {}
    JseSemaphore(uint32_t aInitValue);
    ~JseSemaphore() noexcept;
    int wait();
    int waitTimeout(uint32_t ms);
    int tryWait();
    void post();
private:
    SDL_sem* m_pSem;
};

class JseConditionVariable : public JseNonMovable {
public:
    JseConditionVariable();
    ~JseConditionVariable() noexcept;
    void wait(JseUniqueLock& lck, std::function<bool()> predicate);
    void wait_for(JseUniqueLock& lck, uint32_t ms, std::function<bool()> predicate);
    void notify_one() noexcept;
    void notify_all() noexcept;
private:
    SDL_cond* m_pCond;
};

#endif