#include "../csapp.h"

int main(void)
{
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  // QUERY_STRING 환경 변수에서 두 개의 인자 추출
  if ((buf = getenv("QUERY_STRING")) != NULL) {
    // '&'를 기준으로 두 인자 분리 (e.g., arg1=3 & arg2=5)
    p = strchr(buf, '&');
    *p = '\0'; // '&'를 널 문자로 대체하여 첫 번째 인자 종료
    strcpy(arg1, buf);       // arg1="arg1=3"
    strcpy(arg2, p + 1);     // arg2="arg2=5"

    // '=' 뒤의 문자열을 숫자로 변환
    n1 = atoi(strchr(arg1, '=') + 1); // n1 = 3
    n2 = atoi(strchr(arg2, '=') + 1); // n2 = 5
  }

  /* 응답 본문 생성 */
  sprintf(content, "QUERY_STRING=%s\r\n<p>", buf); // 원본 QUERY_STRING 출력
  sprintf(content + strlen(content), "Welcome to add.com: ");
  sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>");
  sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>", n1, n2, n1 + n2);
  sprintf(content + strlen(content), "Thanks for visiting!\r\n");

  /* HTTP 응답 헤더 및 본문 출력 */
  printf("Content-type: text/html\r\n"); // MIME 타입 설정
  printf("Content-length: %d\r\n", (int)strlen(content)); // 본문 길이
  printf("\r\n"); // 헤더 끝 표시 (\r\n\r\n)
  printf("%s", content); // 본문 출력
  fflush(stdout); // 출력 버퍼 비우기 (클라이언트로 전송 보장)

  exit(0); // 프로그램 종료
}
