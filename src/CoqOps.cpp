#include "CoqOps.h"


extern "C" {
    #include "live_pointers.h"
    #include <stdbool.h>

    extern struct thread_info *make_tinfo();
    value body(struct thread_info *);

    extern value make_Coq_Init_Datatypes_bool_true(void);
    extern value make_Coq_Init_Datatypes_bool_false(void);
    extern unsigned int get_Coq_Init_Datatypes_bool_tag(value);
    extern void print_Coq_Init_Datatypes_bool(value);

    extern unsigned int get_Coq_Init_Datatypes_option_tag(value);
}

namespace certicoq {

/////////////////////////////////
// Instantiation of ThreadInfo //
/////////////////////////////////

    ThreadInfo* ThreadInfo::instance = NULL;

    struct thread_info* ThreadInfo::getTInfo() {
        if (ThreadInfo::instance == NULL) {
            ThreadInfo::instance = new ThreadInfo();
        }
        return (ThreadInfo::instance)->getTInfo_();
    }

    struct stack_frame_dll* ThreadInfo::getBase() {
        if (ThreadInfo::instance == NULL) {
            ThreadInfo::instance = new ThreadInfo();
        }
        return ThreadInfo::instance->getBase_();
    }

    value ThreadInfo::getBody() {
        if (ThreadInfo::instance == NULL) {
            ThreadInfo::instance = new ThreadInfo();
        }
        return ThreadInfo::instance->getBody_();
    }

    // Private Constructor
    ThreadInfo::ThreadInfo() {
        tinfo_ = make_tinfo();

        base_.prev = NULL;
        base_.next = NULL;
        base_.frame = {root_+1, root_, NULL};
        root_[0] = body(tinfo_);
        tinfo_->fp = &(base_.frame);
    };

////////////////////////////////////////////
// Instantiation of toValue and fromValue //
////////////////////////////////////////////

// specialization to bool
bool value_to_bool(const value b) {
    return (get_Coq_Init_Datatypes_bool_tag(b) == 0);
}
value bool_to_value(const bool b) {
    if (b) {
        return make_Coq_Init_Datatypes_bool_true();
    } else {
        return make_Coq_Init_Datatypes_bool_false();
    }
}

template<> value toValue<bool>(bool b) { return bool_to_value(b); }
template<> bool  fromValue<bool>(value v) { return value_to_bool(v); }

// specialization to int
// NOTE: we don't use this right now, and it may be better not to confuse it with 
// `toValue<unsigned int>`, which produces an `N` type.
template<> value toValue<int>(int x) { return (value)Val_int(x); }
template<> int fromValue<int>(value v) { return Int_val((long) v); }


// specialization to CoqOjbect
template<> value toValue<CoqObject>(CoqObject v) {
    return v.getValue();
}


/////////////////////////////////////////////
// Instantiation of other helper functions //
/////////////////////////////////////////////

// let v be a value of Coq type (option A)
// if v = None, throw an error with the given message
// Otherwise, if v = Some v', return v'
enum option_tag {Some_tag, None_tag};
value fromOptionWithError(value v, std::string m) {
    unsigned int ctr = get_Coq_Init_Datatypes_option_tag(v);
    if (v == None_tag) {
      throw(std::runtime_error(m));
    } else {
      return get_args(v)[0];
    }
}



// Call a closure on a variable number of arguments
value calls(struct thread_info* tinfo, value clos) {
    return clos;
}
value calls(struct thread_info* tinfo, value clos, value arg0) {
    return call(tinfo, clos, arg0);
}
value calls(struct thread_info* tinfo, value clos,
            value arg0, value arg1) {
    BEGINFRAME(tinfo, 1);
    value f = clos;
    f = LIVEPOINTERS1(tinfo, call(tinfo, f, arg0), arg1);
    f = call(tinfo, f, arg1);
    return f;
    ENDFRAME
}
value calls(struct thread_info* tinfo, value clos,
            value arg0, value arg1, value arg2) {
    BEGINFRAME(tinfo, 2);
    value f = clos;
    f = LIVEPOINTERS2(tinfo, call(tinfo, f, arg0), arg1, arg2);
    f = LIVEPOINTERS1(tinfo, call(tinfo, f, arg1), arg2);
    return call(tinfo, f, arg2);
    ENDFRAME
}
value calls(struct thread_info* tinfo, value clos,
            value arg0, value arg1, value arg2, value arg3) {
    BEGINFRAME(tinfo, 3);
    value f = clos;
    f = LIVEPOINTERS3(tinfo, call(tinfo, f, arg0), arg1, arg2, arg3);
    f = LIVEPOINTERS2(tinfo, call(tinfo, f, arg1), arg2, arg3);
    f = LIVEPOINTERS1(tinfo, call(tinfo, f, arg2), arg3);
    return call(tinfo, f, arg3);
    ENDFRAME
}
value calls(struct thread_info* tinfo, value clos,
            value arg0, value arg1, value arg2, value arg3, value arg4) {
    BEGINFRAME(tinfo, 4);
    value f = clos;
    f = LIVEPOINTERS4(tinfo, call(tinfo, f, arg0), arg1, arg2, arg3, arg4);
    f = LIVEPOINTERS3(tinfo, call(tinfo, f, arg1), arg2, arg3, arg4);
    f = LIVEPOINTERS2(tinfo, call(tinfo, f, arg2), arg3, arg4);
    f = LIVEPOINTERS1(tinfo, call(tinfo, f, arg3), arg4);
    return call(tinfo, f, arg4);
    ENDFRAME
}
// template<typename... Values>
// value calls(struct thread_info* tinfo, value clos, value arg0, Values... args) {
//     value f = call(tinfo, clos, arg0);
//     return calls(tinfo, f, args...);
// }


/////////////////////////////
// CoqObject Instantiation //
/////////////////////////////

void CoqObject::initializeNode() {
    // Initialize node_ with a new frame
    node_.frame = {value_+1, value_, NULL};

    // Add node_ into the linked list of frames
    insertDLL(&node_, ThreadInfo::getBase());
}
void CoqObject::freeNode() {
    removeDLL(&node_);
}

CoqObject::CoqObject() {
    initializeNode();
}

// Constructor
CoqObject::CoqObject(value v) {
    setValue(v);
    initializeNode();
}


// Copy constructor
CoqObject::CoqObject(const CoqObject& other) {
    setValue(other.getValue());
    initializeNode();
}

}