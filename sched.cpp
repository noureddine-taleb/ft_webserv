#include "webserv.hpp"
#include "sched.hpp"
#include <map>
#include <vector>


void sched_queue_task(std::map<int, SchedulableEntity *> &tasks, int fd, SchedulableEntity *task)
{
	sched_unqueue_task(tasks, fd);
	tasks[fd] = task;
}

void sched_unqueue_task(std::map<int, SchedulableEntity *> &tasks, int fd)
{
	if (tasks.find(fd) != tasks.end())
	{
		delete tasks[fd];
		tasks.erase(fd);
	}
}