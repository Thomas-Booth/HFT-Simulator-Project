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
                new_node->parent = curr_node;
                // Balance tree if issues caused
                balance_tree_insert(tree, new_node);
                found = true;  // TODO: might not need this
                tree->size += 1;
                return;
            } else {
                curr_node = curr_node->right;
            }
        } else if (new_node->price < curr_node->price) {
            if (curr_node->left == NULL) {
                curr_node->left = new_node;
                new_node->parent = curr_node;
                // Balance tree if issues caused
                balance_tree_insert(tree, new_node);
                found = true;  // TODO: might not need this
                tree->size += 1;
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


// Balance Red-Black Tree
void balance_tree_insert(treeStruct *tree, node *curr_node) {
    while (curr_node != NULL && curr_node->parent->colour == Red) {
        // Check parent node's sibling
        if (curr_node->parent == curr_node->parent->parent->left) {
            node *uncle = curr_node->parent->parent->right;
            if (uncle != NULL && uncle->colour == Red) {
                // Simple recolouring of parent and uncle
                uncle->colour = Black; // Recolour uncle
                curr_node->parent->colour = Black;  // Recolour parent
                curr_node->parent->parent->colour = Red;  // Recolour grandparent
                // Move up to grandparent node to check for new issues
                curr_node = curr_node->parent->parent;
            } else {
                //Tri-node rotation needed
                if (curr_node == curr_node->parent->right) {
                    // If node is right child, rotate left first
                    curr_node = curr_node->parent;
                    trinode_left_rotation(tree, curr_node);
                } 
                // Right rotate and recolour
                curr_node->parent->colour = Black;
                curr_node->parent->parent->colour = Red;
                trinode_right_rotate(tree, curr_node->parent->parent);
            }
        } else {
            node *uncle = curr_node->parent->parent->left;
            if (uncle->colour == Red) {
                // Simple recolouring of parent and uncle
                uncle->colour = Black;  // Recolour uncle
                curr_node->parent->colour = Black;  // Recolour parent
                curr_node->parent->parent->colour = Red;  // Recolour grandparent
                // Move up to grandparent node to check for new issues
                curr_node = curr_node->parent->parent;
            } else {
                //Tri-node rotation needed
                if (curr_node == curr_node->parent->left) {
                    curr_node = curr_node->parent;
                    trinode_right_rotation(tree, curr_node);
                }
                curr_node->parent->colour = Black;
                curr_node->parent->parent->colour = Red;
                trinode_left_rotation(tree, curr_node->parent->parent);
            }
        }
    }
    tree->root->colour = Black;
    return;
}

// Rotate nodes to balance tree
void trinode_right_rotation(treeStruct *tree, node *curr_node) {
    node *left_child = curr_node->left;

    // Set left childs new parent to be current nodes parent
    left_child->parent = curr_node->parent;
    if (curr_node->parent == NULL) {
        tree->root = left_child;
    } else if (curr_node == curr_node->parent->left) {
        curr_node->parent->left = left_child;
    } else {
        curr_node->parent->right = left_child;
    }
    // Set left childs right subtree to be left subtree of node
    curr_node->left = left_child->right;
    // If necessary change parent of subtree root
    if (left_child->right != NULL) {
        left_child->right->parent = curr_node;
    }
    // Change parent of current node to be left child
    left_child->right = curr_node;
    curr_node->parent = left_child;
}

// Rotate nodes to balance tree
void trinode_left_rotation(treeStruct *tree, node *curr_node) {
    node *right_child = curr_node->right;

    // Set right childs new parent to be current nodes parent
    right_child->parent = curr_node->parent;
    if (curr_node->parent == NULL) {
        tree->root = right_child;
    } else if (curr_node == curr_node->parent->right) {
        curr_node->parent->right = right_child;
    } else {
        curr_node->parent->left = right_child;
    }
    // Set right childs left subtree to be right subtree of node
    curr_node->right = right_child->left;
    // If necessary change parent of subtree root
    if (right_child->left != NULL) {
        right_child->left->parent = curr_node;
    }
    // Change parent of current node to be right child
    right_child->left = curr_node;
    curr_node->parent = right_child;
}

// Remove node from a tree and balance it
void delete_node() {

}

// Alter the fixed sized trees to retain their rules
void update_node() {

}

void main() {

}