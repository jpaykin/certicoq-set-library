#ifndef COQ_SET_H
#define COQ_SET_H

#include <iostream>
#include <set>

extern "C" {
    // Files found in path-to-certicoq/plugin/runtime/
    #include "values.h"
    #include "gc_stack.h"
    #include "prim_int63.h"

}

namespace certicoq {


struct stack_frame_dll {
    struct stack_frame_dll* next;
    struct stack_frame* frame;
    struct stack_frame_dll* prev;
};


void initialize_global_thread_info();

// Set of integers data structure
class set {
    private:
        // the value underlying the set
        //value t_value_;

        // Each object in the set will be added to a linked list of frames.
        // The frame `this_frame` will be populated with the value myroot, which stores
        // the value underlying the set.
        // The pointer `prev_set_ptr` is a pointer to the frame that comes before
        // `this_frame` in the linked list.
        value myroot[1];
        struct stack_frame this_frame;
        struct stack_frame_dll this_node;

    public:
        value getValue() const { return myroot[0]; };
        void setValue(value v);

        // Constructors and destructors
        set(); // empty set
        ~set();

        void add(int x);
        bool isMember(int x) const;
        int size(); // should be const, but because we need to reference the pointer t_value_ here, is not
};

}

#endif // COQ_SET_H