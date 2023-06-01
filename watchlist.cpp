#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif

#include <stdlib.h>
#include <iostream>
#include "webserv.hpp"

int init_watchlist() {
#ifdef __APPLE__
	int kfd;
	assert((kfd = kqueue()) != -1);
	return kfd;
#elif __linux__
	int efd;
	assert((efd = epoll_create1(0)) != -1);
	return efd;
#endif
}


void watchlist_add_fd(int wfd, int fd, uint32_t events) {
#ifdef __APPLE__
	struct kevent event;
	EV_SET(&event, fd, events, EV_ADD, 0, 0, (void *)(long)fd);
	assert(kevent(wfd, &event, 1, NULL, 0, NULL) == 0);
#elif __linux__
    struct epoll_event event = {
		.events = events,
		.data = {
			.fd = fd,
		}
	};
	assert(epoll_ctl(wfd, EPOLL_CTL_ADD, fd, &event) == 0);
#endif
}

void watchlist_del_fd(int wfd, int fd) {
#ifdef __APPLE__
	struct kevent event;
	EV_SET(&event, fd, 0, EV_DELETE, 0, 0, NULL);
	assert(kevent(wfd, &event, 1, NULL, 0, NULL) < 0);
#elif __linux__
	assert(epoll_ctl(wfd, EPOLL_CTL_DEL, fd, NULL) == 0);
#endif
}

int watchlist_wait_fd(int wfd) {
#ifdef __APPLE__
	struct kevent event;
	assert(kevent(wfd, NULL, 0, &event, 1, NULL) == 1);
	return (int)(long)event.udata;
#elif __linux__
    struct epoll_event event;
	assert(epoll_wait(wfd, &event, 1, -1) == 1);
	return event.data.fd;
#endif
}