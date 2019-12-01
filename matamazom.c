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

int compareProductsID(unsigned int product_id1, unsigned int product_id2) {
    return product_id1 - product_id2;
}

Matamazom matamazomCreate() {

}

ProductInfo copyProduct(ProductInfo info, MtmCopyData copy_costume) {
    if (info == NULL || copy_costume == NULL) {
        return NULL;
    }
    ProductInfo tmp;
    tmp->customData = copy_costume(info);
    tmp->amountType = info->amountType;
    tmp->id = info->id;
    tmp->name = info->name;
    tmp->total_income = info->total_income;
    return tmp;
}

void freeProduct(ProductInfo info, MtmFreeData free_costume) {
    if (info == NULL || free_costume == NULL) {
        return;
    }
    free_costume(info->customData);
    free(info->name);
    free(info);
}
