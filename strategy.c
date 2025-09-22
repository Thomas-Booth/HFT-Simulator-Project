//* Strategies: Trend Trading (up to us to figure out what a "trend" could be), Range trading (figure out what a stable price is - ideal in forex), Swing trading?, Scalping could be very good in a HFT setting


// Includes for UDP data transfer
// I'm on windows but will hopefully get Linux working too
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

// Globab UDP socket
#ifdef _WIN32
    static SOCKET udp_socket = INVALID_SOCKET;
#else
    static int udp_socket = -1;
#endif
static struct sockaddr_in server_addr;

// Including correct header
#include "strategy.h"

// Define our basic support/resistance strategy bounds
#define SUPPORT 1.34500
#define RESISTANCE 1.35400

// Define key user balance values and how much 1 volume is worth
#define STANDARD_LOT 100000  // 1 GBP here = 100000 in reality
#define STARTING_BALANCE 10  // How much USD (in 100,000s) we begin with

//! Some global declarationsv  -- Needed in our main()
orderLine ol;
char filename[] = "GBPUSD_mt5_ticks.csv";  //TODO: Improve this
// Initialise bid and ask trees
treeStruct bidTree = {Bid, NULL, 0};
treeStruct askTree = {Ask, NULL, 0};
// Define a global user
userAccount user = {0, STARTING_BALANCE};
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


// Write data to file/ledger
void write_to_file(double best_bid) {
    fprintf(stdout, "%lf\n", (user.baseCurrencyBalance*best_bid)+user.quoteCurrencyBalance);
    fflush(stdout);
}


// Initialise UDP connection
void init_udp_graphing() {
    #ifdef _WIN32
        // initialise Winsock for Windows machine
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (result != 0) {
            printf("WSAStartup failed with error %d\n", result);
            return;
        }

        udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udp_socket == INVALID_SOCKET) {
            printf("Failed to create UDP socket: %d\n", WSAGetLastError());
            WSACleanup();
            return;
        }
    #else
        udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket < 0) {
            printf("Failed to create UDP socket\n");
        }
    #endif

    // Set up server address (for python receiver)
    server_addr.sin_family = AF_INET;
    // Can access the data through port 8888
    server_addr.sin_port = htons(8888);

    #ifdef _WIN32
        // Using localhost
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    #else
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1")
    #endif

    printf("UDP Initialised for graphing - Python script should be running on port 8888\n");
}

// Sending data to python script for graphing
void send_graph_data(double bid_price, double ask_price, double portfolioValue, int lines_processed) {  //TODO: can send more data for more graphs
    #ifdef _WIN32
        // Not initialised
        if (udp_socket == INVALID_SOCKET) {
            return;
        }
    #else
        // Not initialised
        if (udp_socket < 0) {
            return;
        }
    #endif

    // Sending CSV style data
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%d,%.6f,%.6f,%.2f\n", lines_processed, bid_price, ask_price, portfolioValue);
    
    sendto(udp_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
}


// Clean up of UDP socket
void cleanup_udp() {
    #ifdef _WIN32
        if (udp_socket != INVALID_SOCKET) {
            closesocket(udp_socket);
            udp_socket = INVALID_SOCKET;
            WSACleanup();
        }
    #else
        if (udp_socket >= 0) {
            close(udp_socket);
            upd_socket = -1;
        }
    #endif
}



void main() {  //TODO: Pointer safety checks on malloc calls and stuff
   

   // Initialise hash table
   initHashTable();

   // Initialise UDP
   init_udp_graphing();

   // Value for controlling flow of outputting data
   int lines_processed = 0;

   // Initialise file pointer - so we can leave file open
   FILE *fp = open_data_file(filename);

   while (read_next_line(fp, &ol) > 0) {
      lines_processed++;

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

      // Keep track of the last best bid for ouputting reasons
      node *curr_best_bid = find_best_node(&bidTree);
      node *curr_best_ask = find_best_node(&askTree);

      // Find relevant prices
      double best_bid_price = (curr_best_bid != NULL) ? curr_best_bid->price : 0.0f;
      double best_ask_price = (curr_best_ask != NULL) ? curr_best_ask->price : 0.0f;

      // Calculate current portfolio value based on best bid price
      double portfolioValue = (user.baseCurrencyBalance*best_bid_price)+user.quoteCurrencyBalance;

      // Write what has happened to outer file - acts as ledger and graphing
      send_graph_data(best_bid_price, best_ask_price, portfolioValue, lines_processed);
   }
   // Clean up remaining orders
   freeHashTable();
   fclose(fp);
   double end_balance = (user.baseCurrencyBalance*(find_best_node(&bidTree)->price))+user.quoteCurrencyBalance;
   //Todo: best node might be null so need to remember last or deal with it
   printf("Total Value in USD after end of file:\n Start Balance: %d\n End Balance: %lf\n P/L: %lf\n", STARTING_BALANCE*STANDARD_LOT, (end_balance)*STANDARD_LOT, (end_balance-STARTING_BALANCE)*STANDARD_LOT);
}