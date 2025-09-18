#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char date[11];
    char time[13];
    double bidPrice;
    double askPrice;
    double bidVolume;
    double askVolume;
} orderLine;