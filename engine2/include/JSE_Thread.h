#ifndef JSE_THREAD_H
#define JSE_THREAD_H

typedef SDL_ThreadFunction JSE_ThreadFunction;

class JSE_Thread : public JSE_NonCopyable {
public:
	JSE_Thread(JSE_ThreadFunction func, const char* name, void* data);

	JSE_Thread();

	JSE_Thread(JSE_Thread&& other) noexcept;

	JSE_Thread& operator=(JSE_Thread&& other) noexcept;

	~JSE_Thread();

	void swap(JSE_Thread& other);

	void detach() noexcept;

	bool joinable() const;

	int join();

	uint32_t native_id() const;

	const char* name() const;

	static uint32_t current_id();

private:
	SDL_Thread* m_pThread;
	bool m_bJoinable;
};


#endif // !JSE_THREAD_H
