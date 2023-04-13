#include <chrono>
#include "./ThreadWorker.h"
#include "./Logger.h"

namespace jsr {

	ThreadWorker::ThreadWorker():
		isRunning(false),
		isWorker(false),
		isTerminating(false),
		hasWork(false),
		workDone(false)
	{
	}

	ThreadWorker::~ThreadWorker()
	{
		StopThread(false);
		if (thread.joinable())
		{
			thread.join();
		}
	}

	bool ThreadWorker::StartThread(const std::string& name)
	{
		if (isRunning)
		{
			return false;
		}

		this->name = name;
		this->isTerminating = false;
		
		thread = std::thread(&ThreadWorker::threadProc, this);
		
		this->isRunning = true;

		return true;
	}

	void ThreadWorker::StopThread(bool wait)
	{
		if (isWorker && isRunning)
		{
			
			std::unique_lock<std::mutex> lck(signalMutex);
			isTerminating = true;
			hasWork = true;
			signalHasWork.notify_all();

			if (wait)
			{
				signalDone.wait(lck, [&] { return workDone; });
			}
		}
		else
		{
			isTerminating = true;
		}
	}

	bool ThreadWorker::StartWorkerThread(const std::string& name)
	{
		if (isRunning)
		{
			return false;
		}
		this->isWorker = true;

		bool result = StartThread(name);

		std::unique_lock<std::mutex> lck(signalMutex);
		signalDone.wait(lck, [&] { return workDone; });

		isRunning = true;

		return result;
	}

	bool ThreadWorker::IsWorkDone()
	{
		std::unique_lock<std::mutex> lck(signalMutex);
		return workDone;
	}

	void ThreadWorker::WaitForThread()
	{
		if (isWorker)
		{
			std::unique_lock<std::mutex> lck(signalMutex);
			signalDone.wait(lck, [&] {return workDone; });
		}
		else if (isRunning)
		{
			isTerminating = true;
			if (thread.joinable()) { thread.join(); }
		}
	}

	void ThreadWorker::SignalWork()
	{
		if (isWorker)
		{
			std::unique_lock<std::mutex> lck(signalMutex);
			hasWork = true;
			workDone = false;
			signalHasWork.notify_all();
		}
	}

	int ThreadWorker::Run()
	{
		return 0;
	}

	int ThreadWorker::threadProc()
	{
		int ret = 0;
		try {
			if (isWorker)
			{
				while (true)
				{
					std::unique_lock<std::mutex> lck(signalMutex);
					if (hasWork)
					{
						hasWork = false;
						workDone = false;
						lck.unlock();
					}
					else
					{
						workDone = true;
						signalDone.notify_all();
						signalHasWork.wait(lck, [&] {return hasWork; });
					}

					if (isTerminating)
					{
						break;
					}

					ret = Run();
				}
				workDone = true;
				signalDone.notify_all();
			}
			else
			{
				ret = Run();
			}
		}
		catch (std::runtime_error err)
		{
			Error("Fatal error in WorkerThread: %s", err.what());
		}

		isRunning = false;

		return ret;
	}


}
