#include "matamazom.h"
#include "list.h"
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

typedef struct order_t {
  AmountSet cart;
  unsigned int order_id;
} *Order;

struct Matamazom_t {
  AmountSet products;
  List orders;
};

static bool isOrderExists(Matamazom matamazom, const unsigned int orderId) {
    if (matamazom == NULL) {
        return false;
    }
    LIST_FOREACH(Order, iteraor, matamazom->orders) {
        if (iteraor->order_id == orderId) {
            return true;
        }
    }
    return false;
}

static Order getOrder(Matamazom matamazom, const unsigned int orderId) {
    if (matamazom == NULL) {
        return NULL;
    }
    LIST_FOREACH(Order, iteraor, matamazom->orders) {
        if (iteraor->order_id == orderId) {
            return iteraor;
        }
    }
    return NULL;
}

static double amountVerifications(double amount, MatamazomAmountType type) {
    if (amount < 0) {
        return INVALID_AMOUNT;
    }
    if (type == MATAMAZOM_ANY_AMOUNT) {
        return amount;
    } else if (type == MATAMAZOM_INTEGER_AMOUNT) {
        if (fabs(amount - round(amount)) <= RANGE) {
            return round(amount);
        }
        return INVALID_AMOUNT;
    } else if (type == MATAMAZOM_HALF_INTEGER_AMOUNT) {
        if ((round(amount) - floor(amount)) == 0) {
            if (abs(amount - floor(amount) <= RANGE)) {
                return floor(amount);
            } else if (fabs(amount - floor(amount) - HALF) <= RANGE) {
                return floor(amount) + HALF;
            }
            return INVALID_AMOUNT;
        } else if ((round(amount) - ceil(amount)) == 0) {
            if (abs(ceil(amount) - amount <= RANGE)) {
                return ceil(amount);
            } else if (fabs(ceil(amount) - amount - HALF) <= RANGE) {
                return (ceil(amount) - HALF);
            }
            return INVALID_AMOUNT;
        }
    }
    return INVALID_AMOUNT;
}

static ProductInfo findProductInfo(AmountSet set, unsigned int id) {
    ASElement iterator = asGetFirst(set);
    if (iterator == NULL) {
        return NULL;
    }
    while (((ProductInfo) iterator)->id != id) {
        iterator = asGetNext(set);
        if (iterator == NULL) {
            return NULL;
        }
    }
    asGetFirst(set); //product's iterator back to start
    return iterator;
}

static MatamazomAmountType getAmountType(const unsigned int productId, Matamazom
matamazom) {
    ProductInfo info = findProductInfo(matamazom->products, productId);
    return info->amountType;
}

static bool isNameValid(const char *name) {
  return ((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z')
      || (*name >= '0' && *name <= '9'));
}

int compareProductsID(ASElement product_id1, ASElement product_id2) {
  return (int) (((ProductInfo) product_id1)->id
      - ((ProductInfo) product_id2)->id);
}
int compareOrdersID(Order order_1, Order order_2) {
  return ((int) (order_1->order_id) - (int) (order_2->order_id));
}

void freeProduct(ASElement product_info) {
  if (product_info == NULL) {
    return;
  }
  ((ProductInfo) product_info)->freeData(((ProductInfo) product_info)->customData);
  free(((ProductInfo) product_info)->name);
  free(((ProductInfo) product_info));
}
void freeOrders(ListElement element) {
  if (element == NULL) {
    return;
  }
  Order order = (Order) element;
  asDestroy(order->cart);
  free(order);
}

ASElement copyProductInfo(ASElement product_info) {
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
    freeProduct(new_product_info);
    return NULL;
  }
  new_product_info->total_income = ((ProductInfo) product_info)->total_income;
  return new_product_info;
}
ListElement copyOrder(ListElement element) {
  if (element == NULL) {
    return NULL;
  }
  Order order = (Order) element;
  Order new_order = malloc(sizeof(*new_order));
  if (new_order == NULL) {
    return NULL;
  }
  new_order->cart = asCopy(order->cart);
  if (new_order->cart == NULL) {
    free(new_order);
    return NULL;
  }
  new_order->order_id = order->order_id;
  return new_order;
}

Matamazom matamazomCreate() {
  Matamazom new_warehouse = malloc(sizeof(*new_warehouse));
  if (new_warehouse == NULL) {
    return NULL;
  }
  new_warehouse->products =
      asCreate(copyProductInfo, freeProduct, compareProductsID);
  if (new_warehouse->products == NULL) {
    free(new_warehouse);
    return NULL;
  }

  new_warehouse->orders = listCreate(copyOrder, freeOrders);
  if (new_warehouse->orders == NULL) {
    asDestroy(new_warehouse->products);
    free(new_warehouse);
    return NULL;
  }
  return new_warehouse;
}

