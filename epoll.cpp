#include <sys/epoll>

int init_epoll() {
    int efd = epoll_create1(0);
    if (efd < 0)
        die("epoll_create");
	
	return efd;
}


int epoll_add_fd(int efd, int fd, int events) {
    struct epoll_event event = {
		.events = events,
		.data = {
			.u32 = fd,
		}
	};
	if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) < 0)
		die("epoll_ctl");
}

int epoll_del_fd(int efd, int fd) {
	if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL) < 0)
		die("epoll_ctl");
}

int epoll_wait_fd(int efd) {
    struct epoll_event event;

	if (epoll_wait(efd, &event, 1, -1) <= 0)
		die("epoll_wait");

	return event.u32;
}