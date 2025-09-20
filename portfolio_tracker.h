#ifndef PORTFOLIOTRACKER_H
#define PORTFOLIOTRACKER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "order_book.h"

typedef struct {
    double baseCurrencyBalance;
    double quoteCurrencyBalance;
} userAccount;


//TODO: Could create a ledger for tracking all transactions
//TODO: This would help calculating profit loss

void update_portfolio(tradeType tradeDirection, double priceUsed, double volumeChange, userAccount *user);

#endif