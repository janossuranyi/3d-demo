#include <cassert>
#include <thread>
#include "./TaskExecutor.h"
#include "./System.h"
#include "./Logger.h"

namespace jsr {

	TaskManager taskManager;

	TaskList::TaskList(int id_, eTaskListPriority prio) :
		id(id_),
		currentTask(0),
		listLock(0),
		done(true),
		priority(prio)
	{
		taskList.reserve(32);
	}

	TaskList::~TaskList()
	{
	}

	void TaskList::AddTask(taskfun_t fn, void* data)
	{
		taskList.emplace_back(fn, data, 0);
		taskCount++;
	}

	void TaskList::Submit(TaskListExecutor* executor)
	{
		assert(done);
		assert(listLock == 0);

		if (taskList.empty())
		{
			return;
		}

		done = false;
		currentTask = 0;

		if (!executor)
		{
			taskListThreadState_t state(GetVersion());
			RunTasks(0, state, false);
		}
		else
		{
			executor->AddTaskList(this);
			executor->SignalWork();
		}
	}

	void TaskList::Submit(TaskListExecutor** pool, int numPool)
	{
		assert(done);
		assert(listLock == 0);

		if (taskList.empty())
		{
			return;
		}

		done = false;
		currentTask = 0;

		for (int i = 0; i < numPool; ++i)
		{
			pool[i]->AddTaskList(this);
			pool[i]->SignalWork();
		}
	}

	bool TaskList::IsSubmitted() const
	{
		return !done;
	}

	void TaskList::Wait()
	{

		if (!taskList.empty())
		{
			while (taskCount.load(std::memory_order_relaxed) > 0)
			{
				std::this_thread::yield();
			}

			version++;

			// worker threads may still running
			while (executorThreadCount.load(std::memory_order_relaxed) > 0)
			{
				std::this_thread::yield();
			}

			taskList.clear();
			taskCount = 0;
			currentTask = 0;
		}

		done = true;
	}

	int TaskList::GetId() const
	{
		return id;
	}

	int TaskList::GetVersion() const
	{
		return version;
	}

	int TaskList::GetPriority() const
	{
		return priority;
	}

	int TaskList::RunTasks(int threadNum, taskListThreadState_t& state, bool oneshot)
	{
		executorThreadCount.fetch_add(1, std::memory_order_relaxed);
		int result = InternalRunTasks(threadNum, state, oneshot);
		executorThreadCount.fetch_sub(1, std::memory_order_relaxed);

		return result;
	}

	int TaskList::InternalRunTasks(int threadNum, taskListThreadState_t& state, bool oneshot)
	{
		assert(this == state.taskList);

		if (state.version != GetVersion())
		{
			return TASK_DONE;
		}

		int result = TASK_OK;

		do {
			state.nextIndex = currentTask.fetch_add(1, std::memory_order_relaxed);
			/*
			if (listLock++ == 0)
			{
				// lock aquaired (critical section)
				state.nextIndex = currentTask.fetch_add(1, std::memory_order_relaxed);
				listLock--;
			}
			else
			{
				// lock failed
				listLock--;
				return result | TASK_STALLED;
			}
			*/
			if (state.nextIndex >= taskList.size())
			{
				currentTask.store(0, std::memory_order_relaxed);
				done = true;

				return result | TASK_DONE;
			}
			
			taskList[state.nextIndex].function(taskList[state.nextIndex].payload);
			taskList[state.nextIndex].executed = 1;
			taskCount.fetch_sub(1, std::memory_order_relaxed);

			result |= TASK_PROGRESS;

		} while (!oneshot);

		return result;
	}


	TaskListExecutor::TaskListExecutor() :
		taskList(),
		readIndex(0),
		writeIndex(0),
		threadNum(0),
		jobCount(0)
	{
	}
	TaskListExecutor::~TaskListExecutor()
	{
		StopThread(true);
		Info("TaskExecutor %s, jobCount = %d", GetName().c_str(), jobCount);
	}

	void TaskListExecutor::Start(unsigned int threadNum)
	{
		this->threadNum = threadNum;
		char tmp[100];
		snprintf(tmp, 100, "jsrTask-%d", threadNum);
		StartWorkerThread(tmp);
		//Info("TaskExecutor %s started", tmp);
	}

