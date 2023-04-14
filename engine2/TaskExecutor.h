#pragma once

#include "./ThreadWorker.h"

#include <array>
#include <vector>
#include <atomic>

namespace jsr {

	const int MAX_TASK_NUM = 32;

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

	struct taskListState_t
	{
		class TaskList* taskList;
		int version;
		int nextIndex;

		taskListState_t() = default;
		taskListState_t(int version_) :
			taskList(),
			version(version_),
			nextIndex(0) {}
	};

	enum taskResult_t
	{
		TASK_OK = 0,
		TASK_PROGRESS = 1,
		TASK_DONE = 2,
		TASK_STALLED = 4
	};

	class TaskExecutor;
	class TaskList
	{
	public:
		TaskList(int id);

		void	AddTask(taskfun_t fn, void* data);
		void	Submit(TaskExecutor* executor = nullptr);
		void	Submit(TaskExecutor** pool, int numPool);
		bool	IsSubmitted() const;
		void	Wait();
		int		GetId() const;
		int		GetVersion() const;
		int		RunTasks(int threadNum, taskListState_t& state, bool oneshot);

	private:
		int		InternalRunTasks(int threadNum, taskListState_t& state, bool oneshot);
		int					id;
		bool				done;
		std::vector<task_t>	taskList;
		std::atomic_int		taskCount;
		std::atomic_int		listLock;
		std::atomic_int		currentTask;
		std::atomic_int		executorThreadCount;
		std::atomic_int		version;

	};

	struct workerTask_t
	{
		TaskList* taskList;
		int version;
	};

	class TaskExecutor : public ThreadWorker
	{
	public:
		TaskExecutor();
		void Start(unsigned int threadNum);
		void AddTaskList(TaskList* p0);
		void SetSingleTask(bool b);
		bool GetSingleTask() const;
	private:
		bool singleTask;
		std::array<workerTask_t, MAX_TASK_NUM> taskList;
		unsigned int first;
		unsigned int last;
		unsigned int threadNum;
		std::mutex mtx;

		int Run() override;
	};
}

