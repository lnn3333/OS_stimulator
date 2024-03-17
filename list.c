#include "list.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// Static allocation of nodes and lists
static Node nodes[LIST_MAX_NUM_NODES];
static List lists[LIST_MAX_NUM_HEADS];
static int nodeIndex = 0;
static int listIndex = 0;

// Function to create a new list
List *List_create() {
    if (listIndex >= LIST_MAX_NUM_HEADS)
        return NULL;
    
    List *pList = &lists[listIndex++];
    pList->pFirstNode = NULL;
    pList->pLastNode = NULL;
    pList->pCurrentNode = NULL;
    pList->count = 0;
    pList->lastOutOfBoundsReason = LIST_OOB_END;
    return pList;
}

// Function to count the number of items in the list
int List_count(List *pList) {
    if (pList == NULL)
        return 0;
    return pList->count;
}

// Function to return the first item in the list
void *List_first(List *pList) {
    if (pList == NULL || pList->pFirstNode == NULL)
        return NULL;
    pList->pCurrentNode = pList->pFirstNode;
    return pList->pCurrentNode->pItem;
}

// Function to return the last item in the list
void *List_last(List *pList) {
    if (pList == NULL || pList->pLastNode == NULL)
        return NULL;
    pList->pCurrentNode = pList->pLastNode;
    return pList->pCurrentNode->pItem;
}

// Function to advance to the next item in the list
void *List_next(List *pList) {
    if (pList == NULL || pList->pCurrentNode == NULL)
        return NULL;
    if (pList->pCurrentNode->pNext == NULL) {
        pList->lastOutOfBoundsReason = LIST_OOB_END;
        return NULL;
    }
    pList->pCurrentNode = pList->pCurrentNode->pNext;
    return pList->pCurrentNode->pItem;
}

// Function to go back to the previous item in the list
void *List_prev(List *pList) {
    if (pList == NULL || pList->pCurrentNode == NULL)
        return NULL;
    if (pList->pCurrentNode->pPrev == NULL) {
        pList->lastOutOfBoundsReason = LIST_OOB_START;
        return NULL;
    }
    pList->pCurrentNode = pList->pCurrentNode->pPrev;
    return pList->pCurrentNode->pItem;
}

// Function to return the current item in the list
void *List_curr(List *pList) {
    if (pList == NULL || pList->pCurrentNode == NULL)
        return NULL;
    return pList->pCurrentNode->pItem;
}

// Function to insert an item after the current item
int List_insert_after(List *pList, void *pItem) {
    if (pList == NULL || pItem == NULL)
        return LIST_FAIL;
    if (nodeIndex >= LIST_MAX_NUM_NODES)
        return LIST_FAIL;

    Node *newNode = &nodes[nodeIndex++];
    newNode->pItem = pItem;
    newNode->pNext = pList->pCurrentNode->pNext;
    if (pList->pCurrentNode->pNext != NULL)
        pList->pCurrentNode->pNext->pPrev = newNode;
    newNode->pPrev = pList->pCurrentNode;
    pList->pCurrentNode->pNext = newNode;
    if (pList->pCurrentNode == pList->pLastNode)
        pList->pLastNode = newNode;
    pList->count++;
    return LIST_SUCCESS;
}

// Function to insert an item before the current item
int List_insert_before(List *pList, void *pItem) {
    if (pList == NULL || pItem == NULL)
        return LIST_FAIL;
    if (nodeIndex >= LIST_MAX_NUM_NODES)
        return LIST_FAIL;

    Node *newNode = &nodes[nodeIndex++];
    newNode->pItem = pItem;
    newNode->pNext = pList->pCurrentNode;
    if (pList->pCurrentNode->pPrev != NULL)
        pList->pCurrentNode->pPrev->pNext = newNode;
    newNode->pPrev = pList->pCurrentNode->pPrev;
    pList->pCurrentNode->pPrev = newNode;
    if (pList->pCurrentNode == pList->pFirstNode)
        pList->pFirstNode = newNode;
    pList->count++;
    return LIST_SUCCESS;
}

