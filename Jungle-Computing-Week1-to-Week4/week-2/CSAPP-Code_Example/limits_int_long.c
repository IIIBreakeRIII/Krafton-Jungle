#include <stdio.h>
#include <limits.h>

int main() {
    printf("int 형의 범위:\n");
    printf("  최소값: %d\n", INT_MIN);
    printf("  최대값: %d\n\n", INT_MAX);

    printf("long 형의 범위:\n");
    printf("  최소값: %ld\n", LONG_MIN);
    printf("  최대값: %ld\n", LONG_MAX);

    return 0;
}
