#include "StackFrameDLL.h"
#include <stdlib.h> // NULL

void insertDLL(struct stack_frame_dll* new_node,
                struct stack_frame_dll* A){
    struct stack_frame_dll* B = A->prev;

    // Update new_node to point to A (next) and B (prev)
    new_node->next = A;
    new_node->prev = B;
    new_node->frame.prev = A->frame.prev;

    // Update A->prev to point to new_node and A->frame to point to the new
    // frame
    A->prev        = new_node;
    A->frame.prev = &(new_node->frame);

    // Update B->next to point to new_node
    if (B != NULL) {
        B->next = new_node;
    }
}

void removeDLL(struct stack_frame_dll* node) {
    struct stack_frame_dll* A = node->next;
    struct stack_frame_dll* B = node->prev;

    if (A != NULL) {
        A->prev = B;
        A->frame.prev = node->frame.prev; // in case B is null
    }
    if (B != NULL) {
        B->next = A;
    }
}