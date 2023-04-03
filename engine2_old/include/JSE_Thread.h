#ifndef JSE_THREAD_H
#define JSE_THREAD_H

namespace js
{
	typedef SDL_ThreadFunction ThreadFunction;
	class Thread : public NonCopyable {
	public:
		Thread(ThreadFunction func, const char* name, void* data);

		Thread();

		Thread(Thread&& other) noexcept;

		Thread& operator=(Thread&& other) noexcept;

		~Thread();

		void swap(Thread& other);

		void detach() noexcept;

		bool joinable() const;

		int join();

		uint32_t native_id() const;

		const char* name() const;

		static uint32_t current_id();

	private:
		SDL_Thread* pThread_;
		bool joinable_;
	};
}

#endif // !JseTHREAD_H
