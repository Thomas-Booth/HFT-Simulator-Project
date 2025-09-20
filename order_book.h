#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "data_read.h"

typedef enum {Red, Black} nodeColour;
typedef enum {Bid, Ask} tradeType; 

typedef struct node{
    double price;
    double volume;
    nodeColour colour;
    struct node *left;
    struct node *right;
    struct node *parent;
} node;

typedef struct {
    tradeType type;
    node *root;
    int size;
} treeStruct;

// Function Declarations
void insert_node(treeStruct *tree, node *new_node);
void balance_tree_insert(treeStruct *tree, node *curr_node);
void trinode_right_rotation(treeStruct *tree, node *curr_node);
void trinode_left_rotation(treeStruct *tree, node *curr_node);
void delete_node(treeStruct *tree, node *delNode);
void balance_tree_delete(treeStruct *tree, node *fixup_node, node *parent, bool is_left_child);
void recursive_delete(treeStruct *tree, node *curr_node, node *best_node);
node *inorder_successor(node *delNode);
node *search_tree(treeStruct *tree, double searchPrice);
node *find_best_node(treeStruct *tree);
node *find_worst_node(treeStruct *tree);
node *find_next_best(treeStruct *tree, node *curr_node);
void update_node_volume(treeStruct *tree, node *curr_node, double volumeChange);


//!
void print_tree_visual(treeStruct *tree);
void print_tree_recursive(node *root, int depth, char *prefix);

#endif