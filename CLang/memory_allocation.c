#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int i, *arr;

    // 1. malloc: 초기화되지 않은 5개의 int 할당
    arr = (int *)malloc(5 * sizeof(int));
    if (!arr) {
        perror("malloc 실패");
        return 1;
    }
    for (i = 0; i < 5; i++) {
        arr[i] = i + 1; // 값 직접 초기화
    }
    printf("malloc 사용 결과: ");
    for (i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // 2. calloc: 5개의 int를 0으로 초기화하여 할당
    int *arr2 = (int *)calloc(5, sizeof(int));
    if (!arr2) {
        perror("calloc 실패");
        free(arr);
        return 1;
    }
    printf("calloc 사용 결과: ");
    for (i = 0; i < 5; i++) {
        printf("%d ", arr2[i]); // 모두 0
    }
    printf("\n");

    // 3. realloc: arr 크기를 10개의 int로 확장
    int *tmp = (int *)realloc(arr, 10 * sizeof(int));
    if (!tmp) {
        perror("realloc 실패");
        free(arr);
        free(arr2);
        return 1;
    }
    arr = tmp;
    for (i = 5; i < 10; i++) {
        arr[i] = (i + 1) * 10; // 새로 확장된 공간 초기화
    }
    printf("realloc 사용 결과: ");
    for (i = 0; i < 10; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // 메모리 해제
    free(arr);
    free(arr2);

    return 0;
}
