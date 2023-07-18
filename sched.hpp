#if !defined(SCHED_H)
#define SCHED_H

#include <vector>
#include <map>

#define Q_EMPTY -1

enum SchedulableEntityTypes
{
	REQUEST,
	RESPONSE
};

class SchedulableEntity
{
public:
	virtual enum SchedulableEntityTypes get_type() = 0;
	virtual ~SchedulableEntity(){};
};

void sched_queue_task(std::map<int, SchedulableEntity *> &tasks, int fd, SchedulableEntity *task);
void sched_unqueue_task(std::map<int, SchedulableEntity *> &tasks, int fd);

#endif // SCHED_H
