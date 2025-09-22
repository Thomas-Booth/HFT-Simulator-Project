#include "data_read.h"

// Define the length of a line in CSV input
#define MAX_ROW_LENGTH 80

// Opens CSV file of order details: Date, Time, BidPrice, AskPrice, BidVol, AskVol
FILE *open_data_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    // Catch issues with creating file pointer
    if (!fp) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    return fp;
}

// Read the next tick into the orderLine struct
int read_next_line(FILE *fp, orderLine *orderObj) {
    // Create buffer for reading in new line
    char row[MAX_ROW_LENGTH];
    if (!fgets(row, MAX_ROW_LENGTH, fp)){
        // End of file reached/Error
        return 0;
    }
    // If we succesfully read in a new line we can assign values to our orderObj     
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