void matamazomDestroy(Matamazom matamazom) {
  if (matamazom == NULL) {
    return;
  }
  if (matamazom->products != NULL) {
    asDestroy(matamazom->products);
  }
  if (matamazom->orders != NULL) {
    listDestroy(matamazom->orders);
  }
  free(matamazom);
}

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
  AmountSetResult result = asRegister(matamazom->products, new_product);
  if (result == AS_NULL_ARGUMENT) {
    new_product->freeData(new_product->customData);
    free(new_product->name);
    free(new_product);
    return MATAMAZOM_NULL_ARGUMENT;
  }
  if (result == AS_OUT_OF_MEMORY) {
    new_product->freeData(new_product->customData);
    free(new_product->name);
    free(new_product);
    return MATAMAZOM_OUT_OF_MEMORY;
  }
  MatamazomResult change_amount_result =
      mtmChangeProductAmount(matamazom, new_product->id, temp_amount);
  if (change_amount_result == MATAMAZOM_NULL_ARGUMENT) {
    new_product->freeData(new_product->customData);
    free(new_product->name);
    free(new_product);
    return MATAMAZOM_NULL_ARGUMENT;
  }
  return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmChangeProductAmount(Matamazom matamazom,
                                       const unsigned int id,
                                       const double amount) {
    if (matamazom == NULL) {
        return MATAMAZOM_NULL_ARGUMENT;
    }
    ProductInfo product_info = findProductInfo(matamazom->products, id);
    if (product_info == NULL) {
        return MATAMAZOM_PRODUCT_NOT_EXIST;
    }
    double fixed_amount = amountVerifications(amount, product_info->amountType);
    if (fixed_amount == INVALID_AMOUNT) {
        return MATAMAZOM_INVALID_AMOUNT;
    }
    AmountSetResult
            result = asChangeAmount(matamazom->products, product_info,
                                    fixed_amount);
    if (result == AS_NULL_ARGUMENT) {
        return MATAMAZOM_NULL_ARGUMENT;
    }
    if (result == AS_ITEM_DOES_NOT_EXIST) {
        return MATAMAZOM_PRODUCT_NOT_EXIST;
    }
    if (result == AS_INSUFFICIENT_AMOUNT) {
        return MATAMAZOM_INSUFFICIENT_AMOUNT;
    }
    assert(result == AS_SUCCESS);
    return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmClearProduct(Matamazom matamazom, const unsigned int id) {
  if (matamazom == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  ProductInfo id_ptr = findProductInfo(matamazom->products, id);
  if (id_ptr == NULL) {
    return MATAMAZOM_PRODUCT_NOT_EXIST;
  }
  freeProduct((ASElement) id_ptr);
  asDelete(matamazom->products, (ASElement) id_ptr);
  LIST_FOREACH(Order, element, matamazom->orders) {
    id_ptr = findProductInfo(element->cart, id);
    if (id_ptr != NULL) {
      freeProduct((ASElement) id_ptr);
      assert(asDelete(element->cart, (ASElement) id_ptr) !=
          AS_NULL_ARGUMENT);
    }

    }
    return MATAMAZOM_SUCCESS;
}

unsigned int mtmCreateNewOrder(Matamazom matamazom) {
  if (matamazom == NULL) {
    return 0;
  }
  unsigned int order_id = 0;
  Order current_order = (Order) listGetFirst(matamazom->orders);
  if (current_order == NULL) {
    order_id = 1; // if list is empty
  } else {
    Order next_order = (Order) listGetNext(matamazom->orders);
    while (next_order != NULL) {
      current_order = next_order;
      next_order = (Order) listGetNext(matamazom->orders);
    }
    order_id = current_order->order_id + 1;
  }
  current_order = (Order) malloc(sizeof(*current_order));
  if (current_order == NULL) {
    return 0;
  }
  current_order->order_id = order_id;
  current_order->cart =
      asCreate(copyProductInfo, freeProduct, compareProductsID);
  if (current_order->cart == NULL) {
    free(current_order);
    return 0;
  }
  return order_id;
}

MatamazomResult mtmShipOrder(Matamazom matamazom, const unsigned int orderId) {

}

MatamazomResult mtmCancelOrder(Matamazom matamazom,
                               const unsigned int orderId) {
  if (matamazom == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  if (!isOrderExists(matamazom, orderId)) {
    return MATAMAZOM_ORDER_NOT_EXIST;
  }

}

MatamazomResult
mtmChangeProductAmountInOrder(Matamazom matamazom, const unsigned int orderId,
                              const unsigned int productId,
                              const double amount) {
    if (matamazom == NULL) {
        return MATAMAZOM_NULL_ARGUMENT;
    }
    if (isOrderExists(matamazom, orderId) == false) {
        return MATAMAZOM_ORDER_NOT_EXIST;
    }
    double amount_check = amountVerifications(amount, getAmountType(productId,
                                                                    matamazom));
    if (amount_check == INVALID_AMOUNT) {
        return MATAMAZOM_INVALID_AMOUNT;
    }

    double amount_after_change = as

}

