#include "matamazom.h"
#include "set.h"
#include "amount_set.h"
#include <stdlib.h>

struct productInformation_t {
  MtmProductData customData;
  MatamazomAmountType amountType;
  unsigned int id;
  char *name;
  unsigned int total_income;
};

struct Matamazom_t {
  AmountSet products;
  AmountSet orders;
};

int compareProductsID(unsigned int product_id1, unsigned int product_id2) {
  return product_id1 - product_id2;
}

Matamazom matamazomCreate() {
  Matamazom new_warehouse = malloc(sizeof(*new_warehouse));
  if (new_warehouse == NULL) {
    return NULL;
  }
  new_warehouse->products=asCreate()
}