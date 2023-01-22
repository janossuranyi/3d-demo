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

class JseAtomicInt
{
private:
    SDL_atomic_t    value_;
public:
    JseAtomicInt();
    JseAtomicInt(int v);
    void Set(int x);
    int Get() const;
    int Add(int x);
    int Inc();
    int Dec();
    bool CompareAndSet(int oldval, int newval);
    static bool CompareAndSetPtr(void** a, void* oldval, void* newval);
    static void* SetPtr(void** a, void* v);
    static void* GetPtr(void** a);
    int value() const;
    void operator=(int v);
    int operator+=(int v);
    int operator-=(int v);
    int operator+(int v);
    int operator-(int v);
    int operator++();
    int operator--();
    operator int() const;
};

inline JseAtomicInt::JseAtomicInt(int v)
{
    Set(v);
}

inline JseAtomicInt::JseAtomicInt() : JseAtomicInt(0) {}

inline int JseAtomicInt::Get() const
{
    return SDL_AtomicGet(const_cast<SDL_atomic_t*>(&value_));
}
inline void JseAtomicInt::Set(int v)
{
    SDL_AtomicSet(&value_, v);
}
inline int JseAtomicInt::Add(int v)
{
    return SDL_AtomicAdd(&value_, v);
}
inline int JseAtomicInt::Inc()
{
    return Add(1);
}
inline bool JseAtomicInt::CompareAndSet(int oldval, int newval)
{
    return SDL_AtomicCAS(&value_, oldval, newval) == SDL_TRUE;
}
inline bool JseAtomicInt::CompareAndSetPtr(void** a, void* oldval, void* newval)
{
    return SDL_AtomicCASPtr(a, oldval, newval) == SDL_TRUE;
}
inline void* JseAtomicInt::SetPtr(void** a, void* v)
{
    return SDL_AtomicSetPtr(a, v);
}
inline void* JseAtomicInt::GetPtr(void** a)
{
    return SDL_AtomicGetPtr(a);
}
inline int JseAtomicInt::value() const
{
    return Get();
}

inline void JseAtomicInt::operator=(int v)
{
    Set(v);
}

inline int JseAtomicInt::operator+=(int v)
{
    return Add(v) + v;
}

inline int JseAtomicInt::operator-=(int v)
{
    return Add(-v) - v;
}

inline int JseAtomicInt::operator+(int v)
{
    return Get() + v;
}

inline int JseAtomicInt::operator-(int v)
{
    return Get() - v;
}

inline int JseAtomicInt::operator++()
{
    return Add(1);
}

inline int JseAtomicInt::operator--()
{
    return Add(-1);
}

inline JseAtomicInt::operator int() const
{
    return Get();
}


#endif