#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "order_book.c"


typedef enum {Market, Limit} orderType;

typedef struct {
    tradeType type;
    double price;
    double volume;
    orderType fill;
} orderData;

typedef struct {
    // TODO: More specific typing? i.e. unsigned int?
    int orderID;
    orderData *orderInfo;
} order;