#include "matamazom.h"
#include "amount_set.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HALF 0.5
#define RANGE 0.001
#define INVALID_AMOUNT -1

ASElement copyInt(ASElement element) {
  int *number = 0;
  *number = *(int *) element;
  return number;
}
void freeInt(ASElement element) {
  free(element);
}

int compareInt(ASElement a, ASElement b) {
  return (int) a - (int) b;
}

int main() {
  AmountSet numbers = asCreate(copyInt, freeInt, compareInt);
  asRegister(numbers, 15);
  asChangeAmount(numbers, 15, 15);
  Matamazom matamazom = matamazomCreate();
  mtmCreateNewOrder(matamazom);

  return 0;
}
