#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {Red, Black} nodeColour;
typedef enum {Bid, Ask} tradeType; 

typedef struct {
    double price;
    double volume;
    nodeColour colour;
    node *left = NULL;
    node *right = NULL;
} node;

typedef struct {
    node *root = NULL;
    int size = 0;
} treeStruct;
