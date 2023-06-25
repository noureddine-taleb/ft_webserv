#if !defined(SCHED_H)
#define SCHED_H

#include <vector>
#include <map>

#define Q_EMPTY -1

enum SchedulableEntityTypes {
	REQUEST,
	RESPONSE
};

class SchedulableEntity {
	public:
		int index;
		virtual enum SchedulableEntityTypes get_type() = 0;
		// todo: maybe definitions should be in source file
		virtual ~SchedulableEntity() {};
};

int sched_get_starved(std::map<int, SchedulableEntity *> &tasks);
void sched_queue_task(std::map<int, SchedulableEntity *> &tasks, int fd, SchedulableEntity *task);
void sched_unqueue_task(std::map<int, SchedulableEntity *> &tasks, int fd);

#endif // SCHED_H
