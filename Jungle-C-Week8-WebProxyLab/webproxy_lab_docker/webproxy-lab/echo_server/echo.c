#include "csapp.h"

void echo(int connfd) {
  ssize_t n;
  char buf[MAXLINE];
  int count = 0;

  while ((n = read(connfd, buf, MAXLINE)) > 0) {
    count++;
    printf("packet %d: server received %zd bytes\n", count, n);
    write(connfd, buf, n);
  }
}

