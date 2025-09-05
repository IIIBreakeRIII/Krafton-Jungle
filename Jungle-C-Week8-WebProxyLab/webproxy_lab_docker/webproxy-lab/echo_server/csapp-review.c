#include "csapp.h"

// open_clientfd: 클라이언트 입장에서 서버에 연결하기 위한 소켓을 생성하고, 연결까지 완료하는 헬퍼 함수.
// 성공 시 연결된 소켓의 파일 디스크립터(fd)를, 실패 시 -1을 반환.
int open_clientfd(char *hostname, char *port) {
  int clientfd;
  // addrinfo: <netdb.h>에 정의된, 네트워크 주소 정보를 담는 구조체.
  struct addrinfo hints, *listp, *p;
  // hints: getaddrinfo에 원하는 주소의 조건을 알려주는 힌트(입력용).
  // *listp: getaddrinfo가 반환하는 주소 정보 연결 리스트의 시작점(출력용).
  // *p: 리스트를 순회하기 위한 포인터.
  
  // hints 구조체를 0으로 깨끗하게 초기화.
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;  // TCP 연결(SOCK_STREAM)을 원한다고 명시.
  hints.ai_flags = AI_NUMERICSERV;  // port 인자가 'http' 같은 서비스 이름이 아닌 '80' 같은 숫자임을 명시.
  hints.ai_flags |= AI_ADDRCONFIG;  // 내 컴퓨터(클라이언트)의 IP 설정(IPv4/IPv6)에 맞는 주소만 요청.
  // Getaddrinfo: hostname과 port, hints를 기반으로 접속 가능한 서버 주소 목록을 생성하여 listp에 저장.
  // (대문자로 시작하는 것은 csapp.h에 정의된 에러 처리 래퍼 함수).
  Getaddrinfo(hostname, port, &hints, &listp);
  
  // getaddrinfo가 반환한 주소 목록을 처음부터 순회하며 연결을 시도.
  // 하나의 호스트 이름이 여러 IP 주소를 가질 수 있으므로, 성공할 때까지 모두 시도.
  for (p = listp; p; p = p->ai_next) {

    // 1. 소켓 디스크립터 생성 시도.
    // p가 가리키는 주소 정보(ai_family 등)를 이용해 소켓 생성.
    if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      continue; // 실패 시, 다음 주소 후보로 넘어감.
    }

    // 2. 생성된 소켓으로 서버에 연결 시도.
    if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) {
      break; // 연결 성공 시, 루프 탈출.
    }

    // 3. 연결 실패 시, 생성했던 소켓을 닫고 다음 주소 후보로 넘어감.
    Close(clientfd);
  }

  // getaddrinfo가 동적으로 할당했던 주소 목록 메모리를 해제 (메모리 누수 방지).
  Freeaddrinfo(listp);
  
  if (!p) { // 루프가 끝까지 돌았다면(p가 NULL), 모든 연결 시도가 실패한 것.
    return -1;
  }
  else { // 루프가 중간에 break로 중단되었다면, 연결에 성공한 것.
    return clientfd; // 성공한 소켓 디스크립터를 반환.
  }
}

// open_listenfd: 서버 입장에서 클라이언트의 연결 요청을 받을 준비가 된 '듣기 소켓(listening socket)'을 생성하고 설정하는 헬퍼 함수.
// 성공 시 듣기 소켓의 fd를, 실패 시 -1을 반환.
int open_listenfd(char *port) {
  struct addrinfo hints, *listp, *p;
  int listenfd, optval = 1;

  // hints 구조체를 0으로 깨끗하게 초기화.
  memset(&hints, 0, sizeof(struct addrinfo));

  // 듣기 소켓이 사용할 주소의 조건을 설정.
  hints.ai_socktype = SOCK_STREAM; // TCP 연결을 받아들임.
  // AI_PASSIVE: 서버용 소켓임을 명시. hostname이 NULL일 때 모든 IP 주소(0.0.0.0)에서 연결을 허용하게 함.
  // AI_ADDRCONFIG: 내 컴퓨터의 IP 설정(IPv4/IPv6)에 맞는 주소만 사용.
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
  // AI_NUMERICSERV: port 인자가 숫자임을 명시.
  // [주의] 아래 코드는 이전 flags 설정을 덮어쓰므로, 원래 의도는 |(OR) 연산자를 사용해야 함.
  // 예: hints.ai_flags |= AI_NUMERICSERV;
  hints.ai_flags = AI_NUMERICSERV;
  
  // Getaddrinfo: hostname을 NULL로 전달하여, 이 컴퓨터의 모든 IP 주소에서 연결을 허용하도록 주소 목록을 가져옴.
  Getaddrinfo(NULL, port, &hints, &listp);

  // 받아온 주소 목록을 순회하며, 바인딩(binding) 가능한 주소를 찾음.
  for (p = listp; p; p = p->ai_next) {
    // 1. 소켓 디스크립터 생성.
    if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      continue; // 실패 시 다음 후보로.
    }
    // 2. SO_REUSEADDR 옵션 설정: 서버 재시작 시 "Address already in use" 에러 방지.
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    
    // 3. 생성된 소켓을 주소(IP, 포트)에 바인딩.
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
      break; // 바인딩 성공 시 루프 탈출.
    }

    // 바인딩 실패 시, 소켓을 닫고 다음 후보로.
    Close(listenfd);
  }

  // getaddrinfo가 할당했던 주소 목록 메모리 해제.
  Freeaddrinfo(listp);
  if (!p) { // 루프가 끝까지 돌았다면, 바인딩 가능한 주소가 없었던 것.
    return -1;
  }

  // 4. 소켓을 '듣기 소켓'으로 전환하여 클라이언트의 연결 요청을 받을 준비를 함.
  // LISTENQ는 연결 요청 대기 큐(queue)의 크기.
  if (listen(listenfd, LISTENQ) < 0) {
    Close(listenfd); // listen 실패 시 소켓을 닫고 에러 반환.
    return -1;
  }
  
  return listenfd; // 모든 설정에 성공한 듣기 소켓 디스크립터를 반환.
}
