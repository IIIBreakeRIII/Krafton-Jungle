> Tiny Web server
> Dave O'Hallaron
> Carnegie Mellon University
> 
> This is the home directory for the Tiny server, a 200-line Web
> server that we use in "15-213: Intro to Computer Systems" at Carnegie
> Mellon University.  Tiny uses the GET method to serve static content
> (text, HTML, GIF, and JPG files) out of ./ and to serve dynamic
> content by running CGI programs out of ./cgi-bin. The default 
> page is home.html (rather than index.html) so that we can view
> the contents of the directory from a browser.
> 
> Tiny is neither secure nor complete, but it gives students an
> idea of how a real Web server works. Use for instructional purposes only.
> 
> The code compiles and runs cleanly using gcc 2.95.3 
> on a Linux 2.2.20 kernel.
> 
> To install Tiny:
>    Type "tar xvf tiny.tar" in a clean directory. 
> 
> To run Tiny:
>    Run "tiny <port>" on the server machine, 
> 	e.g., "tiny 8000".
>    Point your browser at Tiny: 
> 	static content: http://<host>:8000
> 	dynamic content: http://<host>:8000/cgi-bin/adder?1&2
> 
> Files:
>   tiny.tar		Archive of everything in this directory
>   tiny.c		The Tiny server
>   Makefile		Makefile for tiny.c
>   home.html		Test HTML page
>   godzilla.gif		Image embedded in home.html
>   README		This file	
>   cgi-bin/adder.c	CGI program that adds two numbers
>   cgi-bin/Makefile	Makefile for adder.c

***

### Tiny 웹 서버 개요

- 약 250줄의 Iterative HTTP/1.0 server
- Static Contents & Dynamic Contents(CGI Program) 모두 제공 가능
    - 프로세스 제어, Unix I/O 소켓, HTTP 개념을 종합적으로 활용

> `main` 루틴  
- `open_listenfd`로 listen socket 생성
- 무한 루프 : `accept` -> `doit`으로 요청 처리 -> `close`
    - 매 연결은 순차적으로 처리되는 구조

> `doit` 함수
- 요청라인 읽고 파싱(`GET`만 지원, 그 외는 501 에러 반환)
- 헤더 읽고 무시
- URI를 해석해 정적 / 동적 여부 결정
    - 파일 존애 여부와 권한 검사 후 `serve_static` 또는 `serve_dynamic` 호출

> `clienterror` 함수
- 상태 코드 / 메시지와 간단한 HTML 에러 페이지를 작성
    `rio_writen`으로 응답라인, 헤더, 바디 전송

> `read_requesthdrs` 함수
- 헤더들을 끝 줄(`\r\n`)까지 읽고 무시
- `Tiny`는 헤더 정보를 사용하지 않음

> `parse_uri` 함수
- `cig-bin` 포함 여부로 동적 / 정적 판별
- 정적 : 현재 디렉토리 기준 경로, `/`로 끝나면 `home_html` 추가
    - 동적 : `?`뒤를 CGI인자로 분리, 나머지는 실행 파일 이름

> `serve_static` 함수
- 파일 확장자로 MIME 타입 결정
- 응답 라인 및 헤더 전송
- `mmap`으로 파일을 메모리에 매핑 후 `rio_writen`으로 클라이언트에 전송
    - `munmap`으로 메모리 해제

> `serve_dynamic` 함수
- 응답 라인과 서버 헤더 전송
- `fork`로 자식 생성 -> `setenv("QUERY_STRING")`설정 -> `dup2`로 `stdout`을 소켓에 연결 -> `execve`로 CGI 실행
    - 부모는 `wait`로 자식 종료 대기

> 보완적 논의
- 실제 웹 서버와 달리 Tiny는 간단하며 에러 처리와 보안, 견고성이 부족
    - 예: 클라이언트가 연결을 조기에 끊었을 경우, `SIGPIPE`처리가 필요

