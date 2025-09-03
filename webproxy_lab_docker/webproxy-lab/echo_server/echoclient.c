#include "csapp.h"
#include <unistd.h>

int main(int argc, char **argv) {

  int clientfd;
  char *host, *port, buf[MAXLINE];
  rio_t rio;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
    exit(0);
  }

  host = argv[1];
  port = argv[2];

  clientfd = Open_clientfd(host, port);
  Rio_readinitb(&rio, clientfd);

  // while (Fgets(buf, MAXLINE, stdin) != NULL) {
  //   Rio_writen(clientfd, buf, strlen(buf));
  //   읽는 과정 생략
  //   Rio_readlineb(&rio, buf, MAXLINE);
  //   Fputs(buf, stdout);

  for (int i = 0; i < 100000; i++) {
    char *msg = "hello\n";
    Rio_writen(clientfd, msg, strlen(msg));
    usleep(50); // 0.00005초 대기
  }
  
  Close(clientfd);
  exit(0);
}
