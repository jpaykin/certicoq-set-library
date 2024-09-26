#ifndef COQ_SET_H
#define COQ_SET_H

#include <iostream>
#include <set>
#include "CoqOps.h"
#include "CoqList.h"

extern "C" {
    // Files found in path-to-certicoq/plugin/runtime/
    #include "values.h"
    #include "gc_stack.h"
    #include "prim_int63.h"
}

namespace certicoq {

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
        list<int> elems() const;

};

}

#endif // COQ_SET_H