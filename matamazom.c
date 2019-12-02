#include "matamazom.h"
#include "set.h"
#include "amount_set.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct productInformation_t {
  MtmProductData customData;
  MtmCopyData copyData;
  MtmFreeData freeData;
  MtmGetProductPrice prodPrice;
  MatamazomAmountType amountType;
  unsigned int id;
  char *name;
  unsigned int total_income;
};

double amountVerifications(double amount, MatamazomAmountType type);
struct Matamazom_t {
  AmountSet products;
  AmountSet orders;
};

int compareProductsID(ASElement product_id1, ASElement product_id2) {
  return (int) (((ProductInfo) product_id1)->id
      - ((ProductInfo) product_id2)->id);
}

void freeProduct(ASElement product_info, MtmFreeData free_custom) {
  if (product_info == NULL || free_custom == NULL) {
    return;
  }
  free_custom(((ProductInfo) product_info)->customData);
  free(((ProductInfo) product_info)->name);
  free(((ProductInfo) product_info));
}

static bool isNameValid(char *name) {
  return ((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z')
      || (*name >= '0' && *name <= '9'));
}

static bool isAmountValid(const double amount,
                          const MatamazomAmountType amountType) {
  if (amount < 0) {
    return false;
  }
}

ProductInfo copyProductInfo(ASElement product_info) {
  if (product_info == NULL) {
    return NULL;
  }
  ProductInfo new_product_info = malloc(sizeof(*new_product_info));
  if (new_product_info == NULL) {
    return NULL;
  }
  new_product_info->customData =
      ((ProductInfo) product_info)->
          copyData(((ProductInfo) product_info)->customData);
  new_product_info->amountType = ((ProductInfo) product_info)->amountType;
  new_product_info->id = ((ProductInfo) product_info)->id;
  new_product_info->name =
      malloc(strlen(((ProductInfo) product_info)->name) + 1);
  if (new_product_info->name == NULL) {
    freeProduct(new_product_info, ((ProductInfo) product_info)->freeData);
    return NULL;
  }
  new_product_info->total_income = ((ProductInfo) product_info)->total_income;
  return new_product_info;
}

Matamazom matamazomCreate() {
  Matamazom new_warehouse = malloc(sizeof(*new_warehouse));
  if (new_warehouse == NULL) {
    return NULL;
  }
  new_warehouse->products = asCreate((ASElement) copyProductInfo,
                                     (ASElement) freeProduct,
                                     (ASElement) compareProductsID);
  if (new_warehouse->products == NULL) {
    free(new_warehouse);
    return NULL;
  }
}

double amountVerifications(double amount, MatamazomAmountType type) {
    if (amount < 0) {
        return -1;
    }
    if (type == MATAMAZOM_ANY_AMOUNT) {
        return amount;
    } else if (type == MATAMAZOM_INTEGER_AMOUNT) {
        if ((fabs(amount - round(amount))) <= RANGE) {
            return round(amount);
        }
        return -1;
    } else if (type == MATAMAZOM_HALF_INTEGER_AMOUNT) {
        if ((round(amount) - floor(amount)) == 0) {
            if (fabs(amount - floor(amount) <= RANGE)) {
                return floor(amount);
            } else if (fabs(amount - floor(amount) - HALF <= RANGE)) {
                return floor(amount) + HALF;
            }
            return -1;
        } else if ((round(amount) - ceil(amount)) == 0) {
            if (fabs(ceil(amount) - amount <= RANGE)) {
                return ceil(amount);
            } else if (fabs(ceil(amount - amount - HALF <= RANGE))) {
                return (ceil(amount) - HALF);
            }
            return -1;
        }
    }
    return -1;
}

void matamazomDestroy(Matamazom matamazom);

MatamazomResult mtmNewProduct(Matamazom matamazom,
                              const unsigned int id,
                              const char *name,
                              const double amount,
                              const MatamazomAmountType amountType,
                              const MtmProductData customData,
                              MtmCopyData copyData,
                              MtmFreeData freeData,
                              MtmGetProductPrice prodPrice) {
  if (matamazom == NULL || name == NULL || customData == NULL
      || copyData == NULL || freeData == NULL || prodPrice == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  if (!isNameValid(name)) {
    return MATAMAZOM_INVALID_NAME;
  }
}