// Including project headers
#include "data_read.h"
#include "order_book.h"
#include "matching.h"
#include "strategy.h"
#include "portfolio_tracker.h"

// Includes for UDP data transfer
// I'm on windows but will hopefully get Linux working too
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
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
void send_graph_data(double bid_price, double ask_price, double portfolioValue, int lines_processed) {
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


//! Global Defines
// Define key user balance values and how much 1 volume is worth
#define STANDARD_LOT 100000  // 1 GBP here = 100000 in reality
#define STARTING_BALANCE 10  // How much USD (in 100,000s) we begin with

// Define our basic support/resistance strategy bounds -- Not necessary if different strategy used
#define SUPPORT 1.28000
#define RESISTANCE 1.35000


//! Some global declarations/definitions
// Create an orderline struct to hold read-in data
orderLine ol;

// Define our input file -- Must be of format: Date, Time, bidPrice, askPrice, bidVolume, askVolume
// CURRENTLY SET TO THE YEAR LONG TICK VERSION - CAN BE CHANGED TO SHORTER FILE
char filename[] = "GBPUSD_PAST_YEAR_ticks.csv";

// Initialise bid and ask trees
treeStruct bidTree = {Bid, NULL, 0};
treeStruct askTree = {Ask, NULL, 0};

// Define a global user and their initial quoteCurrencyBalance
userAccount user = {0, STARTING_BALANCE};


// Main function call
int main() {
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

      // Creates node in the bid tree
      node *bid_node = malloc(sizeof(node));
      if (!bid_node) {
         printf("Error Allocating Memory!\n");
         exit(-1);
      }
      *bid_node = (node){ol.bidPrice, ol.bidVolume, Red, NULL, NULL, NULL};

      // Creates node in the ask tree
      node *ask_node = malloc(sizeof(node));
      if (!ask_node) {
         printf("Error Allocating Memory!\n");
         exit(-1);
      }
      *ask_node = (node){ol.askPrice, ol.askVolume, Red, NULL, NULL, NULL};

      // Inserts these new nodes
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

   // Calculate final Portfolio Value and print
   double end_balance = (user.baseCurrencyBalance*(find_best_node(&bidTree)->price))+user.quoteCurrencyBalance;
   printf("Total Value in USD after end of file:\n Start Balance: %d\n End Balance: %lf\n P/L: %lf\n", STARTING_BALANCE*STANDARD_LOT, (end_balance)*STANDARD_LOT, (end_balance-STARTING_BALANCE)*STANDARD_LOT);
   return 0;
}