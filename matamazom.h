#ifndef MATAMAZOM_H_
#define MATAMAZOM_H_

#include <stdio.h>
#include <stdbool.h>

typedef enum MatamazomResult_t {
    MATAMAZOM_SUCCESS = 0,
    MATAMAZOM_NULL_ARGUMENT,
    MATAMAZOM_OUT_OF_MEMORY,
    MATAMAZOM_INVALID_NAME,
    MATAMAZOM_INVALID_AMOUNT,
    MATAMAZOM_PRODUCT_ALREADY_EXIST,
    MATAMAZOM_PRODUCT_NOT_EXIST,
    MATAMAZOM_ORDER_NOT_EXIST,
    MATAMAZOM_INSUFFICIENT_AMOUNT,
} MatamazomResult;

/** Type for specifying what is a valid amount for a product.
 * For a MATAMAZOM_INTEGER_AMOUNT product, a valid amount is an amount which is
 * within 0.001 of an integer. For example, 8.001 or 7.999 are considered a valid amount
 * for MATAMAZOM_INTEGER_AMOUNT, but 8.0011 or 7.9989 are not.
 *
 * For a MATAMAZOM_HALF_INTEGER_AMOUNT product, a valid amount is an amount which is
 * within 0.001 of a half-integer. For example, 8.001 is valid and 8.0011 is not valid,
 * but also 8.501 is valid and 8.5011 is not valid.
 *
 * For MATAMAZOM_ANY_AMOUNT, any amount is valid. For example, this is suitable for
 * products which are measured by weight.
 */
typedef enum MatamazomAmountType_t {
    MATAMAZOM_INTEGER_AMOUNT,
    MATAMAZOM_HALF_INTEGER_AMOUNT,
    MATAMAZOM_ANY_AMOUNT,
} MatamazomAmountType;

/** Type for representing a Matamazom products */
typedef struct Matamazom_t *Matamazom;

/** Type for additional custom data of a product */
typedef void *MtmProductData;

/**
 * Type of function for copying a product's custom data.
 *
 * Such a function receives a MtmProductData, creates a new MtmProductData
 * object, and returns a pointer to the new object. If the function fails for
 * any reason, it returns NULL.
 */
typedef MtmProductData (*MtmCopyData)(MtmProductData);

/**
 * Type of function for freeing a product's custom data.
 *
 * Such a function receives a MtmProductData object and frees it. The
 * MtmProductData can be NULL, in which case the function does nothing.
 */
typedef void (*MtmFreeData)(MtmProductData);

/**
 * Type of function for calculating the price of a product.
 *
 * Such a function receives the product's custom data (a MtmProductData) and an
 * amount (a double), and returns the price (a double) of purchasing the given
 * quantity of the given product.
 *
 * For example, a function that treats MtmProductData as the price of a single
 * item, with no discounts or other special considerations:
 * @code
 * double basicGetPrice(MtmProductData basePrice, double amount) {
 *     return (*(double*)basePrice) * amount;
 * }
 * @endcode
 */
typedef double (*MtmGetProductPrice)(MtmProductData, const double amount);

/**
 * Type of function for filtering a product.
 *
 * Such a function receives the product's basic data and custom data, and
 * returns true if the product should be considered, or false if the product
 * should be ignored (e.g. "filtered-out").
 *
 * @param id            - the product's id, a non-negative integer
 * @param name          - the product's name, a non-NULL string
 * @param customData    - the generic custom data associated with the product
 * @param amount        - the amount associated with the desired filtering e.g
 *                            the existing amount of some product.
 * @return
 *     true - if the product should be considered
 *     false - if the product should be ignored
 *
 * For example, if we are interested only in products whose name starts with 'A':
 * @code
 * bool nameStartsWithA(int id, const char *name, MtmProductData customData) {
 *     return strlen(name) > 0 && name[0] == 'A';
 * }
 * @endcode
 */
typedef bool (*MtmFilterProduct)(const unsigned int id, const char *name,
                                 const double amount,
                                 MtmProductData customData);

/**
 * matamazomCreate: create an empty Matamazom products.
 *
 * @return A new Matamazom products in case of success, and NULL otherwise (e.g.
 *     in case of an allocation error)
 */
Matamazom matamazomCreate();

/**
 * matamazomDestroy: free a Matamazom products, and all its contents, from
 * memory.
 *
 * @param matamazom - the products to free from memory. A NULL value is
 *     allowed, and in that case the function does nothing.
 */
void matamazomDestroy(Matamazom matamazom);

