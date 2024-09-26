#ifndef COQ_N_H
#define COQ_N_H

#include "CoqOps.h"

namespace certicoq {

enum positive_tag {XI_tag, XO_tag, XH_tag};
enum N_tag {N0_tag, Npos_tag};

// Data type for natural numbers N
class N : public CoqObject {
    public:
        N() : CoqObject() {};
        N(const N& n) : CoqObject(n) {};
        ~N() = default;
        // This class uses static functions from_value and from_unsigned instead
        // of constructors, because the types would otherwise not be able to
        // distinguish between the two constructors, and we don't want to
        // prioritize one over the other.
        static N from_value(value v);
        static N from_unsigned(unsigned int n);
        unsigned int to_unsigned() const;

        friend std::ostream& operator<<(std::ostream& os, const N& n) {
            os << n.to_unsigned();
            return os;
        }
        static value unsigned_to_positive(struct thread_info *, unsigned int);
        static value unsigned_to_N(struct thread_info *, unsigned int);
        static value int_to_N(struct thread_info *, int);
        static unsigned int positive_to_unsigned(value);
        static unsigned int N_to_unsigned(value);
};


}



#endif // COQ_N_H