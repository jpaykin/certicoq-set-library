#include <iostream>
#include <set>

extern "C" {
    // Files found in path-to-certicoq/plugin/runtime/
    #include "values.h"
    #include "gc_stack.h"
    #include "prim_int63.h"

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
//value BODY__;
struct stack_frame GLOBAL__FRAME__ = { GLOBAL__ROOT__ + 1, GLOBAL__ROOT__, NULL };

// Set of integers data structure
class set {
    private:
        // the value underlying the set
        value t_value_;

    public:
        value getValue() const { return t_value_; };
        void setValue(value v);

        // Constructors and destructors
        set(); // empty set
        ~set() = default;

        void add(int x);
        bool isMember(int x) const;
        int size(); // should be const, but because we need to reference the pointer t_value_ here, is not
};

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
    value v = get_args(GLOBAL__ROOT__[0])[set_empty_tag];
    setValue(v);
}

////////////
// Setter //
////////////


void initialize_global_thread_info() {
    if (tinfo_ == NULL) {
        tinfo_ = make_tinfo();

        GLOBAL__ROOT__[0] = body(tinfo_);
        tinfo_->fp = &GLOBAL__FRAME__;

        //BEGINFRAME(tinfo_, 1)
        //nalloc=1; GC_SAVE1(tinfo_, b);
        //value b = body(tinfo_);
        //certicoq_modify(tinfo_, &BODY__, b);
        //ENDFRAME
    }
}


void set::setValue(value v) {
    BEGINFRAME(tinfo_, 1)
    //t_value_ = v;
    nalloc=1; GC_SAVE1(tinfo_, v);
    certicoq_modify(tinfo_, &t_value_, v); // from gc_stack.h
    ENDFRAME
}

/////////////////////
// set Operations //
/////////////////////


void set::add(int x) {
    BEGINFRAME(tinfo_, 3)

    value vx = int_to_value(x);

    value f  = get_args(GLOBAL__ROOT__[0])[set_add_tag];
    value f0 = LIVEPOINTERS0(tinfo_, call(tinfo_, f, vx));

    value vs = getValue();
    value v  = LIVEPOINTERS0(tinfo_, call(tinfo_, f0, vs));
    setValue(v);
    ENDFRAME
}

bool set::isMember(int x) const {
    BEGINFRAME(tinfo_, 3)

    value vx = int_to_value(x);

    value f  = get_args(GLOBAL__ROOT__[0])[set_mem_tag];
    value f0 = LIVEPOINTERS0(tinfo_, call(tinfo_, f, vx));
    value v  = LIVEPOINTERS0(tinfo_, call(tinfo_, f0, getValue()));
    return value_to_bool(v);
    ENDFRAME
}

int set::size() {
    BEGINFRAME(tinfo_, 1);
    value f = get_args(GLOBAL__ROOT__[0])[set_cardinal_tag];

    // need to use the actual t_value_ here, not sufficient to use certicoq_mofidy. also can't seem to not track BODY__ here.
    value v = LIVEPOINTERS1(tinfo_, call(tinfo_, f, getValue()), t_value_);
    return value_to_int(v);
    ENDFRAME
}

}

int main() {

    certicoq::initialize_global_thread_info();

    std::cout << "initialized\n";
    certicoq::set X;
    std::cout << "Created X\n";
    std::cout << "set has size: " << X.size() << "\n";

    for (int i=0; i<10000; i++) {
        std::cout << "Adding " << i << "\n";
        X.add(i);
        std::cout << "set has size: " << X.size() << "\n";
    }
    

    for (int i=-100; i<20000; i++) {
        std::cout << "Checking membership of " << i << ": " << X.isMember(i) << "\n";
    }
}