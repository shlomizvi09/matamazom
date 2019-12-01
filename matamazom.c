#include "matamazom.h"
#include "set.h"
#include "amount_set.h"

struct productInformation_t {
  MtmProductData customData;
  MatamazomAmountType amountType;
  unsigned int id;
  char *name;
  unsigned int total_income;
};

int compareProductsID(unsigned int product_id1, unsigned int product_id2) {
  return product_id1 - product_id2;
}

Matamazom matamazomCreate(){

}

