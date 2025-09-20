#include "order_book.h"


#define MAX_TREE_SIZE 10


// Insert new nodes into the tree until it's max-size is reached
void insert_node(treeStruct *tree, node *new_node) {
    /* Find current best node and compare prices to check if possible trade occured
       Since our data doesn't contain trade data we can only make assumptions
       and infer when a trade could have occured, so by checking if we have a new
       best that is worse than before, we can assume a trader took advantage of the
       previous, better price(s) for the bid/ask and so we can remove them from our tree
    */
    node *best_node = find_best_node(tree);
    if (tree->type == Bid && best_node != NULL && new_node->price < best_node->price) {
        // Delete nodes better than new best
        recursive_delete(tree, tree->root, new_node);
    } else if (tree->type == Ask && best_node != NULL && new_node->price > best_node->price) {
        // Delete nodes better than new best
        recursive_delete(tree, tree->root, new_node);
    }
    // Check if tree empty
    if (tree->size == 0) {
        tree->root = new_node;
        new_node->colour = Black;  // Using the rule that the root is always black... At least to start with
        tree->size += 1;
        return;
    }
    // Start at trees root node
    node *curr_node = tree->root;
    // Inserting a node will always be a red node
    new_node->colour = Red;
    // Insert the new node into the tree
    while (true) {
        if (new_node->price > curr_node->price) {
            // Logic for a new larger price
            if (curr_node->right == NULL) {
                curr_node->right = new_node;
                new_node->parent = curr_node;
                // Balance tree if issues caused
                balance_tree_insert(tree, new_node);
                tree->size += 1;
                if (tree->size > 10) {
                    curr_node = find_worst_node(tree);
                    delete_node(tree, curr_node);
                }
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
                if (tree->size > 10) {
                    curr_node = find_worst_node(tree);
                    delete_node(tree, curr_node);
                }
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
    while (curr_node != NULL && curr_node->parent != NULL && curr_node->parent->colour == Red) {
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
                trinode_right_rotation(tree, curr_node->parent->parent);
            }
        } else {
            node *uncle = curr_node->parent->parent->left;
            if (uncle != NULL && uncle->colour == Red) {
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
    delNode = NULL;
    
    // If we deleted a black node, we may need to rebalance
    if (deleted_color == Black) {
        balance_tree_delete(tree, fixup_node, fixup_parent, deleted_was_left_child);
    }
    // Make tree size smaller to allow new node to be added
    tree->size -= 1;
}


// Balance tree after deletion of node
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


// Used for deleting all nodes better than new best
void recursive_delete(treeStruct *tree, node *curr_node, node *best_node) {
    if (curr_node == NULL) {
        return;
    }
    recursive_delete(tree, curr_node->left, best_node);
    recursive_delete(tree, curr_node->right, best_node);

    if (tree->type == Bid) {
        if (curr_node->price > best_node->price) {
            delete_node(tree, curr_node);
        }
    } else {
        if (curr_node->price < best_node->price) {
            delete_node(tree, curr_node);
        }
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


// Find best ask or bid price depending on tree
node *find_best_node(treeStruct *tree) {
    // Check if tree empty
    if (tree->root == NULL) {
        return NULL;
    }
    node *curr_node = tree->root;
    if (tree->type == Bid) {
        while (curr_node->right != NULL) {
            curr_node = curr_node->right;
        }
        return curr_node;
    } else {
        while (curr_node->left != NULL) {
            curr_node = curr_node->left;
        }
        return curr_node;
    }
}


// Find worst ask or bid price depending on tree
node *find_worst_node(treeStruct *tree) {
    node *curr_node = tree->root;
    // Check if tree empty
    if (curr_node == NULL) {
        return NULL;
    }
    if (tree->type == Bid) {
        while (curr_node->left != NULL) {
            curr_node = curr_node->left;
        }
        return curr_node;
    } else {
        while (curr_node->right != NULL) {
            curr_node = curr_node->right;
        }
        return curr_node;
    }
}


// Find the next best node to move to after best -- Basically inorder traversal step
node *find_next_best(treeStruct *tree, node *curr_node) {
    if (curr_node == NULL) {
        return NULL;
    }
    // Find next highest bid
    if (tree->type == Bid) {
        // Find right most node of left subtree
        if (curr_node->left != NULL) {
            curr_node = curr_node->left;
            while (curr_node->right != NULL) {
                curr_node = curr_node->right;
            }
            return curr_node;
        }
        // If no left subtree find a node that is a right child
        node *parent = curr_node->parent;
        while (parent != NULL && curr_node == parent->left) {
            curr_node = parent;
            parent = parent->parent;

        }
        return parent;
    // Find next smallest ask
    } else {
        // Find left most node of right subtree
        if (curr_node->right != NULL) {
            curr_node = curr_node->right;
            while (curr_node->left != NULL) {
                curr_node = curr_node->left;
            }
            return curr_node;
        }
        // If no right subtree find a node that is a left child
        node *parent = curr_node->parent;
        while (parent != NULL && curr_node == parent->right) {
            curr_node = parent;
            parent = parent->parent;

        }
        return parent;
    }
}


// Alter the volume of an order
void update_node_volume(treeStruct *tree, node *curr_node, double volumeChange) {
    curr_node->volume += volumeChange;
}


//! Clean up functions - Freeing allocated memory
// Recursively Free a node and all decendents
void free_nodes(node *curr_node) {
    if (curr_node == NULL) {
        return;
    }
    free_nodes(curr_node->left);
    free_nodes(curr_node->right);

    free(curr_node); // free this node AFTER its children
}


// Free up all nodes in the tree
void free_tree(treeStruct *tree) {
    if (tree == NULL) {
        return;
    }
    free_nodes(tree->root);
    tree->root = NULL;
    tree->size = 0;
}


//! PRINTING TREE -- PROBABLY DELETE
// Visual tree structure (horizontal layout)
void print_tree_visual(treeStruct *tree) {
    if (tree == NULL || tree->root == NULL) {
        printf("Tree is empty\n");
        return;
    }
    
    printf("Red-Black Tree Structure:\n");
    printf("Format: Value(Color) [LEFT: child | RIGHT: child]\n");
    printf("Colors: R = Red, B = Black\n");
    printf("----------------------------------------\n");
    print_tree_recursive(tree->root, 0, "ROOT");
    printf("\n");
}


void print_tree_recursive(node *root, int depth, char *prefix) {
    if (root == NULL) {
        return;
    }
    
    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("│   ");
    }
    
    // Print current node with clear left/right children info
    printf("├── %s: %lf(%c)%lf [LEFT: ", prefix, root->price, root->colour == Red ? 'R' : 'B', root->volume);
    if (root->left != NULL) {
        printf("%lf(%c)%lf", root->left->price, root->left->colour == Red ? 'R' : 'B',root->left->volume);
    } else {
        printf("NULL");
    }
    printf(" | RIGHT: ");
    if (root->right != NULL) {
        printf("%lf(%c)%lf", root->right->price, root->right->colour == Red ? 'R' : 'B',root->right->volume);
    } else {
        printf("NULL");
    }
    printf("]\n");
    
    // Recursively print children
    if (root->left != NULL) {
        print_tree_recursive(root->left, depth + 1, "LEFT");
    }
    
    if (root->right != NULL) {
        print_tree_recursive(root->right, depth + 1, "RIGHT");
    }
}




/* orderLine ol;
char filename[] = "GBPUSD_mt5_ticks.csv";  //TODO: Improve this */

/* void main() {
    // Initialise file pointer - so we can leave file open
    FILE *fp = open_data_file(filename);
    // Initialise bid and ask trees
    treeStruct bidTree = {Bid, NULL, 0};
    treeStruct askTree = {Ask, NULL, 0};
    
     while (read_next_line(fp, &ol) > 0) {
        printf("Tick: %s %s bid=%.5f ask=%.5f vol_bid=%.2f vol_ask=%.2f\n",
               ol.date, ol.time, ol.bidPrice, ol.askPrice, ol.bidVolume, ol.askVolume);
    } 

    while (read_next_line(fp, &ol) > 0) {
        node *bid_node = malloc(sizeof(node));
        if (!bid_node) {
            printf("Error Allocating Memory!\n");
            exit(-1);
        }
        *bid_node = (node){ol.bidPrice, ol.bidVolume, Red, NULL, NULL, NULL};

        node *ask_node = malloc(sizeof(node));
        if (!ask_node) {
            printf("Error Allocating Memory!\n");
            exit(-1);
        }
        *ask_node = (node){ol.askPrice, ol.askVolume, Red, NULL, NULL, NULL};

        insert_node(&bidTree, bid_node);
        insert_node(&askTree, ask_node);
        print_tree_visual(&askTree);
    }
} */