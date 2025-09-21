//* Strategies: Trend Trading (up to us to figure out what a "trend" could be), Range trading (figure out what a stable price is - ideal in forex), Swing trading?, Scalping could be very good in a HFT setting


#include "strategy.h"

#define SUPPORT 1.35200
#define RESISTANCE 1.35400


//! Some global declarationsv  -- Needed in our main()
orderLine ol;
char filename[] = "GBPUSD_mt5_ticks.csv";  //TODO: Improve this
// Initialise bid and ask trees
treeStruct bidTree = {Bid, NULL, 0};
treeStruct askTree = {Ask, NULL, 0};
// Define a global user
userAccount user = {0, 10};
// Keep track of hashtable's free space --- May change this when implementing strategies
int freeSpace = SIZE;



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
        //printf("Cannot fulfill order - You do not have the necessary funds\n");
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
}


//! Basic Strategy Creation -- Basic Support/Resistance
// Check current prices, making an order if needed
void check_and_react_supportResistance(double support, double resistance) {
    node *best_bid = find_best_node(&bidTree);
    node *best_ask = find_best_node(&askTree);

    if (best_ask->price <= support) {
        create_order(Bid, best_ask->price, best_ask->volume, Market);
    }
    if (best_bid->price >= resistance) {
        create_order(Ask, best_bid->price, best_bid->volume, Market);
    }
}


void main() {  //TODO: Pointer safety checks on malloc calls and stuff
   

   // Initialize hash table
   initHashTable();

   // Initialise file pointer - so we can leave file open
   FILE *fp = open_data_file(filename);

   while (read_next_line(fp, &ol) > 0) {
      node *bid_node = malloc(sizeof(node));
      if (!bid_node) {
         printf("Error Allocating Memory!\n");
         exit(-1);
      }
      *bid_node = (node){ol.bidPrice, ol.bidVolume, Red, NULL, NULL, NULL};

      node *ask_node = malloc(sizeof(node));
      if (!ask_node) {
         printf("Error Allocating Memory!\n");
         exit(-1);
      }
      *ask_node = (node){ol.askPrice, ol.askVolume, Red, NULL, NULL, NULL};

      insert_node(&bidTree, bid_node);
      insert_node(&askTree, ask_node);

      // Create new orders based on strategy -- Support/Resistance
      check_and_react_supportResistance(SUPPORT, RESISTANCE);

      // Try to complete orders with updated order book
      match_all_orders();
   }
   // Clean up remaining orders
   freeHashTable();
   printf("Total Value in USD after end of file: %lf\n", (user.baseCurrencyBalance*(find_best_node(&bidTree))->price)+user.quoteCurrencyBalance);
}