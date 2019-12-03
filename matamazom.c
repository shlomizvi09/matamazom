#include "matamazom.h"
#include "set.h"
#include "amount_set.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define HALF 0.5
#define RANGE 0.001
#define INVALID_AMOUNT -1

typedef struct productInformation_t {
  MtmProductData customData;
  MtmCopyData copyData;
  MtmFreeData freeData;
  MtmGetProductPrice prodPrice;
  MatamazomAmountType amountType;
  unsigned int id;
  char *name;
  unsigned int total_income;
} *ProductInfo;

struct Matamazom_t {
  AmountSet products;
  AmountSet orders;
};

static double amountVerifications(double amount, MatamazomAmountType type) {
  if (amount < 0) {
    return INVALID_AMOUNT;
  }
  if (type == MATAMAZOM_ANY_AMOUNT) {
    return amount;
  } else if (type == MATAMAZOM_INTEGER_AMOUNT) {
    if ((fabs(amount - round(amount))) <= RANGE) {
      return round(amount);
    }
    return INVALID_AMOUNT;
  } else if (type == MATAMAZOM_HALF_INTEGER_AMOUNT) {
    if ((round(amount) - floor(amount)) == 0) {
      if (fabs(amount - floor(amount) <= RANGE)) {
        return floor(amount);
      } else if (fabs(amount - floor(amount) - HALF <= RANGE)) {
        return floor(amount) + HALF;
      }
      return INVALID_AMOUNT;
    } else if ((round(amount) - ceil(amount)) == 0) {
      if (fabs(ceil(amount) - amount <= RANGE)) {
        return ceil(amount);
      } else if (fabs(ceil(amount - amount - HALF <= RANGE))) {
        return (ceil(amount) - HALF);
      }
      return INVALID_AMOUNT;
    }
  }
  return INVALID_AMOUNT;
}

static ProductInfo findProductInfo(Matamazom matamazom, unsigned int id) {
  ASElement iterator = asGetFirst(matamazom->products);
  if (iterator == NULL) {
    return NULL;
  }
  while (((ProductInfo) iterator)->id != id) {
    iterator = asGetNext(matamazom->products);
    if (iterator == NULL) {
      return NULL;
    }
  }
  asGetFirst(matamazom->products); //product's iterator back to start
  return iterator;
}

static bool isNameValid(const char *name) {
  return ((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z')
      || (*name >= '0' && *name <= '9'));
}

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
  double temp_amount = amountVerifications(amount, amountType);
  if (temp_amount == INVALID_AMOUNT) {
    return MATAMAZOM_INVALID_AMOUNT;
  }
  ProductInfo new_product = malloc(sizeof(*new_product));
  if (new_product == NULL) {
    return MATAMAZOM_OUT_OF_MEMORY;
  }
  new_product->id = id;
  new_product->copyData = copyData;
  new_product->freeData = freeData;
  new_product->prodPrice = prodPrice;
  new_product->amountType = amountType;
  new_product->customData = copyData(customData);
  if (new_product->customData == NULL) {
    free(new_product);
    return MATAMAZOM_OUT_OF_MEMORY;
  }
  new_product->name = malloc(strlen(name) + 1);
  if (new_product->name == NULL) {
    new_product->freeData(new_product->customData);
    free(new_product);
    return MATAMAZOM_OUT_OF_MEMORY;
  }
  new_product->name = strcpy(new_product->name, name);

  if (asContains(matamazom->products, new_product)) {
    new_product->freeData(new_product->customData);
    free(new_product->name);
    free(new_product);
    return MATAMAZOM_PRODUCT_ALREADY_EXIST;
  }
  asRegister(matamazom->products, new_product);
  // TODO : need to check return value for asRegister ?
  return mtmChangeProductAmount(matamazom, new_product->id, temp_amount);
}

MatamazomResult mtmChangeProductAmount(Matamazom matamazom,
                                       const unsigned int id,
                                       const double amount) {
  if (matamazom == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  ProductInfo product_info = findProductInfo(matamazom, id);
  if (product_info == NULL) {
    return MATAMAZOM_PRODUCT_NOT_EXIST;
  }
  double fixed_amount = amountVerifications(amount, product_info->amountType);
  if (fixed_amount == INVALID_AMOUNT) {
    return MATAMAZOM_INVALID_AMOUNT;
  }
  AmountSetResult
      result = asChangeAmount(matamazom->products, product_info, amount);
  if (result == AS_NULL_ARGUMENT) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  if (result == AS_ITEM_DOES_NOT_EXIST) {
    return MATAMAZOM_PRODUCT_NOT_EXIST;
  }
  if (result == AS_INSUFFICIENT_AMOUNT) {
    MATAMAZOM_INSUFFICIENT_AMOUNT;
  }
  assert(result == AS_SUCCESS);
  return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmClearProduct(Matamazom matamazom, const unsigned int id) {
  if (matamazom == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  ProductInfo id_ptr = findProductInfo(matamazom, id);
  if (id_ptr == NULL) {
    return MATAMAZOM_PRODUCT_NOT_EXIST;
  }
  freeProduct(id_ptr, id_ptr->freeData);

}