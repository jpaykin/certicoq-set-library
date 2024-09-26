#include <N.h>

extern "C" {
    extern unsigned int get_Coq_Numbers_BinNums_positive_tag(value);
    extern value make_Coq_Numbers_BinNums_positive_xH(void);
    extern value alloc_make_Coq_Numbers_BinNums_positive_xO(struct thread_info *, value);
    extern value alloc_make_Coq_Numbers_BinNums_positive_xI(struct thread_info *, value);
    extern unsigned int get_Coq_Numbers_BinNums_N_tag(value);
    extern value make_Coq_Numbers_BinNums_N_N0(void);
    extern value alloc_make_Coq_Numbers_BinNums_N_Npos(struct thread_info *, value);
}

namespace certicoq {

value N::unsigned_to_positive(struct thread_info* tinfo, unsigned int n) {
    //assert(n > 0);
    value pos;
    if (n <= 1) {
        pos = make_Coq_Numbers_BinNums_positive_xH();
    } else if ((n & 1) == 0) {
        value v = unsigned_to_positive(tinfo, n >> 1);
        pos = alloc_make_Coq_Numbers_BinNums_positive_xO(tinfo, v);
    } else if ((n & 1) == 1) {
        value v = unsigned_to_positive(tinfo, n >> 1);
        pos = alloc_make_Coq_Numbers_BinNums_positive_xI(tinfo, v);
    }
    return pos;
}
value N::unsigned_to_N(struct thread_info* tinfo, unsigned int n) {
    value v_n;
    if (n == 0) {
        v_n = make_Coq_Numbers_BinNums_N_N0();
    } else {
        value v = unsigned_to_positive(tinfo, n);
        v_n = alloc_make_Coq_Numbers_BinNums_N_Npos(tinfo, v);
    }
    return v_n;
}


value N::int_to_N(struct thread_info* tinfo, int n) {
    //assert (n >= 0);
    return N::unsigned_to_N(tinfo, (unsigned) n);
}

unsigned int N::positive_to_unsigned(value n) {
    unsigned int ctr_tag = get_Coq_Numbers_BinNums_positive_tag(n);
    if (ctr_tag == XH_tag) {
        return 1;
    } else if (ctr_tag == XO_tag) {
        value n_rec = get_args(n)[0];
        return positive_to_unsigned(n_rec) << 1;
    } else if (ctr_tag == XI_tag) {
        value n_rec = get_args(n)[0];
        return (positive_to_unsigned(n_rec) << 1) + 1;
    }
    return 0;
}
unsigned int N::N_to_unsigned(value n) {
    unsigned int ctr_tag = get_Coq_Numbers_BinNums_N_tag(n);
    if (ctr_tag == N0_tag) {
        return 0;
    } else {
        value n_rec = get_args(n)[0];
        return (positive_to_unsigned(n_rec));
    }
    return 0;
}


N N::from_value(value v) {
    N n;
    n.setValue(v);
    return n;
}

N N::from_unsigned(unsigned int n) {
    value v = N::unsigned_to_N(ThreadInfo::getTInfo(), n);
    return N::from_value(v);
}

unsigned int N::to_unsigned() const {
    return N::N_to_unsigned(getValue());
}


// specialization to unsigned int
template<> value toValue<unsigned int>(unsigned int x) {
    return N::from_unsigned(x).getValue();
}
template<> unsigned int fromValue<unsigned int>(value v) {
    return N::from_value(v).to_unsigned();
}

}