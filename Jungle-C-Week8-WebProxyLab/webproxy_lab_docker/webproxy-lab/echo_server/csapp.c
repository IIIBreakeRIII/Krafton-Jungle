#include "csapp.h"

// fd == file descriptor
// 열려있는 파일이나 다른 입출력 자원을 식별하기 위한 식별자 / 핸들러
int open_clientfd(char *hostname, char *port) {
  int clientfd;
  // addrinfo = provided by <netdb.h>
  // hints : 설정값 전달용
  // *listp : 결과 리스트 시작점 저장용
  // *p : 리스트 순회용
  struct addrinfo hints, *listp, *p;
  
  // 서버 주소 정보 목록을 가져옴
  // hints 구조체를 0으로 깨끗하게 초기화
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;  // using tcp connection
  hints.ai_flags = AI_NUMERICSERV;  // port param is numeric like 80
  hints.ai_flags |= AI_ADDRCONFIG;  // current ip address setting of my laptop(ipv4 or ipv6)
  Getaddrinfo(hostname, port, &hints, &listp);  // get address list using hostname & port num
  
  // 받아온 주소 목록을 처음부터 순회하며 접속 시도
  for (p = listp; p; p = p->ai_next) {

    // Create a socket descriptor
    // Case 1. Socket failed, try the next
    // p 가 가르키는 주소 정보(ai_family 등)를 이용해 소켓 생성
    // 실패 시 다음 주소 후보로 넘어감
    if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) { continue; }

    // Case 2. Connect to the server (Success)
    // 생성된 소켓으로 서버에 연결 시도
    // 성공 시 루프 탈출(break)
    if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) { break; }

    // Case 3. Connect failed, try another
    // 실패 시, 생성했던 소켓 닫고 다음 주소 후보로 넘어감
    Close(clientfd);
  }

  // Clean up -> 메모리 누수 방지를 위한 주소 목록 메모리 해제
  Freeaddrinfo(listp);
  
  // All connects failed -> 루프가 끝까지 돌았다면, 모든 연결 시도가 실패
  if (!p) { return -1; }
  // The last connect succeeded
  else { return clientfd; }
}

int open_listenfd(char *port) {
  struct addrinfo hints, *listp, *p;
  int listenfd, optval = 1;

  // Get a list of potential server address
  memset(&hints, 0, sizeof(struct addrinfo));

  // Accept Connections
  hints.ai_socktype = SOCK_STREAM;
  // ... on any IP Address
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
  // ... using port number
  hints.ai_flags = AI_NUMERICSERV;
  
  Getaddrinfo(NULL, port, &hints, &listp);

  // Walk the list for one that we can bind to
  for (p = listp; p; p = p->ai_next) {
    // Create a socket descriptor
    // Case 1. Socket failed, try the next
    if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) { continue; }
    // Case 2. Eliminates "Address already in use" error from bind
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    // Case 3. Bind the descriptor to the address (Success)
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) { break; }

    // Bind failed, try the next
    Close(listenfd);
  }

  // Clean up
  Freeaddrinfo(listp);
  // No address worked
  if (!p) { return -1; }

  // Make it a listening socket ready to accept connection requests
  if (listen(listenfd, LISTENQ) < 0) {
    Close(listenfd);
    return -1;
  }
  
  return listenfd;
}
