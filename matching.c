#include "matching.h"

// Define max size of hashtable to be prime number
#define SIZE 103

// Hash table array
order* hashArray[SIZE];

// Keep track of hashtable's free space
int freeSpace = SIZE;


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


// Insert an order into the hashtable
void insert_order_byPointer(order* orderPtr) {
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
	// Insert order into table and update freeSpace
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


// Display all orders in the hash table -- FOR DEBUGGING
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
int valid_match(treeStruct *tree, order *curr_order, userAccount *user) { 
   // Find best current node in desired orderbook side
   node *match_node = find_best_node(tree);
   // Check if price is good enough for current order if required
   bool priceGoodEnough = price_better_or_equal(curr_order, match_node->price);

   // No valid match
   if (match_node == NULL){
      return -1;  // No valid match
   }
   while (match_node != NULL) {

      // Full valid match
      if (match_node->volume >= curr_order->orderInfo->volume && (curr_order->orderInfo->fill == Market || (curr_order->orderInfo->fill == Limit && priceGoodEnough))) { 

         if (curr_order->orderInfo->volume == match_node->volume) {
            // Update portfolio from trade completing
            update_portfolio(curr_order->orderInfo->type, match_node->price, curr_order->orderInfo->volume, user);
            // Remove node from order book
            delete_node(tree, match_node);
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


// Iterate over order hash table and try to resolve them
void match_all_orders() {
   // Create a buffer to hold the orders that need updating
   order *orders_to_process[SIZE];
   int order_count = 0;

   // Locate orders to update
   for (int i = 0; i < SIZE; i++) {
      if (hashArray[i] != NULL) {
         orders_to_process[order_count] = hashArray[i];
         order_count++;
      }
   }
   // Update the orders in our new buffer
   for (int i=0; i < order_count; i++) {
      order *curr_order = orders_to_process[i];
      if (search_orders(curr_order->orderID) != NULL) {
         // Choose correct tree to search
         treeStruct *tree_to_choose = (curr_order->orderInfo->type == Bid) ?  &askTree : &bidTree;
         // Try to resolve order
         int outcome = valid_match(tree_to_choose, curr_order, &user);
         /* //Display new balance if changes made -- Good for debugging
         if (outcome >= 0) {
            printf("- User Balances -\n GBP: %lf\n USD: %lf\n", user.baseCurrencyBalance, user.quoteCurrencyBalance);
         } */
      }
   }
}