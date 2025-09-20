#include "matching.h"

// Define max size of hashtable to be prime number
#define SIZE 37


// Hash table array
order* hashArray[SIZE]; 
// Keep track of hashtable's free space --- May change this when implementing strategies
int freeSpace = SIZE;
// Define a starting index for orders to use as a key if needed
int countID = 0;

// Hash function - uses orderID as the key
int hashCode(int orderID) {
   return orderID % SIZE;
}

// Search for an order by orderID
order *search_orders(int orderID) {
   // Get the hash 
   int hashIndex = hashCode(orderID);  
	
   // Move in array until an empty slot
   while(hashArray[hashIndex] != NULL) {
	
      if(hashArray[hashIndex]->orderID == orderID)
         return hashArray[hashIndex]; 
			
      // Go to next cell
      hashIndex++;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }        
	
   return NULL;        
}

// Insert a new order
void insert_order_byValues(int orderID, tradeType type, double price, double volume, orderType fill) {
   // Return an error message if a user tries to overload the hashtable -- Consider changing this
   if (freeSpace <= 0) {
      printf("Reached Maximum Number of Outgoing Orders!\n");
      return;
   }
   // Create new order
   order *newOrder = (order*) malloc(sizeof(order));
   newOrder->orderID = orderID;
   
   // Create orderData
   newOrder->orderInfo = (orderData*) malloc(sizeof(orderData));
   newOrder->orderInfo->type = type;
   newOrder->orderInfo->price = price;
   newOrder->orderInfo->volume = volume;
   newOrder->orderInfo->fill = fill;
   
   // Get the hash 
   int hashIndex = hashCode(orderID);
   
   // Move in array until an empty or deleted cell
   while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->orderID != orderID) {
      // Go to next cell
      hashIndex++;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }
	
   hashArray[hashIndex] = newOrder;
   freeSpace--;
}

// Alternative insert function that takes an order pointer
void insert_order_byPointer(order* orderPtr) {
   // Return an error message if a user tries to overload the hashtable -- Consider changing this
   if (freeSpace <= 0) {
      printf("Reached Maximum Number of Outgoing Orders!\n");
      return;
   }
   // Don't add NULL pointer to active orders
   if(orderPtr == NULL) {
      return;
   }
   
   // Get the hash 
   int hashIndex = hashCode(orderPtr->orderID);
   
   // Move in array until an empty or deleted cell
   while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->orderID != orderPtr->orderID) {
      // Go to next cell
      hashIndex++;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }
	
   hashArray[hashIndex] = orderPtr;
   freeSpace--;
}

// Delete an order
void delete_order_byPointer(order* orderPtr) {
   if(orderPtr == NULL) {
        return;
   }
   
   int orderID = orderPtr->orderID;
   
   // Get the hash 
   int hashIndex = hashCode(orderID);
   
   // Move in array until an empty slot
   while(hashArray[hashIndex] != NULL) {
	
      if(hashArray[hashIndex]->orderID == orderID) {
         order* temp = hashArray[hashIndex]; 
			
         // Reset position in hash table to NULL
         hashArray[hashIndex] = NULL;
         
         // Free the memory
         if(temp->orderInfo != NULL) {
            free(temp->orderInfo);
         }
         free(temp);
         freeSpace++;
         return;
      }
		
      // Go to next cell
      hashIndex++;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }            
}

// Delete by orderID
void delete_order_byID(int orderID) {
   // Get the hash 
   int hashIndex = hashCode(orderID);
   
   // Move in array until an empty slot
   while(hashArray[hashIndex] != NULL) {
	
      if(hashArray[hashIndex]->orderID == orderID) {
         order* temp = hashArray[hashIndex]; 
			
         // Reset position in hash table to NULL
         hashArray[hashIndex] = NULL; 

        // Free the memory
         if(temp->orderInfo != NULL) {
            free(temp->orderInfo);
         }
         free(temp);
         freeSpace++;
         return;
      }
		
      // Go to next cell
      hashIndex++;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }             
}


//! TO DELETE -- Display all orders in the hash table
void display() {
   int i = 0;
	
   for(i = 0; i < SIZE; i++) {
	
      if(hashArray[i] != NULL) {
         printf(" [ID:%d, Type:%s, Price:%.2f, Vol:%.2f, Fill:%s]", 
                hashArray[i]->orderID,
                hashArray[i]->orderInfo->type == Bid ? "Bid" : "Ask",
                hashArray[i]->orderInfo->price,
                hashArray[i]->orderInfo->volume,
                hashArray[i]->orderInfo->fill == Market ? "Market" : "Limit");
      } else {
         printf(" ~~ ");
      }
   }
	
   printf("\n");
}


// Initialize the hash table
void initHashTable() {
   // Initialize all slots to NULL
   for(int i = 0; i < SIZE; i++) {
      hashArray[i] = NULL;
   }
}


