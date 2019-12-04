#include "matamazom.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define HALF 0.5
#define RANGE 0.001
#define INVALID_AMOUNT -1

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

int main() {
    printf("amount is %f",amountVerifications(123.7484,
            MATAMAZOM_ANY_AMOUNT));
  return 0;
}
