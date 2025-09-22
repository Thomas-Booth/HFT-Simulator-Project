#ifndef MATCHING_H
#define MATCHING_H

// Standard Includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Including other project headers
#include "order_book.h"
#include "portfolio_tracker.h"

// New enum for another differentiator
typedef enum {Market, Limit} orderType;

// Struct for relevant order details
typedef struct {
    tradeType type;
    double price;
    double volume;
    orderType fill;
} orderData;

// Struct to hold key,value pair for an order
typedef struct {
    int orderID;
    orderData *orderInfo;
} order;

// Declare project global variables
extern userAccount user;
extern int freeSpace;
extern treeStruct bidTree;
extern treeStruct askTree;

// Function declarations
int hashCode(int orderID);
order *search_orders(int orderID);
void insert_order_byPointer(order* orderPtr);
void delete_order_byPointer(order* orderPtr);
void delete_order_byID(int orderID);
void display();
void initHashTable();
void freeHashTable();
bool price_better_or_equal(order *curr_order, double nodePrice);
int valid_match(treeStruct *tree, order *curr_order, userAccount *user);
void match_all_orders();

#endif