	void TaskListExecutor::AddTaskList(TaskList* p0)
	{
		std::lock_guard<std::mutex> lock(mtx);

		while ( writeIndex - readIndex >= MAX_TASK_NUM )
		{
			// ring buffer is full
			std::this_thread::yield();
		}

		taskList[ writeIndex & (MAX_TASK_NUM - 1) ].taskList = p0;
		taskList[ writeIndex & (MAX_TASK_NUM - 1) ].version = p0->GetVersion();
		++writeIndex;
	}

	int TaskListExecutor::Run()
	{
		taskListThreadState_t localList[MAX_TASK_NUM];

		int numTaskLists = 0;
		int lastStalledTaskList = -1;
		while ( !IsTerminating() )
		{
			if (numTaskLists < MAX_TASK_NUM && readIndex < writeIndex)
			{
				localList[ numTaskLists ].taskList	= taskList[ readIndex & (MAX_TASK_NUM - 1) ].taskList;
				localList[ numTaskLists ].version	= taskList[ readIndex & (MAX_TASK_NUM - 1) ].version;
				localList[ numTaskLists ].nextIndex	= -1;
				numTaskLists++;
				readIndex++;
			}
			if (numTaskLists == 0)
			{
				break;
			}

			int currentTaskList = 0;
			if (lastStalledTaskList > -1)
			{
				// try to find another tasklist
				for (int i = currentTaskList; i < numTaskLists; ++i)
				{
					if (lastStalledTaskList != i && localList[i].taskList->GetPriority() > localList[currentTaskList].taskList->GetPriority())
					{
						currentTaskList = i;
					}
				}
			}

			bool singleTask = localList[currentTaskList].taskList->GetPriority() == PRIO_HIGH ? true : false;
			int result = localList[currentTaskList].taskList->RunTasks(threadNum, localList[currentTaskList], singleTask);

			jobCount++;


			if ((result & TASK_DONE) != 0)
			{
				for (int i = currentTaskList; i < numTaskLists - 1; ++i)
				{
					localList[i] = localList[i + 1];
				}
				numTaskLists--;
			}
			else if ((result & TASK_STALLED) != 0)
			{
				if (currentTaskList == lastStalledTaskList)
				{
					if ((result & TASK_PROGRESS) == 0)
					{
						Info("Stall th: %d, taskList: %d", threadNum, currentTaskList);
						std::this_thread::yield();
					}
				}
				lastStalledTaskList = currentTaskList;
			}
			else 
			{
				lastStalledTaskList = -1;
			}
		}
		return 0;
	}


	TaskManager::TaskManager() : initialized(false)
	{
	}

	TaskManager::~TaskManager()
	{
		Shutdown();
	}

	bool TaskManager::Init()
	{
		const int numCore = GetCpuCount();
		const int threads = numCore > 4 ? numCore - 2 : 2;

		Info("* TaskManager: Launching %d threads, cpu cores %d", threads, numCore);

		for (int i = 0; i < threads; ++i)
		{
			auto* t = threadPool.emplace_back(new TaskListExecutor());
			t->Start(i);
		}
		initialized = true;

		return true;
	}

	void TaskManager::Shutdown()
	{
		if (initialized)
		{
			initialized = false;
			for (auto* t : threadPool)
			{
				delete t;
			}
			threadPool.clear();
		}
	}

	bool TaskManager::IsInitalized() const
	{
		return initialized;
	}

	void TaskManager::Submit(TaskList* taskList, bool threaded)
	{
		if (!IsInitalized() || !threaded)
		{
			taskListThreadState_t state{};
			state.version = taskList->GetVersion();
			taskList->RunTasks(0, state, false);
		}
		else if (taskList->GetPriority() == PRIO_HIGH)
		{
			taskList->Submit(threadPool.data(), threadPool.size());
		}
		else
		{
			unsigned int next = (unsigned int)nextThread.fetch_add(1, std::memory_order_relaxed);
			taskList->Submit(threadPool[next % threadPool.size()]);
		}
	}
}