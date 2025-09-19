#include "matching.h"

// Define max size of hashtable to be prime number
#define SIZE 37

// Hash table array and dummy item for deletions
order* hashArray[SIZE]; 
order* dummyItem;

// Hash function - uses orderID as the key
int hashCode(int orderID) {
   return orderID % SIZE;
}

// Search for an order by orderID
order *search(int orderID) {
   // Get the hash 
   int hashIndex = hashCode(orderID);  
	
   // Move in array until an empty slot
   while(hashArray[hashIndex] != NULL) {
	
      if(hashArray[hashIndex]->orderID == orderID)
         return hashArray[hashIndex]; 
			
      // Go to next cell
      ++hashIndex;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }        
	
   return NULL;        
}

// Insert a new order
void insert_order_byValues(int orderID, tradeType type, double price, double volume, orderType fill) {
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
   while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->orderID != -1) {
      // Go to next cell
      ++hashIndex;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }
	
   hashArray[hashIndex] = newOrder;
}

// Alternative insert function that takes an order pointer
void insert_order_byPointer(order* orderPtr) {
   if(orderPtr == NULL) return;
   
   // Get the hash 
   int hashIndex = hashCode(orderPtr->orderID);
   
   // Move in array until an empty or deleted cell
   while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->orderID != -1) {
      // Go to next cell
      ++hashIndex;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }
	
   hashArray[hashIndex] = orderPtr;
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
			
         // Assign a dummy item at deleted position
         hashArray[hashIndex] = dummyItem;
         
         // Free the memory
         if(temp->orderInfo != NULL) {
            free(temp->orderInfo);
         }
         free(temp);
         return;
      }
		
      // Go to next cell
      ++hashIndex;
		
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
			
         // Assign a dummy item at deleted position
         hashArray[hashIndex] = dummyItem; 

        // Free the memory
         if(temp->orderInfo != NULL) {
            free(temp->orderInfo);
         }
         free(temp);

         return;
      }
		
      // Go to next cell
      ++hashIndex;
		
      // Wrap around the table
      hashIndex %= SIZE;
   }             
}

//! TO DELETE -- Display all orders in the hash table
void display() {
   int i = 0;
	
   for(i = 0; i < SIZE; i++) {
	
      if(hashArray[i] != NULL && hashArray[i] != dummyItem) {
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
   
   // Create dummy item for deleted positions
   dummyItem = (order*) malloc(sizeof(order));
   dummyItem->orderID = -1;  
   dummyItem->orderInfo = NULL;
}

//! Clean up memory
// Free remaining orders in orderList
void freeHashTable() {
   for(int i = 0; i < SIZE; i++) {
      if(hashArray[i] != NULL && hashArray[i] != dummyItem) {
         if(hashArray[i]->orderInfo != NULL) {
            free(hashArray[i]->orderInfo);
         }
         free(hashArray[i]);
      }
   }
   
   if(dummyItem != NULL) {
      free(dummyItem);
   }
}





/*
Could have a system where:
return 1, complete valid match, order can succeed
return 0, can partially match order, price level correct but not enough volume
return -1, invalid match, create new order?
*/
int valid_match(treeStruct *tree, order *curr_order) {
   node *match_node = NULL;
   match_node = search_tree(tree, curr_order->orderInfo->price);
   if (match_node == NULL){
      return -1;  // No valid match
   } else if (match_node->volume >= curr_order->orderInfo->volume) {
      // Remove node from order book
      delete_node(tree, match_node);
      // Update portfolio from trade completing
      
      // Delete order since it will be fulfilled
      delete_order_byPointer(curr_order);

      return 1;  // Full valid match --- might not need this
   } else {
      if (curr_order->orderInfo->fill == Market) {
         // Move up tree and buy/sell
      } else {
         // Create new order/Modify Current order for remaining volume
      }
        return 0;  // Partial valid match --- Might not need this
    }
}


void main() {  //TODO: Pointer safety checks on malloc calls and stuff
// Initialize hash table
   initHashTable();
   
   // Insert some orders
   insert_order_byValues(1, Bid, 100.50, 10.0, Limit);
   insert_order_byValues(2, Ask, 101.25, 5.0, Market);
   insert_order_byValues(42, Bid, 99.75, 15.0, Limit);
   insert_order_byValues(4, Ask, 102.00, 8.0, Limit);
   insert_order_byValues(41, Bid, 1002.00, 18.0, Limit);
   insert_order_byValues(12, Bid, 100.00, 12.0, Market);
   
   printf("Hash Table Contents:\n");
   display();
   
   // Search for an order
   order* foundOrder = search(42);
   if(foundOrder != NULL) {
      printf("\nOrder found: ID=%d, Type=%s, Price=%.2f\n", 
             foundOrder->orderID,
             foundOrder->orderInfo->type == Bid ? "Bid" : "Ask",
             foundOrder->orderInfo->price);
   } else {
      printf("\nOrder not found\n");
   }
   
   // Delete the order (memory automatically freed)
   delete_order_byID(42);
   printf("Order 42 deleted and memory freed\n");
   
   // Try to search again
   foundOrder = search(42);
   if(foundOrder != NULL) {
      printf("Order still found: %d\n", foundOrder->orderID);
   } else {
      printf("Order 42 not found (successfully deleted)\n");
   }
   
   printf("\nHash Table after deletion:\n");
   display();
   
   // Clean up remaining orders
   freeHashTable();
   
   return;
}