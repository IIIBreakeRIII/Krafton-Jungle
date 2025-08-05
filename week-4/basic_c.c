#include <stdio.h>
#include <string.h>

int main() {
    char input[20];
    printf("명령어 입력 (star 또는 reverseStar): ");
    scanf("%s", input);

    if (strcmp(input, "star") == 0) {
        // 오름차순 출력
        for (int i = 1; i <= 5; i++) {
            for (int j = 1; j <= i; j++) {
                printf("*");
            }
            printf("\n");
        }
    } else if (strcmp(input, "reverseStar") == 0) {
        // 내림차순 출력
        for (int i = 5; i >= 1; i--) {
            for (int j = 1; j <= i; j++) {
                printf("*");
            }
            printf("\n");
        }
    } else {
        printf("잘못된 명령어입니다.\n");
    }

    return 0;
}
