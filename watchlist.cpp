#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif

#include "webserv.hpp"
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>

int init_watchlist()
{
#ifdef __APPLE__
	int kfd = kqueue();
	assert_msg(kfd != -1, "kqueue: " << strerror(errno));
	return kfd;
#elif __linux__
	int efd = epoll_create(1);
	assert_msg(efd != -1, "epoll_create1: " << strerror(errno));
	return efd;
#endif
}

void watchlist_add_fd(int wfd, int fd, uint32_t events)
{
#ifdef __APPLE__
	struct kevent event;
	EV_SET(&event, fd, events, EV_ADD, 0, 0, (void *)(long)fd);
	assert_msg(kevent(wfd, &event, 1, NULL, 0, NULL) == 0, "kevent: " << strerror(errno));
#elif __linux__
	struct epoll_event event = {.events = events,
								.data = {
									.fd = fd,
								}};
	assert_msg(epoll_ctl(wfd, EPOLL_CTL_ADD, fd, &event) == 0, "epoll_ctl: " << strerror(errno));
#endif
}

void watchlist_del_fd(int wfd, int fd)
{
#ifdef __APPLE__
	struct kevent event;
	EV_SET(&event, fd, 0, EV_DELETE, 0, 0, NULL);
	assert_msg(kevent(wfd, &event, 1, NULL, 0, NULL) < 0, "kevent: " << strerror(errno));
#elif __linux__
	assert_msg(epoll_ctl(wfd, EPOLL_CTL_DEL, fd, NULL) == 0, "epoll_ctl: " << strerror(errno));
#endif
}

/**
 * ret == 0 is reserved to mean no requests are currently waiting
 */
int watchlist_wait_fd(int wfd)
{
#ifdef __APPLE__
	struct kevent event;
	int ret = kevent(wfd, NULL, 0, &event, 1, NULL);
	assert_msg(ret == 1, "kevent: " << strerror(errno));
	return (int)(long)event.udata;
#elif __linux__
	struct epoll_event event;
	int ret = epoll_wait(wfd, &event, 1, -1);
	assert_msg(ret == 1, "epoll_wait: " << strerror(errno));
	return event.data.fd;
#endif
}

void watchlist_insert(int wfd, int fd)
{
	watchlist_add_fd(
		wfd,
		fd,
#ifdef __APPLE__
		EVFILT_READ | EVFILT_WRITE
#elif __linux__
		EPOLLIN | EPOLLOUT
#endif
	);
}
