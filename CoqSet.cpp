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
struct stack_frame *BASE_FRAME = &GLOBAL__FRAME__;
struct stack_frame_dll BASE_DLL = {NULL, BASE_FRAME, NULL};

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
//      Typical argument for A is &BASE_DLL
//
void insert(struct stack_frame_dll* new_node,
            struct stack_frame_dll* A){
    struct stack_frame_dll* B = A->prev;

    // Update new_node to point to A (next) and B (prev)
    new_node->next = A;
    new_node->prev = B;
    new_node->frame->prev = A->frame->prev;

    // Update A->prev to point to new_node and A->frame to point to the new
    // frame
    A->prev        = new_node;
    A->frame->prev = new_node->frame;

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
        A->frame->prev = node->frame->prev; // in case B is null
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


// These macros are taken from VeriFFI examples,
// e.g. https://github.com/CertiCoq/VeriFFI/blob/main/examples/array/prims.c
// e.g. https://github.com/CertiCoq/VeriFFI/blob/d33c677e3af8a62eaa14e262a1bdd1908bf3bc3d/examples/uint63nat/prims.c
// See https://github.com/CertiCoq/certicoq/wiki/When-C-code-allocates-on-the-Coq-heap
// for more info
#define BEGINFRAME(tinfo,n) {{{{{ \
   value *_ALLOC, *_LIMIT; \
   value __ROOT__[n];   \
   struct stack_frame __FRAME__ = { NULL/*bogus*/, __ROOT__, tinfo->fp }; \
   struct stack_frame *__PREV__; \
   size_t nalloc; \
   value __RTEMP__;

#define ENDFRAME }}}}}

#define LIVEPOINTERS0(tinfo, exp) (exp)

#define LIVEPOINTERS1(tinfo, exp, a0) \
   (tinfo->fp= &__FRAME__, __FRAME__.next=__ROOT__+1, \
   __ROOT__[0]=(a0), __RTEMP__=(exp), (a0)=__ROOT__[0], \
   __PREV__=__FRAME__.prev, tinfo->fp=__PREV__, __RTEMP__)

#define LIVEPOINTERS2(tinfo, exp, a0, a1)	\
  (tinfo->fp= &__FRAME__, __FRAME__.next=__ROOT__+2, \
  __ROOT__[0]=(a0), __ROOT__[1]=(a1),		\
  __RTEMP__=(exp),                              \
  (a0)=__ROOT__[0], (a1)=__ROOT__[1],             \
   __PREV__=__FRAME__.prev, tinfo->fp=__PREV__, __RTEMP__)

#define LIVEPOINTERS3(tinfo, exp, a0, a1, a2)   \
  (tinfo->fp= &__FRAME__, __FRAME__.next=__ROOT__+3,                       \
  __ROOT__[0]=(a0), __ROOT__[1]=(a1), __ROOT__[2]=(a2),  \
  __RTEMP__=(exp),                                       \
  (a0)=__ROOT__[0], (a1)=__ROOT__[1], (a2)=__ROOT__[2],    \
   __PREV__=__FRAME__.prev, tinfo->fp=__PREV__, __RTEMP__)

#define LIVEPOINTERS4(tinfo, exp, a0, a1, a2, a3)	\
  (tinfo->fp= &__FRAME__,  __FRAME__.next=__ROOT__+4,  \
  __ROOT__[0]=(a0), __ROOT__[1]=(a1), __ROOT__[2]=(a2), __ROOT__[3]=(a3),  \
  __RTEMP__=(exp),                                       \
  (a0)=__ROOT__[0], (a1)=__ROOT__[1], (a2)=__ROOT__[2], (a3)=__ROOT__[3],    \
   __PREV__=__FRAME__.prev, tinfo->fp=__PREV__, __RTEMP__)

#define GC_SAVE1(tinfo, save0) \
    if (!(_LIMIT=tinfo->limit, _ALLOC=tinfo->alloc, nalloc <= _LIMIT-_ALLOC)) { \
    tinfo->nalloc = nalloc;  \
    LIVEPOINTERS1(tinfo,(garbage_collect(tinfo),(value)NULL),save0);	\
  }

#define GC_SAVE2(tinfo, save0, save1) \
    if (!(_LIMIT=tinfo->limit, _ALLOC=tinfo->alloc, nalloc <= _LIMIT-_ALLOC)) { \
    tinfo->nalloc = nalloc;  \
    LIVEPOINTERS2(tinfo,(garbage_collect(tinfo),(value)NULL),save0,save1);  \
  }
#define GC_SAVE3(tinfo, save0, save1, save2) \
    if (!(_LIMIT=tinfo->limit, _ALLOC=tinfo->alloc, nalloc <= _LIMIT-_ALLOC)) { \
    tinfo->nalloc = nalloc;  \
    LIVEPOINTERS2(tinfo,(garbage_collect(tinfo),(value)NULL),save0,save1,save2);  \
  }


//////////////////
// Constructors //
//////////////////


// Empty set
set::set() {
    // Add an empty set to myroot[0]
    myroot[0] = get_args(GLOBAL__ROOT__[0])[set_empty_tag];

    // Initialize this_node with a new frame
    this_frame = {myroot+1, myroot, NULL};
    this_node.frame = &this_frame;

    // Add this_node into the linked list of frames
    insert(&this_node, &BASE_DLL);
}

// Copy constructor
set::set(const set& other) {
    myroot[0] = other.getValue();

    // Initialize this_node with a new frame
    this_frame = {myroot+1, myroot, NULL};
    this_node.frame = &this_frame;

    // Add this_node into the linked list of frames
    insert(&this_node, &BASE_DLL);
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
        tinfo_->fp = BASE_FRAME;
    }
}


void set::setValue(value v) {
    myroot[0] = v;
}

/////////////////////
// set Operations //
/////////////////////


void set::add(int x) {
    value vx = int_to_value(x);

    value f  = get_args(GLOBAL__ROOT__[0])[set_add_tag];
    value f0 = LIVEPOINTERS0(tinfo_, call(tinfo_, f, vx));
    value v  = LIVEPOINTERS0(tinfo_, call(tinfo_, f0, getValue()));
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
    value v = LIVEPOINTERS0(tinfo_, call(tinfo_, f, getValue()));
    return value_to_int(v);
}
}

int foo() {
    certicoq::set Z;
    Z.add(5);
    Z.add(4);
    Z.add(200);
    Z.add(20000);
    return Z.size();
}

/*
int main() {

    certicoq::initialize_global_thread_info();

    std::cout << "initialized\n";
    certicoq::set X;
    std::cout << "Created X\n";
    std::cout << "set has size: " << X.size() << "\n";

    certicoq::set Y;
    Y.add(-3);

    for (int i=0; i<1000; i++) {
        std::cout << "Adding " << i << "\n";
        X.add(i);
        std::cout << "set has size: " << X.size() << "\n";
    }

    for (int i=-100; i<20000; i++) {
        std::cout << "Checking membership of " << i << ": " << X.isMember(i) << "\n";
    }

    for (int i=0; i<10000; i++) {
        std::cout << "Calling foo, which created a set of size " << foo() << "\n";
    }
    std::cout << "X has size " << X.size() << "\n";
    std::cout << "Y has size " << Y.size() << "\n";
}
*/