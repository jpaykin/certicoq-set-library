#ifndef STACK_FRAME_DLL_H
#define STACK_FRAME_DLL_H


// Files found in path-to-certicoq/plugin/runtime/
#include "values.h"
#include "gc_stack.h"

struct stack_frame_dll {
    struct stack_frame_dll* next;
    struct stack_frame frame;
    struct stack_frame_dll* prev;
};

// Insert a new stack_frame_dll node into an existing stack_frame_dll stack.
//
// Assume the stack had the form:
//
//     A[next,frameA,prev=B] <-> B[next=A,frameB,prev]
//              |                            |
//              v                            v
//     [next,root,prev=frameB] ->  [nextB,rootB,prev]
//
// The result should have the form:
//
//     A[next,frameA,prev=new] <-> new[next=A,frameNew,prev=B]  <-> B[next=new,frameB,prev]
//              |                            |                         |
//              v                            v                         v
//     [-,-,prev=frameNew]     ->     [-,-,prev=frameB]       ->  [-,-,prev]
//
// Want to insert a new stack frame node between A and B
//
// Parameters:
//
//      new_node: a non-null pointer to a stack_frame_dll. The next and prev fields
//      of new_node are ignored, and can be assumed to be NULL.
//
//      A: a non-null pointer to a stack_frame_dll. A should be on the current stack in tinfo.
//      Typical argument for A is &BASE
//
void insertDLL(struct stack_frame_dll* new_node,
                struct stack_frame_dll* A);

// Remove the given stack_frame_dll node from its doubly linked list.
//
// Parameters:
//
//      node: a non-NULL pointer to a stack_frame_dll
// Assume the stack had the form:
//
//      A[next,frameA,prev=new] <-> node[next=A,frameN,prev=B]  <-> B[next=node,frameB,prev]
//               |                            |                         |
//               v                            v                         v
//      [-,-,prev=frameN]      ->     [-,-,prev=frameB]       ->  [-,-,prev]
//
// Then the result should have the form:
//
//      A[next,frameA,prev=B] <-> B[next=A,frameB,prev]
//               |                            |
//               v                            v
//      [next,root,prev=frameB] ->  [nextB,rootB,prev]
//
void removeDLL(struct stack_frame_dll* node);


#endif