/**
 * mtmNewProduct: add a new product to a Matamazom products.
 *
 * @param matamazom - products to add the product to. Must be non-NULL.
 * @param id - new product id. Must be non-negative, and unique.
 * @param name - name of the product, e.g. "apple". Must be non-empty.
 * @param amount - the initial amount of the product when added to the products.
 * @param amountType - defines what are valid amounts for this product.
 * @param customData - pointer to product's additional data
 * @param copyData - function pointer to be used for copying product's additional
 *      data.
 * @param freeData - function pointer to be used for free product data.
 * @param prodPrice - function pointer to be used for getting the price of some
 *      product.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if matamazom/name/customData/copyData/freeData
 *      /prodPrice are NULL.
 *     MATAMAZOM_INVALID_NAME - if name is empty, or doesn't start with a
 *         letter (a -z, A -Z) or a digit (0 -9).
 *     MATAMAZOM_INVALID_AMOUNT - if amount < 0, or is not consistent with amountType
 *         (@see MatamazomAmountType documentation above)
 *     MATAMAZOM_PRODUCT_ALREADY_EXIST - if a product with the given id already exist.
 *     MATAMAZOM_SUCCESS - if product was added successfully.
 */
MatamazomResult
mtmNewProduct(Matamazom matamazom, const unsigned int id, const char *name,
              const double amount, const MatamazomAmountType amountType,
              const MtmProductData customData, MtmCopyData copyData,
              MtmFreeData freeData, MtmGetProductPrice prodPrice);

/**
 * mtmChangeProductAmount: increase or decrease the amount of an *existing* product in a Matamazom products.
 * if 'amount' < 0 then this amount should be decreased from the matamazom products.
 * if 'amount' > 0 then this amount should be added to the matamazom products.
 * if 'amount' = 0 then nothing should be done.
 * please note:
 * If the amount to decrease is larger than the product's amount in the
 * products, then the product's amount is not changed, and a proper error-code
 * is returned.
 * If the amount is equal to the product's amount in the
 * products,then the product will remain inside the products
 * with amount of zero.
 *
 * @param matamazom - products to add the product to. Must be non-NULL.
 * @param id - existing product id. Must exist in the products.
 * @param amount - the amount of the product to increase/decrease to the products.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_PRODUCT_NOT_EXIST - if matamazom does not contain a product with
 *         the given id.
 *     MATAMAZOM_INVALID_AMOUNT - if amount is not consistent with product's amount type
 *         (@see parameter amountType in mtmNewProduct).
 *     MATAMAZOM_INSUFFICIENT_AMOUNT - if 'amount' < 0 and the amount to be decreased
 *         is bigger than product's amount in the products.
 *     MATAMAZOM_SUCCESS - if product amount was increased/decreased successfully.
 * @note Even if amount is 0 (thus the function will change nothing), still a proper
 *    error code is returned if one of the parameters is invalid, and MATAMAZOM_SUCCESS
 *    is returned if all the parameters are valid.
 */
MatamazomResult
mtmChangeProductAmount(Matamazom matamazom, const unsigned int id,
                       const double amount);

/**
 * mtmClearProduct: clear a product from a Matamazom products.
 *
 * The entire amount of the product is removed, and the product is erased
 * completely from the products, from all existing orders and from the
 * 'income' mechanism(holding the profits for each existing product).
 * For example, after clearing a product with
 * mtmClearProduct, calling mtmChangeProductAmount on that product will fail.
 *
 * @param matamazom - products to remove the product from.
 * @param id - id of product to be removed.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_PRODUCT_NOT_EXIST - if matamazom does not contain a product with
 *         the given id.
 *     MATAMAZOM_SUCCESS - if product was cleared successfully.
 */
MatamazomResult mtmClearProduct(Matamazom matamazom, const unsigned int id);

/**
 * mtmCreateNewOrder: create a new empty order in a Matamazom products, and
 * return the order's id.
 *
 * @param matamazom - a Matamazom products
 * @return
 *     Positive id of the new order, if successful.
 *     0 in case of failure.
 */
unsigned int mtmCreateNewOrder(Matamazom matamazom);

