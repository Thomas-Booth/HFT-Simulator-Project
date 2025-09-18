#include "order_book.h"

#define MAX_TREE_SIZE 10

// Insert new nodes into the tree until it's max-size is reached
void insert_node(treeStruct *tree, node *new_node) {
    if (tree->size == 0) {
        tree->root = new_node;
        new_node->colour = Black;  // Using the rule that the root is always black... At least to start with
        return;
    }
    // Start at trees root node
    node *curr_node = tree->root;
    // Inserting a node will always be a red node
    new_node->colour = Red;
    while (true) {
        if (new_node->price > curr_node->price) {
            // Logic for a new larger price
            if (curr_node->right == NULL) {
                curr_node->right = new_node;
                new_node->parent = curr_node;
                // Balance tree if issues caused
                balance_tree_insert(tree, new_node);
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


// Delete a node from a given tree while maintaining balance
void delete_node(treeStruct *tree, node *delNode) {
    // Keep track of moving nodes and data
    node *replacement = NULL;
    node *fixup_node = NULL;
    node *fixup_parent = NULL;
    nodeColour deleted_color = delNode->colour;
    bool deleted_was_left_child = false;

    // Track if delNode was a left child of its parent
    if (delNode->parent != NULL) {
        deleted_was_left_child = (delNode == delNode->parent->left) ? true : false;
    }
    // Node has no children
    if (delNode->left == NULL && delNode->right == NULL) {
        fixup_parent = delNode->parent;
        if (delNode->parent == NULL) {
            tree->root = NULL;
        } else if (deleted_was_left_child) {
            delNode->parent->left = NULL;
        } else {
            delNode->parent->right = NULL;
        }
    }
    //Node has one child
    else if (delNode->left == NULL || delNode->right == NULL) {
        replacement = (delNode->left != NULL) ? delNode->left : delNode->right;
        fixup_node = replacement;
        fixup_parent = delNode->parent;
        
        replacement->parent = delNode->parent;
        
        if (delNode->parent == NULL) {
            tree->root = replacement;
        } else if (deleted_was_left_child) {
            delNode->parent->left = replacement;
        } else {
            delNode->parent->right = replacement;
        }
    }
    // Node has two children
    else {
        node *successor = inorder_successor(delNode);
        // Color of the actually removed node
        deleted_color = successor->colour; 
        fixup_node = successor->right;
        
        if (successor->parent == delNode) {
            // Successor is direct right child of delNode
            fixup_parent = successor;
            deleted_was_left_child = false; 
        } else {
            // Successor is further down the tree
            fixup_parent = successor->parent;
            // successor is always left child of its parent
            deleted_was_left_child = true; 
            
            // Remove successor from its current position
            successor->parent->left = successor->right;
            if (successor->right != NULL) {
                successor->right->parent = successor->parent;
            }
            
            // Connect successor to delNode's right subtree
            successor->right = delNode->right;
            delNode->right->parent = successor;
        }
        // Replace delNode with successor
        successor->parent = delNode->parent;
        successor->left = delNode->left;
        delNode->left->parent = successor;
        // Preserve original color
        successor->colour = delNode->colour; 
        
        if (delNode->parent == NULL) {
            tree->root = successor;
        } else if (deleted_was_left_child) {
            delNode->parent->left = successor;
        } else {
            delNode->parent->right = successor;
        }
    }
    free(delNode); // TODO: KEEP THIS??
    
    // If we deleted a black node, we may need to rebalance
    if (deleted_color == Black) {
        balance_tree_delete(tree, fixup_node, fixup_parent, deleted_was_left_child);
    }
}

void balance_tree_delete(treeStruct *tree, node *fixup_node, node *parent, bool is_left_child) {
    // Continue until we reach root or find a red node to recolor black
    while (fixup_node != tree->root && (fixup_node == NULL || fixup_node->colour == Black)) {
        if (is_left_child) {
            node *sibling = parent->right;
            // Sibling is red
            if (sibling != NULL && sibling->colour == Red) {
                sibling->colour = Black;
                parent->colour = Red;
                trinode_left_rotation(tree, parent);
                 // Update sibling after rotation
                sibling = parent->right;
            }
            // Sibling is black with two black children
            if (sibling == NULL || 
                ((sibling->left == NULL || sibling->left->colour == Black) &&
                 (sibling->right == NULL || sibling->right->colour == Black))) {
                if (sibling != NULL) {
                    sibling->colour = Red;
                }
                fixup_node = parent;
                parent = parent->parent;
                if (parent != NULL) {
                    is_left_child = (fixup_node == parent->left);
                }
            } else {
                // Sibling is black, left child is red, right child is black
                if (sibling->right == NULL || sibling->right->colour == Black) {
                    if (sibling->left != NULL) {
                        sibling->left->colour = Black;
                    }
                    sibling->colour = Red;
                    trinode_right_rotation(tree, sibling);
                    sibling = parent->right;
                }
                // Sibling is black with red right child
                sibling->colour = parent->colour;
                parent->colour = Black;
                if (sibling->right != NULL) {
                    sibling->right->colour = Black;
                }
                trinode_left_rotation(tree, parent);
                fixup_node = tree->root; // Break out of loop
            }
        } else {
            // Mirror cases for right child
            node *sibling = parent->left;
            // Sibling is red
            if (sibling != NULL && sibling->colour == Red) {
                sibling->colour = Black;
                parent->colour = Red;
                trinode_right_rotation(tree, parent);
                sibling = parent->left;
            }
            // Sibling is black with two black children
            if (sibling == NULL || 
                ((sibling->left == NULL || sibling->left->colour == Black) &&
                 (sibling->right == NULL || sibling->right->colour == Black))) {
                if (sibling != NULL) {
                    sibling->colour = Red;
                }
                fixup_node = parent;
                parent = parent->parent;
                if (parent != NULL) {
                    is_left_child = (fixup_node == parent->left);
                }
            } else {
                // Sibling is black, right child is red, left child is black
                if (sibling->left == NULL || sibling->left->colour == Black) {
                    if (sibling->right != NULL) {
                        sibling->right->colour = Black;
                    }
                    sibling->colour = Red;
                    trinode_left_rotation(tree, sibling);
                    sibling = parent->left;
                }
                // Sibling is black with red left child
                sibling->colour = parent->colour;
                parent->colour = Black;
                if (sibling->left != NULL) {
                    sibling->left->colour = Black;
                }
                trinode_right_rotation(tree, parent);
                fixup_node = tree->root; // Break out of loop
            }
        }
    }
    // Ensure the fixup node is black
    if (fixup_node != NULL) {
        fixup_node->colour = Black;
    }
}


// Find inorder successor of a node for BST deletion
node *inorder_successor(node *delNode) {
    if (delNode->right == NULL) {
        return NULL;
    } else {
        node *curr_node = delNode->right;
        while (curr_node->left != NULL) {
            curr_node = curr_node->left;
        }
        return curr_node;
    }
}


// BST search for a node with a given price value
node *search_tree(treeStruct *tree, double searchPrice) {
    node *curr_node = tree->root; 
    while (true) {
        // Check if at correct node first
        if (searchPrice == curr_node->price) {
            return curr_node;
        // Check if we need to move right
        } else if (searchPrice > curr_node->price) {
            curr_node = curr_node->right;
            // If we reach NULL then the node can't exist
            if (curr_node == NULL) {
                return NULL;
            }
        // Move to the left
        } else {
            curr_node = curr_node->left;
            // If we reach NULL then the node can't exist
            if (curr_node == NULL) {
                return NULL;
            }
        }
    }
}


// Alter the fixed sized trees to retain their rules
void update_node() {

}


void main() {

}