#include <stdio.h>

int main() {
  int x = -1;
  unsigned u = 2147483648; // => 2^32
  
  // u = unsigned, d = signed
  printf("x = %u = %d", x, x);

  return 0;
}
