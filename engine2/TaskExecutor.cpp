#include <cassert>
#include <thread>
#include "./TaskExecutor.h"

namespace jsr {

	TaskList::TaskList(int id_, int prio) :
		id(id_),
		currentTask(0),
		listLock(0),
		done(true),
		priority(prio)
	{
		taskList.reserve(32);
	}

	void TaskList::AddTask(taskfun_t fn, void* data)
	{
		taskList.emplace_back(fn, data, 0);
		taskCount.fetch_add(1, std::memory_order_release);
	}

	void TaskList::Submit(TaskExecutor* executor)
	{
		assert(done);
		assert(listLock == 0);

		done = false;
		currentTask = 0;

		if (taskList.empty())
		{
			return;
		}

		if (!executor)
		{
			taskListState_t state(GetVersion());
			RunTasks(0, state, false);
		}
		else
		{
			executor->AddTaskList(this);
			executor->SignalWork();
		}
	}

	void TaskList::Submit(TaskExecutor** pool, int numPool)
	{
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
			while (taskCount.load(std::memory_order_acquire) > 0)
			{
				std::this_thread::yield();
			}

			version.fetch_add(1, std::memory_order_release);

			while (executorThreadCount.load(std::memory_order_acquire) > 0)
			{
				std::this_thread::yield();
			}

			taskList.clear();
			taskCount.store(0);
		}		
		done = true;
	}

	int TaskList::GetId() const
	{
		return id;
	}

	int TaskList::GetVersion() const
	{
		return version.load(std::memory_order_acquire);
	}

	int TaskList::GetPriority() const
	{
		return priority;
	}

	int TaskList::RunTasks(int threadNum, taskListState_t& state, bool oneshot)
	{
		executorThreadCount.fetch_add(1, std::memory_order_release);
		int result = InternalRunTasks(threadNum, state, oneshot);
		executorThreadCount.fetch_sub(1, std::memory_order_release);

		return result;
	}

	int TaskList::InternalRunTasks(int threadNum, taskListState_t& state, bool oneshot)
	{
		if (state.version != GetVersion())
		{
			return TASK_DONE;
		}

		int result = TASK_OK;

		do {
			if (listLock.fetch_add(1, std::memory_order_acq_rel) == 0)
			{
				state.nextIndex = currentTask.fetch_add(1, std::memory_order_relaxed);
				listLock.fetch_sub(1, std::memory_order_release);
			}
			else
			{
				listLock.fetch_sub(1, std::memory_order_release);
				return result | TASK_STALLED;
			}

			if (state.nextIndex >= taskList.size())
			{
				return result | TASK_DONE;
			}

			taskList[state.nextIndex].function(taskList[state.nextIndex].payload);
			taskList[state.nextIndex].executed = 1;
			taskCount.fetch_sub(1, std::memory_order_release);

			result |= TASK_PROGRESS;

		} while (!oneshot);

		return result;
	}


	TaskExecutor::TaskExecutor() :
		taskList(),
		first(0),
		last(0),
		threadNum(0)
	{
	}

	void TaskExecutor::Start(unsigned int threadNum)
	{
		this->threadNum = threadNum;
		char tmp[100];
		snprintf(tmp, 100, "SJTask-%d", threadNum);
		StartWorkerThread(tmp);
	}

	void TaskExecutor::AddTaskList(TaskList* p0)
	{
		std::lock_guard<std::mutex> lock(mtx);

		while (last - first >= taskList.size())
		{
			std::this_thread::yield();
		}

		taskList[last & (MAX_TASK_NUM - 1)].taskList = p0;
		taskList[last & (MAX_TASK_NUM - 1)].version = p0->GetVersion();
		last++;
	}

	int TaskExecutor::Run()
	{
		taskListState_t localList[MAX_TASK_NUM];

		int numTaskLists = 0;
		int lastStalledTaskList = -1;
		while (!IsTerminating())
		{
			if (numTaskLists < MAX_TASK_NUM && first < last)
			{
				localList[numTaskLists].taskList = taskList[first & (MAX_TASK_NUM - 1)].taskList;
				localList[numTaskLists].version = taskList[first & (MAX_TASK_NUM - 1)].version;
				localList[numTaskLists].nextIndex = -1;
				numTaskLists++;
				first++;
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
					if (lastStalledTaskList != i)
					{
						currentTaskList = i;
					}
				}
			}

			bool singleTask = localList[currentTaskList].taskList->GetPriority() == PRIO_HIGH ? true : false;
			int result = localList[currentTaskList].taskList->RunTasks(threadNum, localList[currentTaskList], singleTask);

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
	bool TaskListManager::IsInitalized() const
	{
		return initialized;
	}
}