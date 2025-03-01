#include "netutils.h"
#include <errno.h>
#include <string.h>
#include <netdb.h>

int open_listenfd(uint16_t port) {
  struct addrinfo hints, *results, *rp;
  char sport[8] = {0};
  int reuse = 1;
  int sock = -1;

  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (snprintf(sport, sizeof(sport) - 1, "%d", port) <= 0) {
    fprintf(stderr, "snprintf failed\n");
    return -1;
  }

  if (getaddrinfo(NULL, sport, &hints, &results) != 0) {
    fprintf(stderr, "getaddrinfo failed\n");
    return -1;
  }

  for (rp = results; rp != NULL; rp = rp->ai_next) {
    sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sock < 0) continue;

    if (setsockopt(
      sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)
    ) < 0) {
      close(sock);
      sock = -1;
      continue;
    }

    if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;
    }
    close(sock);
    sock = -1;
  }

  freeaddrinfo(results);
  if (rp == NULL || sock < 0) {
    fprintf(stderr, "Error while opening listenfd\n");
    return -1;
  }
  if (listen(sock, MAX_CONN) < 0) {
    fprintf(stderr, "Error while listening on listenfd\n");
    return -1;
  }
  return sock;
}

size_t rio_writen(int fd, const char *usrbuf, size_t n) 
{
  size_t nleft = n;
  ssize_t nwritten;
  const char *bufp = usrbuf;

  while (nleft > 0) {
    if ((nwritten = write(fd, bufp, nleft)) <= 0) {
      return 0;
    }
    nleft -= nwritten;
    bufp += nwritten;
  }

  return n;
}
