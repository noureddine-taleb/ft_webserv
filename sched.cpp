#include "webserv.hpp"
#include "sched.hpp"
#include <map>
#include <vector>

/**
 * @return
 * -1: map is empty
 * else: fd
*/
int sched_get_starved(std::map<int, SchedulableEntity *> &tasks) {
	if (tasks.empty())
		return Q_EMPTY;
	int last = tasks.begin()->second->index;
	int fd = tasks.begin()->first;
	for (std::map<int, SchedulableEntity *>::iterator it = tasks.begin();
			it != tasks.end();
			it++
		) {
			if (it->second->index < last) {
				last = it->second->index;
				fd = it->first;
			}
		}
	return fd;
}

int newest_index(std::map<int, SchedulableEntity *> &tasks) {
	if (tasks.empty())
        return 0;
	int first = 0;
    for (std::map<int, SchedulableEntity *>::iterator it = tasks.begin();
			it != tasks.end();
			it++
    ) {
			if (tasks[it->first]->index > first) {
				first = tasks[it->first]->index;
			}
    }
    return first;
}

void sched_queue_task(std::map<int, SchedulableEntity *> &tasks, int fd, SchedulableEntity *task) {
	sched_unqueue_task(tasks, fd);
	task->index = newest_index(tasks) + 1;
    tasks[fd] = task;
}

void sched_unqueue_task(std::map<int, SchedulableEntity *> &tasks, int fd) {
    if (tasks.find(fd) != tasks.end()) {
		delete tasks[fd];
		tasks.erase(fd);
	}
}