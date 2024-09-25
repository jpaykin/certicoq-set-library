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


// Global thread info
static struct thread_info* tinfo_ = NULL;
value GLOBAL__ROOT__[1];
struct stack_frame GLOBAL__FRAME__ = { GLOBAL__ROOT__ + 1, GLOBAL__ROOT__, NULL };
struct stack_frame_dll BASE = {NULL, GLOBAL__FRAME__, NULL};


///////////////////
// Instantiation //
///////////////////

// This enum copies the order of constructors of MSet_struct from
// MSetImplementation.v
enum CertiCoqTag {
        set_ops_tag
};
enum SetOpsTag {
        set_empty_tag,
        set_mem_tag,
        set_add_tag,
        set_cardinal_tag
};
enum natTag {
        nat_O_tag,
        nat_S_tag
};

//////////////////////
// Helper functions //
//////////////////////


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
int value_to_int(const value v) {
    return Int_val((long) v);
}
value int_to_value(const int x) {
    return (value)Val_int(x);
}
value uint63_from_nat(value n) {
  value temp = n;
  uint64_t i = 0;

  while (get_Coq_Init_Datatypes_nat_tag(temp) == nat_S_tag) {
    i++;
    temp = get_args(temp)[0];
  }
  return (value) ((i << 1) + 1);
}


//////////////////
// Constructors //
//////////////////

void CoqObject::initializeNode() {
    // Initialize node_ with a new frame
    node_.frame = {value_+1, value_, NULL};

    // Add node_ into the linked list of frames
    insertDLL(&node_, &BASE);
}

void CoqObject::freeNode() {
    removeDLL(&node_);
}

// Empty set
set::set() : CoqObject() {
    // Add an empty set to value_[0]
    setValue(get_args(get_args(GLOBAL__ROOT__[0])[set_ops_tag])[set_empty_tag]);
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


////////////
// Setter //
////////////


void initialize_global_thread_info() {
    if (tinfo_ == NULL) {
        tinfo_ = make_tinfo();

        GLOBAL__ROOT__[0] = body(tinfo_);
        tinfo_->fp = &(BASE.frame);
    }
}

/////////////////////
// set Operations //
/////////////////////


void set::add(int x) {
    value vx = int_to_value(x);

    value f  = get_args(GLOBAL__ROOT__[0])[set_ops_tag];
    f = get_args(f)[set_add_tag];
    value f0 = call(tinfo_, f, vx);
    value v  = call(tinfo_, f0, getValue());
    setValue(v);
}

bool set::isMember(int x) const {
    value vx = int_to_value(x);

    value f  = get_args(GLOBAL__ROOT__[0])[set_ops_tag];
    f = get_args(f)[set_mem_tag];
    value f0 = call(tinfo_, f, vx);
    value v  = call(tinfo_, f0, getValue());

    return value_to_bool(v);
}

int set::size() const {
    value f = get_args(GLOBAL__ROOT__[0])[set_ops_tag];
    f = get_args(f)[set_cardinal_tag];
    value v = call(tinfo_, f, getValue());
    return value_to_int(v);
}
}
