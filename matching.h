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

// Define max size of hashtable to be prime number
#define SIZE 37

extern int freeSpace;
extern userAccount user;
extern treeStruct bidTree;
extern treeStruct askTree;

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