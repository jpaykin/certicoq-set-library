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
    struct stack_frame frame;
    struct stack_frame_dll* prev;
};


void initialize_global_thread_info();

// Set of integers data structure
class set {
    private:
        // Each object in the set will be added to a linked list of frames.
        // The frame `this_frame` will be populated with the value t_value_, which stores
        // the value underlying the set.
        // The pointer `prev_set_ptr` is a pointer to the frame that comes before
        // `this_frame` in the linked list.
        value t_value_[1];
        struct stack_frame_dll this_node;

    public:
        value getValue() const { return t_value_[0]; };
        void setValue(value v);

        // Constructors and destructors
        set(); // empty set
        set(const set&); // copy constructor
        ~set();

        void add(int x);
        bool isMember(int x) const;
        int size() const;
};

}

#endif // COQ_SET_H