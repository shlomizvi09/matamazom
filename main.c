#include "matamazom.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define HALF 0.5
#define RANGE 0.001
#define INVALID_AMOUNT -1

static bool amountVerifications(double amount_to_change, MatamazomAmountType
type) {
    double amount=fabs(amount_to_change);
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

int main() {
    printf("amount is :%d",amountVerifications(3.0000666,
            MATAMAZOM_INTEGER_AMOUNT));


  return 0;
}
