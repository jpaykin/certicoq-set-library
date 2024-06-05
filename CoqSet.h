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

class CoqObject {
    private:
        // Each object will be added to a linked list of frames.
        // The dll node `this_node` will be populated with the value value_, which stores
        // the value underlying the object.
        value value_[1];
        struct stack_frame_dll node_;

        void initializeNode(); // called by constructor to insert node_ into global dll
        void freeNode(); // called by destructor to remove node_ from global dll

    public:

        value getValue() const { return value_[0]; };
        void setValue(value v) { value_[0] = v; };

        // Constructors and destructors
        CoqObject();
        CoqObject(value v);
        CoqObject(const CoqObject&); // copy constructor
        ~CoqObject() { freeNode(); }; // destructor
};

// Set of integers data structure
class set : public CoqObject {
    public:
        // empty set
        set();
        set(value v) : CoqObject(v) {};
        set(const set& s) : CoqObject(s) {}; // copy constructor
        ~set() = default;

        void add(int x);
        bool isMember(int x) const;
        int size() const;
};

}

#endif // COQ_SET_H