#include <stdio.h>
#include <stdlib.h>

int main() {
  float f = 3.14f;
  double d = 3.14;

  printf("Single Precision Floating Point Number : f = %.20lf\n", f);
  printf("Double Precision Floating Point Number : d = %.20lf\n", d);

  float myVarFloat = 789.123456f;
  double myVarDouble = 789.123456f;

  printf("Float Value : myVarFloat is %f\n", myVarFloat);
  printf("Double Value : myVarDouble is %lf\n", myVarDouble);

  return 0;
}
