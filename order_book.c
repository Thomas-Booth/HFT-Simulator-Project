#include "order_book.h"

#define MAX_TREE_SIZE 10

// Insert new nodes into the tree until it's max-size is reached
void insert_node(treeStruct *tree, node *new_node) {
    if (tree->size == 0) {
        tree->root = new_node;
        new_node->colour = Black;  // Using the rule that the root is always black... At least to start with
        return;
    }
    // Keep searching until a NULL leaf node found
    bool found = false;
    // Start at trees root node
    node *curr_node = tree->root;
    // Inserting a node will always be a red node
    new_node->colour = Red;
    while (!found) {
        if (new_node->price > curr_node->price) {
            // Logic for a new larger price
            if (curr_node->right == NULL) {
                curr_node->right = new_node;
                // TODO: Tree balancing/checking
                found = true;  // TODO: might not need this
                return;
            } else {
                curr_node = curr_node->right;
            }
        } else if (new_node->price < curr_node->price) {
            if (curr_node->left == NULL) {
                curr_node->left = new_node;
                // TODO: Balancing stuff
                found = true;  // TODO: might not need this
                return;
            } else {
                curr_node = curr_node->left;
            }
        } else {
            curr_node->volume += new_node->volume;
            return;
        }
    }
}

// Remove node from a tree and balance it
void delete_node() {

}

// Alter the fixed sized trees to retain their rules
void update_node() {

}

void main() {

}