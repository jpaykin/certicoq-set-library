#ifndef COQ_LIST_H
#define COQ_LIST_H

#include <set>
#include "CoqOps.h"

extern "C" {
    // List-specific declarations from glue code
    extern value make_Coq_Init_Datatypes_list_nil(void);
    extern value make_Coq_Init_Datatypes_list_cons(value, value, value *);
    extern value alloc_make_Coq_Init_Datatypes_list_cons(struct thread_info *, value, value);
    extern unsigned int get_Coq_Init_Datatypes_list_tag(value);
}

namespace certicoq {

enum ListTag {NIL, CONS};
template <typename T> // Assume there is an instance of ValueRepr<T>
class list : public CoqObject {
    public:
        // empty list constructor
        list();
        list(value v) : CoqObject(v) {};
        list(const list<T>& l) : CoqObject(l) {};
        list(std::vector<T>& vec);
        ~list() { freeNode(); };

        void cons(T x);
        std::vector<T> toVector() const;
        std::set<T> toSet() const;
        T head() const;
        list<T> tail() const;

        friend std::ostream& operator<<(std::ostream& os, const list<T>& ls) {
            std::vector<T> elems = ls.toVector();

            os << "[";
            auto it = elems.begin();
            if (it != elems.end()) {
                os << *it;
                ++it;
                for (auto it2=it; it2!=elems.end(); ++it2) {
                    os << "; " << *it2;
                }
            }
            os << "]";
            return os;
        }

        // Only a forward iterator, cannot go backwards
        /*
        class Iterator {
            private:
                T iter_T_;
                void set_iter_T_() {
                    // Update iter_T_ so it stores the head of the value iter_
                    iter_T_ = toValue<T>(get_args(*iter_)[0]);
                }

            public:

                value* iter_;
                Iterator(value* v) : iter_(v) { set_iter_T_(); }

                Iterator& operator++() { // increment (prefix)
                    if (iter_) iter_ = &(get_args(*iter_)[1]); // get the tail
                    set_iter_T_();
                    return *this;
                }
                
                // Iterator& operator++(int) { // increment (postfix)
                //     Iterator temp = *this;
                //     ++(*this);
                //     return temp;
                // }

                // dereference
                T& operator*() {
                    return iter_T_;
                }

                // equality comparison
                bool operator==(const Iterator& other) const {
                    return (iter_ == other.iter_);
                }

                bool operator!=(const Iterator& other) const {
                    return !(*this == other);
                }

        };

        Iterator begin() {
            return Iterator(value_);
        }

        Iterator end() {
            return Iterator(nullptr);
        }
        */
};

template <typename T>
list<T>::list() : CoqObject() {
    setValue(make_Coq_Init_Datatypes_list_nil());
};
template <typename T>
list<T>::list(std::vector<T>& vec) {
    initializeNode();
    value v_l = make_Coq_Init_Datatypes_list_nil();
    // Iterate backwards through the list to cons things in the right order
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
        value hd = toValue<T>(*it);
        value tl = v_l;
        v_l = alloc_make_Coq_Init_Datatypes_list_cons(
                    ThreadInfo::getTInfo(), hd, tl
        );
    }
    setValue(v_l);
}
template <typename T> void list<T>::cons(T x) {
    value hd = toValue<T>(x);
    value tl = getValue();
    value new_list = alloc_make_Coq_Init_Datatypes_list_cons(
                        ThreadInfo::getTInfo(), hd, tl
    );
    setValue(new_list);
}
template <typename T> 
std::vector<T> list<T>::toVector() const {
    value x = getValue();
    std::vector<T> vec;
    while(get_Coq_Init_Datatypes_list_tag(x) != NIL) {
        value* x_args   = get_args(x);
        T hd            = fromValue<T>(x_args[0]);
        value x_tail    = x_args[1];
        vec.push_back(hd);
        x = x_tail;
    }
    return vec;
}
template <typename T> 
std::set<T> list<T>::toSet() const {
    value x = getValue();
    std::set<T> X;
    while(get_Coq_Init_Datatypes_list_tag(x) != NIL) {
        value* x_args   = get_args(x);
        T hd            = fromValue<T>(x_args[0]);
        value x_tail    = x_args[1];
        X.insert(hd);
        x = x_tail;
    }
    return X;
}
template <typename T> T list<T>::head() const {
    value v = getValue();
    return fromValue<T>(get_args(v)[0]);
}
template <typename T> list<T> list<T>::tail() const {
    value v = getValue();
    return list<T>(get_args(v)[1]);
}

}

#endif // COQ_LIST_H