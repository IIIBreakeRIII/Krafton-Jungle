#include <stdio.h>

// 1. 플래그 정의: 각 스타일에 1비트씩 할당
#define STYLE_BOLD        (1 << 0)  // 0001: 굵게
#define STYLE_ITALIC      (1 << 1)  // 0010: 기울임
#define STYLE_UNDERLINE   (1 << 2)  // 0100: 밑줄
#define STYLE_STRIKETHRU  (1 << 3)  // 1000: 취소선

// 플래그 상태를 출력하는 헬퍼 함수
void print_styles(unsigned int flags) {
    printf("Styles: ");
    if (flags & STYLE_BOLD)       printf("Bold ");
    if (flags & STYLE_ITALIC)     printf("Italic ");
    if (flags & STYLE_UNDERLINE)  printf("Underline ");
    if (flags & STYLE_STRIKETHRU) printf("Strikethrough ");
    if (flags == 0)               printf("None");
    printf("\n");
}

int main() {
    unsigned int style = 0;  // 초기에는 아무 스타일도 없음 (0000)

    // 2. 스타일 설정 (set)
    style |= STYLE_BOLD;       // 굵게 추가 → 0001
    style |= STYLE_UNDERLINE;  // 밑줄 추가 → 0101

    print_styles(style);
    // 출력: Styles: Bold Underline

    // 3. 스타일 토글 (toggle) — 기울임 추가
    style ^= STYLE_ITALIC;     // 0010을 XOR → 0111
    print_styles(style);
    // 출력: Styles: Bold Italic Underline

    // 4. 스타일 제거 (clear) — 밑줄만 제거
    style &= ~STYLE_UNDERLINE; // 0100 비트를 0으로 → 0011
    print_styles(style);
    // 출력: Styles: Bold Italic

    // 5. 모든 스타일 초기화
    style = 0;  // 0000
    print_styles(style);
    // 출력: Styles: None

    return 0;
}
