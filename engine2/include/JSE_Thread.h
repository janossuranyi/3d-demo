#ifndef JSE_THREAD_H
#define JSE_THREAD_H

typedef SDL_ThreadFunction JseThreadFunction;

class JseThread : public JseNonCopyable {
public:
	JseThread(JseThreadFunction func, const char* name, void* data);

	JseThread();

	JseThread(JseThread&& other) noexcept;

	JseThread& operator=(JseThread&& other) noexcept;

	~JseThread();

	void swap(JseThread& other);

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


#endif // !JseTHREAD_H
