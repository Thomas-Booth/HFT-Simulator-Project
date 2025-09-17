#include "data_read.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ROW_LENGTH 77

// Opens CSV file of order details: Date, Time, BidPrice, AskPrice, BidVol, AskVol
FILE *open_data_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        exit(EXIT_FAILURE); // or return NULL and handle gracefully
    }
    return fp;
}

// Read the next tick into the orderLine struct
int read_next_line(FILE *fp, orderLine *orderObj) {
    char row[MAX_ROW_LENGTH];
    if (!fgets(row, MAX_ROW_LENGTH, fp)){
        // End of file reached/Error
        return 0;
    }     
    if (sscanf(row, "%10[^,],%12[^,],%lf,%lf,%lf,%lf", 
        orderObj->date,
        orderObj->time,
        &orderObj->bidPrice,
        &orderObj->askPrice,
        &orderObj->bidVolume,
        &orderObj->askVolume) != 6) {
            printf("Error assigning values to orderObj");
            return -1;
        }
    return 1;
}




orderLine ol;
char filename[] = "GBPUSD_mt5_ticks.csv";  //TODO: Improve this

void main(){
    // Initialise file pointer - so we can leave file open
    FILE *fp = open_data_file(filename);
    
    while (read_next_line(fp, &ol) > 0) {
        printf("Tick: %s %s bid=%.5f ask=%.5f vol_bid=%.2f vol_ask=%.2f\n",
               ol.date, ol.time, ol.bidPrice, ol.askPrice, ol.bidVolume, ol.askVolume);
    }
}