//===----------------------------------------------------------------------===//
//
// This file defines the infrastructure needed to compile Coq APIs into C++
// classes. It includes:
//
// 1. The `stack_frame_dll` type and operations on it, used to safely store
//    Coq values on the CertiCoq heap.
// 2. A global instance of CertiCoq thread_info via a class `ThreadInfo`
// 3. A class `CoqObject`---a datatype backed by a single CertiCoq value
// 4. Various helper functions to interact with compiled CertiCoq code such as `calls`.
//
//===----------------------------------------------------------------------===//


#ifndef COQ_OPS_H
#define COQ_OPS_H

#include <cassert>
#include <iostream>
#include <vector>


extern "C" {
    #include "values.h"
    #include "gc_stack.h"
    #include "StackFrameDLL.h"

    extern value *get_args(value);
    extern value call(struct thread_info *, value, value);
}

namespace certicoq {

// Index into the extracted code
enum CoqOpsTag {
    SetBodyTag
};

// To interact with extracted Coq code, we need a thread_info pointer
// that manages the runtime/garbage collection.
// This thread_info pointer is a global singleton class that can be accessed via
// ThreadInfo::getTInfo()
class ThreadInfo {
    public:
        static struct thread_info* getTInfo();
        static struct stack_frame_dll* getBase();
        static value getBody();
        // ThreadInfo should not be cloneable or assignable
        ThreadInfo(ThreadInfo &other)     = delete;
        void operator=(const ThreadInfo&) = delete;
        ~ThreadInfo() = default;
    protected:
        struct thread_info* tinfo_;
        value root_[1];
        struct stack_frame_dll base_;
        // private constructor
        ThreadInfo();
        // private accessor
        struct thread_info* getTInfo_() {
            return tinfo_;
        }
        struct stack_frame_dll* getBase_() {
            return &base_;
        }
        value getBody_() {
            return root_[0];
        }

        // singleton instance pointer
        static ThreadInfo* instance;
};


/////////////////////
// CoqObject Class //
/////////////////////

class CoqObject {
    private:

    protected:

        value value_[1];
        // Each object will be added to a linked list of frames
        // The dll node `node_` will be populated with the value value_, which stores
        // the value underlying the object.
        struct stack_frame_dll node_;

        
        // helper functions for frame management
        void initializeNode(); // insert node_ into global dll
        void freeNode();       // remove node_ from global dll

        CoqObject(value v);
    public:
        CoqObject();
        CoqObject(const CoqObject&); // copy
        CoqObject& operator=(const CoqObject& other) { // assignment operator
            if (this != &other) {
                // The assignment operator should NOT copy the node_, which
                // stores address information, only the contents of the value
                // array
                setValue(other.getValue());
            }
            return *this;
        }
        ~CoqObject() { freeNode(); };

        void setValue(value v) { value_[0] = v; };
        value getValue() const { return value_[0]; };
};

//////////////////////
// Helper functions //
//////////////////////


// Templated functions that characterizes the ability to convert to and from
// values
template <typename T> value toValue(T v);
template <typename T> T fromValue(value v);

////////////////////////////
// Other helper functions //
////////////////////////////

// let v be a value of Coq type (option A)
// if v = None, throw an error with the given message
// Otherwise, if v = Some v', return v'
value fromOptionWithError(value v, std::string m);

value calls(struct thread_info* tinfo, value clos);
value calls(struct thread_info* tinfo, value clos, value arg0);
value calls(struct thread_info* tinfo, value clos,
            value arg0, value arg1);
value calls(struct thread_info* tinfo, value clos,
            value arg0, value arg1, value arg2);
value calls(struct thread_info* tinfo, value clos,
            value arg0, value arg1, value arg2, value arg3);
value calls(struct thread_info* tinfo, value clos,
            value arg0, value arg1, value arg2, value arg3, value arg4);

// A helper function to look up a tag in the body of
// the main function and apply that callback to any
// number of value arguments
template<typename... Values>
value applyTag(value body, int tag, Values... args) {
    auto tinfo = ThreadInfo::getTInfo();
    value fun  = get_args(body)[tag];
    return calls(tinfo, fun, args...);
}
template<typename... Values>
value applyTag(const CoqObject& body, int tag, Values... args) {
    return applyTag(body.getValue(), tag, args...);
}

}

#endif // COQ_OPS_H