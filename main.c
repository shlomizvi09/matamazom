//#include "matamazom.h"
#include <math.h>
#include <stdio.h>
#include "matamazom.h"

#define HALF 0.5
#define RANGE 0.001

double amountVerifications(double amount, MatamazomAmountType type);

int main() {
    double x = amountVerifications(0.00006, MATAMAZOM_HALF_INTEGER_AMOUNT);
    printf("x is %f\n", x);
    printf("ceil of x is %f\n", ceil(1.51));
    double y = 1.51 - 1.5;
    printf("%f", y);
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