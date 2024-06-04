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
void insert(struct stack_frame_dll* new_node,
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
void remove(struct stack_frame_dll* node) {
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


///////////////////
// Instantiation //
///////////////////

// This enum copies the order of constructors of MSet_struct from
// MSetImplementation.v
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


// Empty set
set::set() {
    // Add an empty set to t_value_[0]
    t_value_[0] = get_args(GLOBAL__ROOT__[0])[set_empty_tag];

    // Initialize this_node with a new frame
    this_node.frame = {t_value_+1, t_value_, NULL};

    // Add this_node into the linked list of frames
    insert(&this_node, &BASE);
}

// Copy constructor
set::set(const set& other) {
    t_value_[0] = other.getValue();

    // Initialize this_node with a new frame
    this_node.frame = {t_value_+1, t_value_, NULL};

    // Add this_node into the linked list of frames
    insert(&this_node, &BASE);
}


// Destructor
 set::~set() { 
    remove(&this_node);
};

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

void set::setValue(value v) {
    t_value_[0] = v;
}

/////////////////////
// set Operations //
/////////////////////


void set::add(int x) {
    value vx = int_to_value(x);

    value f  = get_args(GLOBAL__ROOT__[0])[set_add_tag];
    value f0 = call(tinfo_, f, vx);
    value v  = call(tinfo_, f0, getValue());
    setValue(v);
}

bool set::isMember(int x) const {
    value vx = int_to_value(x);

    value f  = get_args(GLOBAL__ROOT__[0])[set_mem_tag];
    value f0 = call(tinfo_, f, vx);
    value v  = call(tinfo_, f0, getValue());

    return value_to_bool(v);
}

int set::size() const {
    value f = get_args(GLOBAL__ROOT__[0])[set_cardinal_tag];
    value v = call(tinfo_, f, getValue());
    return value_to_int(v);
}
}
