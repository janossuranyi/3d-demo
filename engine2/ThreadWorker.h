#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace jsr {
	class ThreadWorker
	{
	public:
		ThreadWorker();
		virtual ~ThreadWorker();
		std::string GetName() const
		{
			return name;
		}
		bool IsRunning() const
		{
			return isRunning;
		}
		bool IsTerminating() const
		{
			return isTerminating;
		}
		bool StartThread(const std::string& name);
		void StopThread(bool wait);
		bool StartWorkerThread(const std::string& name);
		bool IsWorkDone();
		void WaitForThread();
		virtual void SignalWork();

	protected:
		virtual int Run();
	private:
		std::string name;
		std::thread thread;
		bool isWorker;
		bool isRunning;
		bool isTerminating;
		bool hasWork;
		bool workDone;
		std::mutex signalMutex;
		std::condition_variable signalDone;
		std::condition_variable signalHasWork;
		int	threadProc();
	};
}