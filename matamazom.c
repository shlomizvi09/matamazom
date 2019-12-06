#include "matamazom.h"
#include "list.h"
#include "amount_set.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "matamazom_print.h"

#define HALF 0.5
#define RANGE 0.001

typedef struct productInformation_t {
  MtmProductData customData;
  MtmCopyData copyData;
  MtmFreeData freeData;
  MtmGetProductPrice prodPrice;
  MatamazomAmountType amountType;
  unsigned int id;
  char *name;
  double total_income;
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

static bool isAmountValid(double amount_to_change, MatamazomAmountType
type) {
  double amount = fabs(amount_to_change);
  if (type == MATAMAZOM_ANY_AMOUNT) {
    return true;
  } else if (type == MATAMAZOM_INTEGER_AMOUNT) {
    if (fabs(amount - round(amount)) <= RANGE) {
      return true;
    }
    return false;
  } else if (type == MATAMAZOM_HALF_INTEGER_AMOUNT) {
    if ((round(amount) - floor(amount)) == 0) {
      if (abs(amount - floor(amount) <= RANGE)) {
        return true;
      } else if (fabs(amount - floor(amount) - HALF) <= RANGE) {
        return true;
      }
      return false;
    } else if ((round(amount) - ceil(amount)) == 0) {
      if (abs(ceil(amount) - amount <= RANGE)) {
        return true;
      } else if (fabs(ceil(amount) - amount - HALF) <= RANGE) {
        return true;
      }
      return false;
    }
  }
  return false;
}

static ProductInfo findProductInfo(AmountSet set, unsigned int id) {
  ProductInfo iterator = (ProductInfo) asGetFirst(set);
  if (iterator == NULL) {
    return NULL;
  }
  while (iterator != NULL) {
    if (iterator->id == id) {
      return iterator;
    }
    iterator = (ProductInfo) asGetNext(set);
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

void freeProduct(ASElement element) {
  if (element == NULL) {
    return;
  }
  ProductInfo product_info = (ProductInfo) element;
  product_info->freeData(product_info->customData);
  free(product_info->name);
  free(product_info);
}

void freeOrders(ListElement element) {
  if (element == NULL) {
    return;
  }
  Order order = (Order) element;
  asDestroy(order->cart);
  free(order);
}

ASElement copyProductInfo(ASElement element) {
  if (element == NULL) {
    return NULL;
  }
  ProductInfo product_info = (ProductInfo) element;
  ProductInfo new_product_info = malloc(sizeof(*new_product_info));
  if (new_product_info == NULL) {
    return NULL;
  }
  new_product_info->customData =
      product_info->copyData(product_info->customData);
  new_product_info->amountType = product_info->amountType;
  new_product_info->id = product_info->id;
  new_product_info->name =
      malloc(strlen(product_info->name) + 1);
  if (new_product_info->name == NULL) {
    freeProduct(new_product_info);
    return NULL;
  }
  strcpy(new_product_info->name, product_info->name);
  new_product_info->total_income = product_info->total_income;
  new_product_info->copyData = product_info->copyData;
  new_product_info->prodPrice = product_info->prodPrice;
  new_product_info->freeData = product_info->freeData;

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
  if (amount < 0) {
    return MATAMAZOM_INVALID_AMOUNT;
  }
  if (!isAmountValid(amount, amountType)) {
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
  new_product->customData = new_product->copyData(customData);
  if (new_product->customData == NULL) {
    free(new_product);
    return MATAMAZOM_OUT_OF_MEMORY;
  }
  new_product->name = malloc(strlen(name) + 1);
  if (new_product->name == NULL) {
    freeProduct(new_product);
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
    freeProduct(new_product); // asRegister uses a copy of product
    return MATAMAZOM_NULL_ARGUMENT;
  }
  if (result == AS_OUT_OF_MEMORY) {
    freeProduct(new_product); // asRegister uses a copy of product
    return MATAMAZOM_OUT_OF_MEMORY;
  }
  mtmChangeProductAmount(matamazom, new_product->id, amount);
  // won't be NULL_ARGUMENT, all pointers checked before
  freeProduct(new_product); // asRegister uses a copy of product
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
  if (!isAmountValid(amount, product_info->amountType)) {
    return MATAMAZOM_INVALID_AMOUNT;
  }
  AmountSetResult
      result = asChangeAmount(matamazom->products, product_info,
                              amount);
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
  asDelete(matamazom->products, (ASElement) id_ptr);
  LIST_FOREACH(Order, element, matamazom->orders) {
    id_ptr = findProductInfo(element->cart, id);
    if (id_ptr != NULL) {
      asDelete(element->cart, (ASElement) id_ptr);
    }

  }
  return MATAMAZOM_SUCCESS;
}

unsigned int mtmCreateNewOrder(Matamazom matamazom) {
  if (matamazom == NULL) {
    return 0;
  }
  unsigned int max_id = 0;
  Order current_order = (Order) listGetFirst(matamazom->orders);
  while (current_order != NULL) {
    max_id =
        current_order->order_id > max_id ? current_order->order_id : max_id;
    current_order = (Order) listGetNext(matamazom->orders);
  }
  current_order = (Order) malloc(sizeof(*current_order));
  if (current_order == NULL) {
    return 0;
  }
  current_order->order_id = max_id + 1;
  current_order->cart =
      asCreate(copyProductInfo, freeProduct, compareProductsID);
  if (current_order->cart == NULL) {
    free(current_order);
    return 0;
  }
  ListResult
      result = listInsertLast(matamazom->orders, (ListElement) current_order);
  asDestroy(current_order->cart);
  free(current_order);
  if (result == LIST_OUT_OF_MEMORY || result == LIST_NULL_ARGUMENT) {
    return 0;
  }
  return max_id + 1;
}

MatamazomResult mtmShipOrder(Matamazom matamazom, const unsigned int orderId) {
  if (matamazom == NULL || matamazom->products == NULL
      || matamazom->orders == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  if (!isOrderExists(matamazom, orderId)) {
    return MATAMAZOM_ORDER_NOT_EXIST;
  }
  Order order = getOrder(matamazom, orderId);
  assert(order != NULL);
  ProductInfo current_product_in_order = asGetFirst(order->cart);
  double amount_in_order = 0;
  double amount_in_warehouse = 0;
  AmountSetResult result;
  while (current_product_in_order != NULL) {
    result =
        asGetAmount(order->cart, current_product_in_order,
                    &amount_in_order);
    if (result != AS_SUCCESS) {
      return MATAMAZOM_NULL_ARGUMENT;
    }
    result =
        asGetAmount(matamazom->products,
                    current_product_in_order,
                    &amount_in_warehouse);
    if (result != AS_SUCCESS) {
      return MATAMAZOM_NULL_ARGUMENT;
    }
    if (amount_in_order > amount_in_warehouse) {
      return MATAMAZOM_INSUFFICIENT_AMOUNT;
    }
    current_product_in_order = asGetNext(order->cart);
  }
  double product_price_in_order = 0;
  current_product_in_order = asGetFirst(order->cart);
  ProductInfo current_product_in_products = NULL;
  while (current_product_in_order != NULL) {
    current_product_in_products =
        findProductInfo(matamazom->products,
                        current_product_in_order->id);
    asGetAmount(order->cart, current_product_in_order, &amount_in_order);
    product_price_in_order =
        current_product_in_order->prodPrice(
            current_product_in_order->customData,
            amount_in_order);
    current_product_in_products->total_income += product_price_in_order;
    asChangeAmount(matamazom->products,
                   current_product_in_products,
                   -(amount_in_order));
    current_product_in_order = asGetNext(order->cart);
  }
  return mtmCancelOrder(matamazom, orderId);
  // should succeed because all condition were checked
}

MatamazomResult mtmCancelOrder(Matamazom matamazom,
                               const unsigned int orderId) {
  if (matamazom == NULL || matamazom->orders == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  if (!isOrderExists(matamazom, orderId)) {
    return MATAMAZOM_ORDER_NOT_EXIST;
  }
  Order order = getOrder(matamazom, orderId); /*set internal iterator to the
 * order that needs to be canceled */
  if (order == NULL) {
    return MATAMAZOM_ORDER_NOT_EXIST;
  } // item exist, pointer shouldn't be NULL
  ListResult result = listRemoveCurrent(matamazom->orders);
  if (result != LIST_SUCCESS) {
    return MATAMAZOM_PRODUCT_NOT_EXIST;
    //orders isn't NULL, checked at the beginning
    //listRemove should succeed, this is just for safety
  }
  return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmPrintInventory(Matamazom matamazom, FILE *output) {
  if (matamazom == NULL || output == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  fprintf(output, "Inventory Status:\n");
  ProductInfo product = (ProductInfo) asGetFirst(matamazom->products);
  if (product == NULL) {
    //if products is empty
    return MATAMAZOM_SUCCESS;
  }
  double amount = 0;
  while (product != NULL) {
    AmountSetResult result = asGetAmount(matamazom->products, product,
                                         &amount);
    if (result == AS_NULL_ARGUMENT) {
      return MATAMAZOM_NULL_ARGUMENT;
    }
    double product_price = product->prodPrice(product->customData, 1);
    mtmPrintProductDetails(product->name,
                           product->id,
                           amount,
                           product_price,
                           output);
    product = asGetNext(matamazom->products);
  }
  return MATAMAZOM_SUCCESS;
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
  if (asContains(matamazom->products, findProductInfo(matamazom->products,
                                                      productId)) == false) {
    return MATAMAZOM_PRODUCT_NOT_EXIST;
  }
  bool amount_check = isAmountValid(amount, getAmountType(productId,
                                                          matamazom));
  if (amount_check == false) {
    return MATAMAZOM_INVALID_AMOUNT;
  }
  if (amount == 0) {
    return MATAMAZOM_SUCCESS;
  }
  double outamount;

  Order order_ptr = getOrder(matamazom, orderId);
  ProductInfo info = findProductInfo(order_ptr->cart, productId);
  asGetAmount(order_ptr->cart, info, &outamount);
  double amount_after_change = outamount + amount;
  if (amount_after_change > 0) {
    if (asContains(order_ptr->cart, info)) {
      asChangeAmount(order_ptr->cart, info, amount);
      return MATAMAZOM_SUCCESS;
    }
    asRegister(order_ptr->cart, (ASElement) info);
    asChangeAmount(order_ptr->cart, info, amount);
    return MATAMAZOM_SUCCESS;
  } else if (amount_after_change <= 0) {
    asDelete(order_ptr->cart, info);
    return MATAMAZOM_SUCCESS;
  }
}

MatamazomResult
mtmPrintOrder(Matamazom matamazom, const unsigned int orderId, FILE *output) {
  if (matamazom == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  if (isOrderExists(matamazom, orderId) == false) {
    return MATAMAZOM_ORDER_NOT_EXIST;
  }
  Order order_ptr = getOrder(matamazom, orderId);
  double total_price = 0;
  double price_of_each = 0;
  double amount_of_each = 0;
  mtmPrintOrderHeading(orderId, output);
  AS_FOREACH(ProductInfo, iterator, order_ptr->cart) {
    asGetAmount(order_ptr->cart, iterator, &amount_of_each);
    price_of_each = iterator->prodPrice(iterator->customData,
                                        amount_of_each);
    mtmPrintProductDetails(iterator->name, iterator->id, amount_of_each,
                           price_of_each, output);
    total_price = +price_of_each;
  }
  mtmPrintOrderSummary(total_price, output);
  return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmPrintBestSelling(Matamazom matamazom, FILE *output) {
  if (matamazom == NULL || output == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  ProductInfo product = asGetFirst(matamazom->products);
  ProductInfo best_selling_product = product;
  double max_income = 0;
  if (product == NULL) {
    return MATAMAZOM_ORDER_NOT_EXIST;
  }
  while (product != NULL) {
    if (product->total_income == best_selling_product->total_income) {
      if (product->id < best_selling_product->id) {
        best_selling_product = product;
        max_income = best_selling_product->total_income;
      }
    } else if (product->total_income > best_selling_product->total_income) {
      best_selling_product = product;
      max_income = best_selling_product->total_income;
    }
    product = asGetNext(matamazom->products);
  }
  if (max_income == 0) {
    fprintf(output, "Best Selling Product:\n"
                    "none");
    return MATAMAZOM_SUCCESS;
  }
  fprintf(output, "Best Selling Product:\n");
  mtmPrintIncomeLine(best_selling_product->name,
                     best_selling_product->id,
                     max_income, output);
  return MATAMAZOM_SUCCESS;
}

MatamazomResult
mtmPrintFiltered(Matamazom matamazom, MtmFilterProduct customFilter,
                 FILE *output) {
  if (matamazom == NULL) {
    return MATAMAZOM_NULL_ARGUMENT;
  }
  double price_of_each = 0;
  double amount_of_each = 0;
  AS_FOREACH(ProductInfo, iterator, matamazom->products) {
    asGetAmount(matamazom->products, iterator, &amount_of_each);
    price_of_each = iterator->prodPrice(iterator->customData, 1);
    if (customFilter(iterator->id, iterator->name, amount_of_each,
                     iterator->customData) == true) {
      mtmPrintProductDetails(iterator->name, iterator->id,
                             amount_of_each, price_of_each, output);
    }
  }
  return MATAMAZOM_SUCCESS;
}