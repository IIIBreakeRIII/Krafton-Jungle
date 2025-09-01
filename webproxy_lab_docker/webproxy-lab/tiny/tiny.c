#include "csapp.h"
#include <string.h>

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

// main function
int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  // 소켓 주소 구조체의 크기를 저장, 전달(e.g., IPv4, IPv6 모두 가능)
  socklen_t clientlen;
  // 소켓의 주소를 저장
  struct sockaddr_storage clientaddr;

  // Check command line args
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  
  // ./tiny 80 이 입력되었다고 가정
  // argv[1] = 80 -> 80번 포트에서 클라이언트 연결 요청을 기다리도록 리슨 소켓 생성
  listenfd = Open_listenfd(argv[1]);
  // 무한 루프 진입
  while (1) {
    clientlen = sizeof(clientaddr);
    // 클라이언트가 80포트로 접속 시도 -> 서버: 연결 수락, 새 소켓 connfd 생성
    // typedef struct sockaddr SA
    connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
    // 클라이언트 주소 구조체에서 정보 추출
    // 클라이언트의 hostname:port == 127.0.0.100:54321일 경우, port = 54321로 변환
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    // connfd로 HTTP 요청 읽기
    // static / dynamic 여부에 따라 각 메서드 실행
    doit(connfd);
    // Transaction 끝나면 연결 소켓 종료
    Close(connfd);
  }
}

void doit(int fd) {
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  // Read request line and headers
  // riot_t 버퍼 구조체 / fd 초기화
  Rio_readinitb(&rio, fd);
  // \n을 만날때까지 텍스트의 한 줄을 읽어옴
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  // sscanf : 문자열 버퍼 즉, 메모리에 이미 들어있는 문자열
  sscanf(buf, "%s %s %s", method, uri, version);
  
  // make error if not "GET"
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");

    return;
  }
  // 모든 헤더를 읽고 무시함 (빈 줄 \r\n에서 종료) : Tiny는 헤더 정보를 필요로 하지 않음
  read_requesthdrs(&rio);

  // Parse URI from GET request
  // 정적 컨텐츠인지 판별, static : 1, cgi : 0
  is_static = parse_uri(uri, filename, cgiargs);

  // 파일 존재 여부 및 접근 권한 확인
  if (stat(filename, &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");

    return;
  }
  
  // Server static content
  if (is_static) {
    // S_ISREG(sbuf.st_mode): 요청된 파일이 regular file이 맞는지 확인
    // S_IRUSR & subf.st_mode : 서버 프로세스가 이 파일에 대한 읽기 권한이 있는지 확인
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");

      return;
    }
    // serve file
    serve_static(fd, filename, sbuf.st_size);
  }
  // Serve dynamic content
  else {
    // S_IXUSR & sbuf.st_mode: 유저가 실행(execute) 권한이 있는지 확인
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");

      return;
    }
    serve_dynamic(fd, filename, cgiargs);
  }
}
// 클라이언트에 에러 메시지 전송
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
  char buf[MAXLINE], body[MAXBUF];

  // Build the HTTP response body
  // HTTP 응답 본문 생성
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor='FFFFFF'\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  // Print the HTTP response
  // Client에 HTTP 응답 헤더 전송
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  // 클라이언트에 HTTP 응답 본문 전송
  Rio_writen(fd, body, strlen(body));
}

// 요청 헤더 읽기 및 출력(Tiny는 헤더를 사용하지 않음)
void read_requesthdrs(rio_t * rp) {
  char buf[MAXLINE];
  // 요청 헤더의 끝(\r\n)까지 반복해서 읽음
  Rio_readlineb(rp, buf, MAXLINE);
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }

  return;
}

// URI를 분석하여 정적 또는 동적 콘텐츠 여부를 판별
// filename & cgiargs를 설정
int parse_uri(char *uri, char *filename, char *cgiargs) {
  char *ptr;
  
  // Static content
  if (!strstr(uri, "cgi-bin")) {
    // 정적 콘텐츠는 CGI 인자가 없음
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    // URI가 '/'로 끝나면 기본 페이지로 home.html 설정
    if (uri[strlen(uri) - 1] == '/') { strcat(filename, "home.html"); }

    return 1;
  }
  // Dynamic content
  else {
    ptr = index(uri, '?');
    if (ptr) {
      strcpy(cgiargs, ptr + 1); // CGI 인자 추출
      *ptr = '\0'; // '?' 이후 잘라내기
    }
    else { strcpy(cgiargs, ""); }

    strcpy(filename, ".");
    strcat(filename, uri); // 파일 경로 설정
    return 0;
  }
}

// 정적 컨텐츠 처리
void serve_static(int fd, char *filename, int filesize) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  // Send response headers to client
  // 정적 컨텐츠 처리
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  // Send response body to client
  // 파일을 메모리에 매핑하고 클라이언트에 전송
  // srcfd = Open(filename, O_RDONLY, 0); // 파일 열기
  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // 파일 매핑
  // Close(srcfd); // 파일 디스크립터 닫기 (매핑되었으므로 OK)
  // Rio_writen(fd, srcp, filesize); // 파일 내용 전송
  // Munmap(srcp, filesize); // 매핑 해제
  
  // Use malloc
  srcfd = Open(filename, O_RDONLY, 0);
  srcp = (char *)malloc(sizeof(filesize));
  Rio_readn(srcfd, srcp, filesize);
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  free(srcp);
}

// get_filetype - Derive file type from filename
void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html")) { strcpy(filetype, "text/html"); }
  else if (strstr(filename, ".gif")) { strcpy(filetype, "image/gif"); }
  else if (strstr(filename, ".png")) { strcpy(filetype, "image/png"); }
  else if (strstr(filename, ".jpg")) { strcpy(filetype, "image/jpeg"); }
  else if (strstr(filename, ".mp4")) { strcpy(filetype, "video/mp4"); }
  else { strcpy(filetype, "text/plain"); }  // 기본값
}

void serve_dynamic(int fd, char *filename, char *cgiargs) {
  char buf[MAXLINE], *emptylist[] = { NULL };

  // Return first part of HTTP response
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));
  
  // 자식 프로세스를 생성하여 CGI 프로그램 실행
  if (Fork() == 0) { // 자식 프로세스
    // CGI 환경 변수 설정
    setenv("QUERY_STRING", cgiargs, 1);
    // 표준 출력을 클라이언트 소켓으로 리다이렉션
    Dup2(fd, STDOUT_FILENO);
    // CGI 프로그램 실행
    Execve(filename, emptylist, environ);
  }
  // 부모 프로세스는 자식 종료까지 대기
  Wait(NULL);
}
