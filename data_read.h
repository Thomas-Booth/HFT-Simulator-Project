#ifndef DATAREAD_H
#define DATAREAD_H

// Standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Struct to hold a read-in order from CSV
typedef struct {
    char date[11];
    char time[13];
    double bidPrice;
    double askPrice;
    double bidVolume;
    double askVolume;
} orderLine;

// Function declarations
FILE *open_data_file(const char *filename);
int read_next_line(FILE *fp, orderLine *orderObj);

#endif