//! Clean up memory
// Free remaining orders in orderList
void freeHashTable() {
   for(int i = 0; i < SIZE; i++) {
      if(hashArray[i] != NULL) {
         if(hashArray[i]->orderInfo != NULL) {
            free(hashArray[i]->orderInfo);
         }
         free(hashArray[i]);
      }
   }
}


// Determine if a price at the best node in the order book is good enough for an order
bool price_better_or_equal(order *curr_order, double nodePrice) {
   if (curr_order->orderInfo->type == Bid) {
      if (curr_order->orderInfo->price >= nodePrice) {
         return true;
      } else {
         return false;
      }
   } else {
      if (curr_order->orderInfo->price <= nodePrice) {
         return true;
      } else {
         return false;
      } 
   }
}


// Check order book if a passed order can be completed at all - if so, do it
int valid_match(treeStruct *tree, order *curr_order, userAccount *user) {  //TODO: Maybe re-write this function again to make it more seamless, it's a bit fiddly still with the market/limit differences
   // Find best current node in desired orderbook side
   node *match_node = find_best_node(tree);
   // Check if price is good enough for current order if required
   bool priceGoodEnough = price_better_or_equal(curr_order, match_node->price);

   // No valid match
   if (match_node == NULL){
      return -1;  // No valid match
   }
   while (match_node != NULL) {  //TODO: Consider this condition more

      // Full valid match
      if (match_node->volume >= curr_order->orderInfo->volume && (curr_order->orderInfo->fill == Market || (curr_order->orderInfo->fill == Limit && priceGoodEnough))) { 

         if (curr_order->orderInfo->volume == match_node->volume) {
            // Update portfolio from trade completing
            update_portfolio(curr_order->orderInfo->type, match_node->price, curr_order->orderInfo->volume, user);
            // Remove node from order book
            delete_node(tree, match_node); // TODO: Might need to store this data for portfolio update tho...
         } else {
            // Update portfolio from trade completing
            update_portfolio(curr_order->orderInfo->type, match_node->price, curr_order->orderInfo->volume, user);
            // Update volume of node in order book
            update_node_volume(tree, match_node, -(curr_order->orderInfo->volume));
         }
         // Delete order since it will be fulfilled
         delete_order_byPointer(curr_order);
         return 1;  // Full valid match --- might not need this

      // Partial valid match
      } else {
         if ((curr_order->orderInfo->fill == Limit && priceGoodEnough) || curr_order->orderInfo->fill == Market) {
            // Modify Current order for remaining volume
            curr_order->orderInfo->volume -= match_node->volume;
            // Update portfolio to keep it up to date
            update_portfolio(curr_order->orderInfo->type, match_node->price, match_node->volume, user);
            node *temp = match_node;
            // Find next best node to try and complete order with
            match_node = find_next_best(tree, match_node);
            // If the order is a limit order and we pass the desired price we terminate
            if (curr_order->orderInfo->fill == Limit && match_node->price > curr_order->orderInfo->price) {
               return 0;  // Denote a partial order completion
            }
            // We delete the node as we use up all it's volume
            delete_node(tree, temp);
         // Order is a limit type and the current node isn't good enough at all
         } else {
            return -1;
         }
      }
   }
   // Denotes a failed match - most likely the tree is empty while completing order here
   return -1;
}


// Some global declarations
orderLine ol;
char filename[] = "GBPUSD_mt5_ticks.csv";  //TODO: Improve this

void main() {  //TODO: Pointer safety checks on malloc calls and stuff
   // Define a global user
   userAccount user = {0, 10};

   // Initialize hash table
   initHashTable();

   // Initialise file pointer - so we can leave file open
   FILE *fp = open_data_file(filename);
   // Initialise bid and ask trees
   treeStruct bidTree = {Bid, NULL, 0};
   treeStruct askTree = {Ask, NULL, 0};
    
   /* while (read_next_line(fp, &ol) > 0) {
      printf("Tick: %s %s bid=%.5f ask=%.5f vol_bid=%.2f vol_ask=%.2f\n",
         ol.date, ol.time, ol.bidPrice, ol.askPrice, ol.bidVolume, ol.askVolume);
   }  */

   // Manually inserting value into order map for now
   insert_order_byValues(1, Bid, 1.35022, 0.5, Limit); //TODO: Make it so I can search through all orders in the loop below -- then we can move onto live trading strategies or make a button or something idk


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


      // Search through orders to see if we can resolve any
      order *curr_order = search_orders(1);
      // Choose correct tree to search
      treeStruct *tree_to_choose = (curr_order->orderInfo->type == Bid) ?  &askTree: &bidTree;
      int outcome = valid_match(tree_to_choose, curr_order, &user);

      if (outcome >= 0) {
         printf("- User Balances -\n GBP: %lf\n USD: %lf\n", user.baseCurrencyBalance, user.quoteCurrencyBalance);
      }
   }
   // Clean up remaining orders
   freeHashTable();
}