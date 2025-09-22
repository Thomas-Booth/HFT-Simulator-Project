#ifndef STRATEGY_H
#define STRATEGY_H

// Standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// My code includes
#include "matching.h"
#include "portfolio_tracker.h"

// Declaring global variables
extern userAccount user;
extern int freeSpace;

// Function declarations
order *create_order(tradeType type, double price, double volume, orderType fill);
void check_and_react_supportResistance(double support, double resistance);

#endif