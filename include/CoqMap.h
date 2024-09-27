#ifndef COQ_MAP_H
#define COQ_MAP_H

#include "CoqOps.h"
#include "CoqList.h"

#include <optional>

extern "C"
{
    extern void print_Coq_Init_Datatypes_bool(value);
    extern void print_Coq_Init_Datatypes_nat(value);
    extern void print_Coq_Init_Datatypes_option(value, void (*)(value));
    extern void print_Coq_Init_Datatypes_list(value, void (*)(value));
}

namespace certicoq
{

    enum MapOpsTag
    {
        map_empty_tag,
        map_is_empty_tag,
        map_add_tag,
        map_find_tag,
        map_remove_tag,
        map_mem_tag,
        map_equal_tag,
        map_elements_tag,
        map_size_tag
    };

    template <class T>
    class intmap : public CoqObject
    {
    private:
        static value getBody()
        {
            return applyTag(ThreadInfo::getBody(), MapBodyTag);
        }
        template <typename... Values>
        static value applyMapTag(MapOpsTag op, Values... vals)
        {
            return applyTag(getBody(), op, vals...);
        }

    public:
        intmap();
        intmap(value v) : CoqObject(v) {};
        intmap(const intmap &m) : CoqObject(m) {};
        ~intmap() = default;

        bool isEmpty() const;
        void add(int key, T value);
        std::optional<T> find(int key) const;
        void remove(int key);
        bool mem(int key) const;
        bool operator==(const intmap &) const;
        int size() const;
        list<std::pair<int, T>> elems() const;
    };

    ///////////////////
    // Instantiation //
    ///////////////////

    // Constructor
    template <typename T>
    intmap<T>::intmap() : CoqObject()
    {
        setValue(applyMapTag(map_empty_tag));
    }

    template <typename T>
    bool intmap<T>::isEmpty() const
    {
        return fromValue<bool>(applyMapTag(map_is_empty_tag, getValue()));
    }

    template <typename T>
    void intmap<T>::add(int key, T val)
    {
        setValue(applyMapTag(map_add_tag,
                             toValue(key),
                             toValue(val),
                             getValue()));
    }

    template <typename T>
    std::optional<T> intmap<T>::find(int key) const
    {
        return fromOption<T>(applyMapTag(map_find_tag,
                                         toValue(key),
                                         getValue()));
    }

    template <typename T>
    void intmap<T>::remove(int key)
    {
        setValue(applyMapTag(map_remove_tag, toValue(key), getValue()));
    }

    template <typename T>
    bool intmap<T>::mem(int key) const {
        return fromValue<bool>(applyMapTag(map_mem_tag,
                                           toValue(key),
                                           getValue()
        ));
    }

    template<typename T>
    int intmap<T>::size() const {
        return fromValue<int>(applyMapTag(map_size_tag,
                                            getValue()));
    }

    // TODO: I could do this if fromValue/toValue were a class rather than a value, I think
    /*
    template<typename T1, typename T2>
    std::pair<T1, T2> fromValue<std::pair<T1, T2>>(value v) {
        value v1 = get_args(v)[0];
        value v2 = get_args(v)[1];
        return std::make_pair(fromValue<T1>(v1), fromValue<T2>(v2));
    }

    template<typename T>
    list<std::pair<int,T>> intmap<T>::elems() const {
        return list<std::pair<int,T>>(applyMapTag(map_elements_tag, getValue()));

    }
    */

    // template<typename T>
    // bool intmap<T>::operator==(const intmap<T>& other) {
    //     // For equality, we need a value corresponding to Coq type (T -> T -> bool)
            // Not sure how to do that. Could construct a closure (defined in glue.c) with 
            // great difficulty.

    //     return fromValue<bool>(applyMapTag(map_equal_tag,
    //                                        _
    //                                        getValue(),
    //                                        other.getValue()
    //     ));
    // }

} // namespace certicoq

#endif // COQ_MAP_H