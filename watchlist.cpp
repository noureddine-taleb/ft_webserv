#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif

#include "webserv.hpp"
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>

int init_watchlist() {
#ifdef __APPLE__
  int kfd = kqueue();
  assert(kfd != -1, "kqueue() failed");
  return kfd;
#elif __linux__
  int efd = epoll_create1(0);
  assert(efd != -1, "epoll_create1() failed");
  return efd;
#endif
}

void watchlist_add_fd(int wfd, int fd, uint32_t events) {
#ifdef __APPLE__
  struct kevent event;
  EV_SET(&event, fd, events, EV_ADD, 0, 0, (void *)(long)fd);
  assert(kevent(wfd, &event, 1, NULL, 0, NULL) == 0, "kevent() failed");
#elif __linux__
  struct epoll_event event = {.events = events,
                              .data = {
                                  .fd = fd,
                              }};
  assert(epoll_ctl(wfd, EPOLL_CTL_ADD, fd, &event) == 0, "epoll_ctl() failed");
#endif
}

void watchlist_del_fd(int wfd, int fd) {
#ifdef __APPLE__
  struct kevent event;
  EV_SET(&event, fd, 0, EV_DELETE, 0, 0, NULL);
  assert(kevent(wfd, &event, 1, NULL, 0, NULL) < 0, "kevent() failed");
#elif __linux__
  assert(epoll_ctl(wfd, EPOLL_CTL_DEL, fd, NULL) == 0, "epoll_ctl() failed");
#endif
}

/**
 * ret == 0 is reserved to mean no requests are currently waiting
 */
int watchlist_wait_fd(int wfd) {
#ifdef __APPLE__
  struct kevent event;
  struct timespec ts = {.tv_nsec = 1000};
  int ret = kevent(wfd, NULL, 0, &event, 1, &ts);
  if (ret == 0)
    return WATCHL_NO_PENDING;
  assert(ret == 1, "kevent() wait failed");
  return (int)(long)event.udata;
#elif __linux__
  struct epoll_event event;
  int ret = epoll_wait(wfd, &event, 1, 1);
  if (ret == 0)
    return WATCHL_NO_PENDING;
  assert(ret == 1, "epoll_wait() failed");
  return event.data.fd;
#endif
}
