#ifndef PORTFOLIOTRACKER_H
#define PORTFOLIOTRACKER_H

// Standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Inlcudes from other project headers
#include "order_book.h"

// Struct to hold user balances
typedef struct {
    double baseCurrencyBalance;
    double quoteCurrencyBalance;
} userAccount;

// Function declarations
void update_portfolio(tradeType tradeDirection, double priceUsed, double volumeChange, userAccount *user);

#endif