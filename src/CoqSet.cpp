#include "CoqSet.h"

extern "C" {
    extern struct thread_info *make_tinfo();
    extern value *get_args(value);
    extern value call(struct thread_info *, value, value);
    value body(struct thread_info *);

    extern value make_Coq_Init_Datatypes_bool_true(void);
    extern value make_Coq_Init_Datatypes_bool_false(void);
    extern unsigned int get_Coq_Init_Datatypes_bool_tag(value);
    extern void print_Coq_Init_Datatypes_bool(value);
    extern unsigned int get_Coq_Init_Datatypes_nat_tag(value);
}

namespace certicoq {


///////////////////
// Instantiation //
///////////////////

// This enum copies the order of constructors of MSet_struct from
// MSetImplementation.v
enum SetOpsTag {
        set_empty_tag,
        set_mem_tag,
        set_add_tag,
        set_cardinal_tag,
        set_elements_tag
};


value getBody() { 
    return applyTag(ThreadInfo::getBody(), SetBodyTag);
}

// Empty set
set::set() : CoqObject() {
    // Add an empty set to value_[0]
    setValue(get_args(getBody())[set_empty_tag]);
}

////////////////////
// set Operations //
////////////////////


void set::add(int x) {
    value vx = toValue<int>(x);
    value v = applyTag(getBody(), set_add_tag, vx, getValue());
    setValue(v);
}

bool set::isMember(int x) const {
    value vx = toValue<int>(x);
    value v = applyTag(getBody(), set_mem_tag, vx, getValue());
    return fromValue<bool>(v);
}

int set::size() const {
    value v = applyTag(getBody(), set_cardinal_tag, getValue());
    return fromValue<int>(v);
}

list<int> set::elems() const {
    value v = applyTag(getBody(), set_elements_tag, getValue());
    return list<int>(v);
}

}
