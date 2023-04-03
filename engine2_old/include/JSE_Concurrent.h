#ifndef JSE_CONCURRENT_H
#define JSE_CONCURRENT_H

namespace js
{
    class Mutex : public NonMovable {
        friend class ConditionVariable;
    public:
        /// <summary>
        /// Creates a mutex
        /// </summary>
        Mutex();
        ~Mutex();

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

    class JseLockGuard : public NonMovable {
    public:
        JseLockGuard() = delete;
        JseLockGuard(Mutex& mutex);
        ~JseLockGuard();
    private:
        Mutex& m_mutex;
    };

    class UniqueLock : public NonCopyable {
    public:
        UniqueLock() = delete;
        ~UniqueLock();
        UniqueLock(Mutex& mutex, bool defer_lock = false);
        void swap(UniqueLock& other);
        void release();
        UniqueLock& operator=(UniqueLock&& other) noexcept;
        Mutex* mutex();
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
        Mutex* m_pMutex;
    };

    class Semaphore : public NonMovable {
    public:
        Semaphore() : Semaphore(0) {}
        Semaphore(uint32_t aInitValue);
        ~Semaphore() noexcept;
        int wait();
        int waitTimeout(uint32_t ms);
        int tryWait();
        void post();
    private:
        SDL_sem* m_pSem;
    };

    class ConditionVariable : public js::NonMovable {
    public:
        ConditionVariable();
        ~ConditionVariable() noexcept;
        void wait(UniqueLock& lck, std::function<bool()> predicate);
        void wait_for(UniqueLock& lck, uint32_t ms, std::function<bool()> predicate);
        void notify_one() noexcept;
        void notify_all() noexcept;
    private:
        SDL_cond* m_pCond;
    };

    class AtomicInt
    {
    private:
        SDL_atomic_t    value_;
    public:
        AtomicInt();
        AtomicInt(int v);
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

    inline AtomicInt::AtomicInt(int v)
    {
        Set(v);
    }

    inline AtomicInt::AtomicInt() : AtomicInt(0) {}

    inline int AtomicInt::Get() const
    {
        return SDL_AtomicGet(const_cast<SDL_atomic_t*>(&value_));
    }
    inline void AtomicInt::Set(int v)
    {
        SDL_AtomicSet(&value_, v);
    }
    inline int AtomicInt::Add(int v)
    {
        return SDL_AtomicAdd(&value_, v);
    }
    inline int AtomicInt::Inc()
    {
        return Add(1);
    }
    inline bool AtomicInt::CompareAndSet(int oldval, int newval)
    {
        return SDL_AtomicCAS(&value_, oldval, newval) == SDL_TRUE;
    }
    inline bool AtomicInt::CompareAndSetPtr(void** a, void* oldval, void* newval)
    {
        return SDL_AtomicCASPtr(a, oldval, newval) == SDL_TRUE;
    }
    inline void* AtomicInt::SetPtr(void** a, void* v)
    {
        return SDL_AtomicSetPtr(a, v);
    }
    inline void* AtomicInt::GetPtr(void** a)
    {
        return SDL_AtomicGetPtr(a);
    }
    inline int AtomicInt::value() const
    {
        return Get();
    }

    inline void AtomicInt::operator=(int v)
    {
        Set(v);
    }

    inline int AtomicInt::operator+=(int v)
    {
        return Add(v) + v;
    }

    inline int AtomicInt::operator-=(int v)
    {
        return Add(-v) - v;
    }

    inline int AtomicInt::operator+(int v)
    {
        return Get() + v;
    }

    inline int AtomicInt::operator-(int v)
    {
        return Get() - v;
    }

    inline int AtomicInt::operator++()
    {
        return Add(1);
    }

    inline int AtomicInt::operator--()
    {
        return Add(-1);
    }

    inline AtomicInt::operator int() const
    {
        return Get();
    }

}
#endif