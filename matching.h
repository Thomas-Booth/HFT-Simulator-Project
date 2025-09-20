#ifndef MATCHING_H
#define MATCHING_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "data_read.h"
#include "order_book.h"
#include "portfolio_tracker.h"


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

#endif