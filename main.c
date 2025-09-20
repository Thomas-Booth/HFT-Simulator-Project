




// void main(int argc, char* argv[]){
//     // Run simulation

//     // Clean up pointers
// }



//! Saved code for backup
/* 
int valid_match(treeStruct *tree, order *curr_order) {
   // Find best current node in desired orderbook side
   node *match_node = find_best_node(tree);
   // No valid match
   if (match_node == NULL){
      return -1;  // No valid match
   // Full valid match
   } else if (match_node->volume >= curr_order->orderInfo->volume) {
      if (curr_order->orderInfo->volume == match_node->volume) {
         // Remove node from order book
         delete_node(tree, match_node);
      } else {
         // Update volume of node in order book
         update_node_volume(tree, curr_order->orderInfo->price, -(curr_order->orderInfo->volume));
      }
      // Update portfolio from trade completing
      update_portfolio();  //TODO: Implement
      // Delete order since it will be fulfilled
      delete_order_byPointer(curr_order);

      return 1;  // Full valid match --- might not need this
   // Partial valid match
   } else {
      if (curr_order->orderInfo->fill == Market) {
         // Move up tree and buy/sell
         while (match_node != NULL && curr_order->orderInfo->volume > 0) {
            curr_order->orderInfo
         }
      } else {
         // Create new order/Modify Current order for remaining volume
      }
        return 0;  // Partial valid match --- Might not need this
    }
}



//! Also saved code from matching.c
void main() {  //TODO: Pointer safety checks on malloc calls and stuff 
// Initialize hash table
   initHashTable();
   
   // Insert some orders
   insert_order_byValues(1, Bid, 100.50, 10.0, Limit);
   insert_order_byValues(2, Ask, 101.25, 5.0, Market);
   insert_order_byValues(42, Bid, 99.75, 15.0, Limit);
   insert_order_byValues(4, Ask, 102.00, 8.0, Limit);
   insert_order_byValues(41, Bid, 1002.00, 18.0, Limit);
   //insert_order_byValues(12, Bid, 100.00, 12.0, Market);
   insert_order_byValues(1, Bid, 1000000.76, 125.0, Market);

   
   printf("Hash Table Contents:\n");
   display();
   
   // Search for an order
   order* foundOrder = search_orders(42);
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
   foundOrder = search_orders(42);
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
} */