// Function to append an item to the end of the list
int List_append(List *pList, void *pItem) {
    if (pList == NULL || pItem == NULL)
        return LIST_FAIL;
    if (nodeIndex >= LIST_MAX_NUM_NODES)
        return LIST_FAIL;

    Node *newNode = &nodes[nodeIndex++];
    newNode->pItem = pItem;
    newNode->pNext = NULL;
    newNode->pPrev = pList->pLastNode;
    if (pList->pLastNode != NULL)
        pList->pLastNode->pNext = newNode;
    pList->pLastNode = newNode;
    if (pList->pFirstNode == NULL)
        pList->pFirstNode = newNode;
    
    pList->count++;
    return LIST_SUCCESS;
}

// Function to prepend an item to the start of the list
int List_prepend(List *pList, void *pItem) {
    if (pList == NULL || pItem == NULL)
        return LIST_FAIL;
    if (nodeIndex >= LIST_MAX_NUM_NODES)
        return LIST_FAIL;

    Node *newNode = &nodes[nodeIndex++];
    newNode->pItem = pItem;
    newNode->pNext = pList->pFirstNode;
    newNode->pPrev = NULL;
    if (pList->pFirstNode != NULL)
        pList->pFirstNode->pPrev = newNode;
    pList->pFirstNode = newNode;
    if (pList->pLastNode == NULL)
        pList->pLastNode = newNode;
    pList->count++;
    return LIST_SUCCESS;
}

// Function to remove the current item from
// Function to remove the current item from the list
void *List_remove(List *pList) {
    if (pList == NULL || pList->pCurrentNode == NULL)
        return NULL;

    // Store a reference to the current node to be removed
    Node *removedNode = pList->pCurrentNode;
    void *removedItem = removedNode->pItem;

    // Update pointers to remove the node from the list
    if (removedNode->pPrev != NULL) {
        removedNode->pPrev->pNext = removedNode->pNext;
    } else {
        pList->pFirstNode = removedNode->pNext; // Update first node if removing the first node
    }

    if (removedNode->pNext != NULL) {
        removedNode->pNext->pPrev = removedNode->pPrev;
    } else {
        pList->pLastNode = removedNode->pPrev; // Update last node if removing the last node
    }

    // Move the current pointer to the next node
    if (pList->pCurrentNode->pNext != NULL) {
        pList->pCurrentNode = pList->pCurrentNode->pNext;
    } else if (pList->pCurrentNode->pPrev != NULL) {
        pList->pCurrentNode = pList->pCurrentNode->pPrev;
    } else {
        pList->pCurrentNode = NULL; // No more nodes left in the list
    }

    // Free the removed node by marking it as unused
    removedNode->pNext = NULL;
    removedNode->pPrev = NULL;
    removedNode->pItem = NULL;
    pList->count--;

    return removedItem;
}

void *List_trim(List *pList) {
    if (pList == NULL || pList->pLastNode == NULL) {
        return NULL; // List is empty or invalid
    }

    Node *last_node = pList->pLastNode;
    void *item = last_node->pItem; // Save a reference to the item

    // Update the last node and the list pointers
    pList->pLastNode = last_node->pPrev;
    if (pList->pLastNode != NULL) {
        pList->pLastNode->pNext = NULL;
    } else {
        // List became empty
        pList->pFirstNode = NULL;
    }

    // Clean up the removed node
    last_node->pPrev = NULL;
    last_node->pItem = NULL; // Optionally NULL out the item
    last_node->pNext = NULL;

    return item;
}

// comparator function
bool pComparator(void *pItem, void *pComparisonArg)
{
    int pItem_value = *(int *)pItem;
    int pComparisonArg_value = *(int*) pComparisonArg;
    return (pItem_value == pComparisonArg_value);
};


void *List_search(List *pList, COMPARATOR_FN pComparator, void *pComparisonArg) {
    // Check for NULL pointers
    if (pList == NULL || pList->pFirstNode == NULL || pComparator == NULL) {
        return NULL;
    }

    Node *ptr_node = pList->pFirstNode;
    while (ptr_node != NULL) {
        // Use the comparison function to check for a match
        if (pComparator(ptr_node->pItem, pComparisonArg)) {
            
            // Match found, update the current node and return the item
            pList->pCurrentNode = ptr_node;
            return ptr_node->pItem;
        }
        ptr_node = ptr_node->pNext;
    }
    // No match found, set the out-of-bounds reason
    pList->lastOutOfBoundsReason = LIST_OOB_END;
    return NULL;
}

