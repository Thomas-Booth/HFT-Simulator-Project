#include "strategy.h"

// Define a starting index for orders to use as a key if needed
int countID = 0;

order *create_order(tradeType type, double price, double volume, orderType fill) {
   // Return an error message if a user tries to overload the hashtable -- Consider changing this
   if (freeSpace <= 0) {
        printf("Reached Maximum Number of Outgoing Orders!\n");
        return NULL;
   }
   // Check if the user has enough of the correct currency to fulfill the trade
   if ((type == Bid && (price*volume) > user.quoteCurrencyBalance) || (type == Ask && volume > user.baseCurrencyBalance)) {
        return NULL;
   }
   // Create new order
   order *newOrder = (order*) malloc(sizeof(order));
   newOrder->orderID = countID++;
   
   // Create orderData
   newOrder->orderInfo = (orderData*) malloc(sizeof(orderData));
   newOrder->orderInfo->type = type;
   newOrder->orderInfo->price = price;
   newOrder->orderInfo->volume = volume;
   newOrder->orderInfo->fill = fill;

   // Insert order into order hashtable
   insert_order_byPointer(newOrder);
   return newOrder;
}


//! Basic Strategy Creation -- Basic Support/Resistance
// Check current prices, making an order if needed
void check_and_react_supportResistance(double support, double resistance) {
    node *best_bid = find_best_node(&bidTree);
    node *best_ask = find_best_node(&askTree);

    // Buy as much as possible if price falls below support
    if (best_ask->price <= support) {
        create_order(Bid, best_ask->price, best_ask->volume, Limit);
    }
    // Sell as much as possible if price rises above resistance
    if (best_bid->price >= resistance) {
        create_order(Ask, best_bid->price, best_bid->volume, Limit);
    }
}


/*In this file we can create new strategies to employ too*/
