#pragma once

#include "./ThreadWorker.h"

#include <array>
#include <vector>
#include <atomic>

namespace jsr {

	const int MAX_TASK_NUM = 32;	// must be 2^n

	typedef void(*taskfun_t)(void*);

	struct task_t
	{
		taskfun_t function;
		void* payload;
		int executed;

		task_t(taskfun_t p0, void* p1, int p2) :
			function(p0),
			payload(p1),
			executed(p2) {}
	};

	struct taskListThreadState_t
	{
		class TaskList* taskList;
		int version;
		int nextIndex;

		taskListThreadState_t() = default;
		taskListThreadState_t(int version_) :
			taskList(),
			version(version_),
			nextIndex(0) {}
	};

	enum eTaskListPriority 
	{
		PRIO_LOW,
		PRIO_HIGH
	};

	enum eTaskResult
	{
		TASK_OK = 0,
		TASK_PROGRESS = 1,
		TASK_DONE = 2,
		TASK_STALLED = 4
	};

	class TaskListExecutor;
	class TaskList
	{
	public:
		TaskList(int id, int prio);
		~TaskList();
		void	AddTask(taskfun_t fn, void* data);
		void	Submit(TaskListExecutor* executor = nullptr);
		void	Submit(TaskListExecutor** pool, int numPool);
		bool	IsSubmitted() const;
		void	Wait();
		int		GetId() const;
		int		GetVersion() const;
		int		GetPriority() const;
		int		RunTasks(int threadNum, taskListThreadState_t& state, bool oneshot);

	private:
		int		InternalRunTasks(int threadNum, taskListThreadState_t& state, bool oneshot);
		int					id;
		bool				done;
		int					priority;
		std::vector<task_t>	taskList;
		std::atomic_int		taskCount;
		std::atomic_int		listLock;
		std::atomic_int		currentTask;
		std::atomic_int		executorThreadCount;
		std::atomic_int		version;
	};

	class TaskListExecutor : public ThreadWorker
	{
	public:
		TaskListExecutor();
		~TaskListExecutor();
		void Start(unsigned int threadNum);
		void AddTaskList(TaskList* p0);
	private:
		struct workerTask_t
		{
			TaskList* taskList;
			int version;
		};

		// ringbuffer
		workerTask_t taskList[ MAX_TASK_NUM ];
		unsigned int readIndex;
		unsigned int writeIndex;
		unsigned int threadNum;
		unsigned int jobCount;
		std::mutex mtx;

		int Run() override;
	};

	class TaskManager
	{
	public:
		TaskManager();
		~TaskManager();
		bool Init();
		void Shutdown();
		bool IsInitalized() const;
		void Submit(TaskList* taskList, bool threaded = true);
	private:
		bool initialized;
		std::atomic_int nextThread;
		std::vector<TaskListExecutor*> threadPool;
	};
}