/**
 * mtmChangeProductAmountInOrder: add/increase/remove/decrease products to an existing order.
 * Only products that exist inside the matamazom can be added to an order.
 * if 'amount' < 0 then this amount should be decreased from the product in the order.
 * if 'amount' > 0 then this amount should be added to the product in the order.
 * if 'amount' = 0 then nothing should be done.
 * please note:
 *  1) If the amount to decrease('amount' < 0) is *larger or equal* then the amount of the product in the
 *     order, then the product is removed entirely from the order.
 *  2) If 'amount' > 0 and the product doesn't exist inside the order then you should add it to the order
 *     with the amount given as argument.
 *
 * @param matamazom - products containing the order and the product.
 * @param orderId - id of the order being modified.
 * @param productId - id of product to add to the order.
 * @param amount - amount of product to add to the order.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_ORDER_NOT_EXIST - if matamazom does not contain an order with
 *         the given orderId.
 *     MATAMAZOM_PRODUCT_NOT_EXIST - if matamazom does not contain a product with
 *         the given productId.
 *     MATAMAZOM_INVALID_AMOUNT - if amount is not consistent with product's amount type
 *         (@see parameter amountType in mtmNewProduct).
 *     MATAMAZOM_SUCCESS - if product was added/removed/increased/decreased to the order successfully.
 * @note Even if amount is 0 (thus the function will change nothing), still a proper
 *    error code is returned if one of the parameters is invalid, and MATAMAZOM_SUCCESS
 *    is returned if all the parameters are valid.
 */
MatamazomResult
mtmChangeProductAmountInOrder(Matamazom, const unsigned int orderId,
                              const unsigned int productId,
                              const double amount);

/**
 * mtmShipOrder: ship an order and remove it from a Matamazom products.
 *
 * All products in the order are removed from the products, and the order is
 * deleted. The amount of each product in the order is the amount of the product
 * that is removed from the products. additionally once order is shipped
 * the profit from the products shipped needs to be updated
 *
 * If the order cannot be shipped for any reason, e.g. some product's amount in
 * the order is larger than its amount in the products, then the entire
 * operation is canceled - the order remains in the products, and the
 * products contents are not modified.
 *
 * @param matamazom - products containing the order and all the products.
 * @param orderId - id of the order being shipped.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_ORDER_NOT_EXIST - if matamazom does not contain an order with
 *         the given orderId.
 *     MATAMAZOM_INSUFFICIENT_AMOUNT - if the order contains a product with an amount
 *         that is larger than its amount in matamazom.
 *     MATAMAZOM_SUCCESS - if the order was shipped successfully.
 */
MatamazomResult mtmShipOrder(Matamazom matamazom, const unsigned int orderId);

/**
 * mtmCancelOrder: cancel an order and remove it from a Matamazom products.
 *
 * The order is deleted from the products. The products and their amounts in
 * the products is not changed.
 *
 * @param matamazom - products containing the order.
 * @param orderId - id of the order being canceled.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_ORDER_NOT_EXIST - if matamazom does not contain an order with
 *         the given orderId.
 *     MATAMAZOM_SUCCESS - if the order was shipped successfully.
 */
MatamazomResult mtmCancelOrder(Matamazom matamazom, const unsigned int orderId);

/**
 * mtmPrintInventory: print a Matamazom products and its contents as
 * explained in the *.pdf
 *
 * @param matamazom - a Matamazom products to print.
 * @param output - an open, writable output stream, to which the contents are printed.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_SUCCESS - if printed successfully.
 */
MatamazomResult mtmPrintInventory(Matamazom matamazom, FILE *output);

/**
 * matamazomPrintOrder: print a summary of an order from a Matamazom products,
 * as explained in the *.pdf
 *
 * The printout includes the total price of the order.
 *
 * @param matamazom - the Matamazom products containing the order.
 * @param orderId - id of the order in matamazom.
 * @param output - an open, writable output stream, to which the order is printed.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_ORDER_NOT_EXIST - if matamazom does not contain an order with
 *         the given orderId.
 *     MATAMAZOM_SUCCESS - if printed successfully.
 */
MatamazomResult
mtmPrintOrder(Matamazom matamazom, const unsigned int orderId, FILE *output);

/**
 * mtmPrintBestSelling: print the best selling products of a Matamazom
 * products, as explained in the *.pdf.
 *
 * @param matamazom - a Matamazom products.
 * @param output - an open, writable output stream, to which the order is printed.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_SUCCESS - if printed successfully.
 */
MatamazomResult mtmPrintBestSelling(Matamazom matamazom, FILE *output);

/**
 * mtmPrintFiltered: print some products of a Matamazom products, according to
 * a custom filter, as explained in the *.pdf.
 *
 * Please note: This function filter only products that are inside the warehouses inventory.
 * I.e. this function should not care about what is happening inside the orders or the 'income'
 * mechanism.
 *
 * @param matamazom - a Matamazom products.
 * @param customFilter - a boolean function that receives a product's information and
 *     returns true if it should be printed.
 * @param output - an open, writable output stream, to which the order is printed.
 * @return
 *     MATAMAZOM_NULL_ARGUMENT - if a NULL argument is passed.
 *     MATAMAZOM_SUCCESS - if printed successfully.
 */
MatamazomResult
mtmPrintFiltered(Matamazom matamazom, MtmFilterProduct customFilter,
                 FILE *output);

#endif /* MATAMAZOM_H_ */
