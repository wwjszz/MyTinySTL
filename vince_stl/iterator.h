#ifndef VINCE_ITERATOR_H__
#define VINCE_ITERATOR_H__

#include "type_traits.h"
#include <cstddef>
#include <type_traits>

namespace vince {

// Iterator types
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iteartor_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iteartor_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

// Iterator template
template <class Category, class T, class Distance = ptrdiff_t, class Pointer = T*, class Reference = T&>
struct iterator {
    typedef Category  iterator_categoty;
    typedef T         value_type;
    typedef Pointer   pointer;
    typedef Reference reference;
    typedef Distance  difference_type;
};

// Iterator traits: performed only if the iterator defines iterator_category

template <class Iterator, class = void>
struct iterator_traits_impl {};

template <class Iterator>
struct iterator_traits_impl<Iterator, typename Iterator::iterator_category> {
    typedef typename Iterator::iterator_category iterator_category;
    typedef typename Iterator::value_type        value_type;
    typedef typename Iterator::pointer           pointer;
    typedef typename Iterator::reference         reference;
    typedef typename Iterator::difference_type   difference_type;
};

template <class Iterator>
struct iterator_traits : public iterator_traits_impl<Iterator> {};

template <class T>
struct iterator_traits<T*> {
    typedef random_access_iterator_tag iterator_categoty;
    typedef T                          value_type;
    typedef T*                         pointer;
    typedef T&                         reference;
    typedef ptrdiff_t                  difference_type;
};

template <class T>
struct iterator_traits<const T*> {
    typedef random_access_iterator_tag iterator_categoty;
    typedef T                          value_type;
    typedef const T*                   pointer;
    typedef const T&                   reference;
    typedef ptrdiff_t                  difference_type;
};

// Helper tratis: check if the type is the specific iterator category

template <class Iterator, class Tag, class = void>
struct iterator_check_helper : public _false_type {};

template <class Iterator, class Tag>
struct iterator_check_helper<Iterator, Tag,
                             std::enable_if_t<std::is_convertible_v<typename Iterator::iterator_category, Tag>>>
    : public _true_type {};

template <class Iterator>
struct is_input_iterator : public iterator_check_helper<Iterator, input_iterator_tag> {};

template <class Iterator>
struct is_output_iterator : public iterator_check_helper<Iterator, output_iterator_tag> {};

template <class Iterator>
struct is_forward_iterator : public iterator_check_helper<Iterator, forward_iteartor_tag> {};

template <class Iterator>
struct is_bidirectional_iterator : public iterator_check_helper<Iterator, bidirectional_iterator_tag> {};

template <class Iterator>
struct is_random_access_iterator : public iterator_check_helper<Iterator, random_access_iterator_tag> {};

// Extracts the various properties of an iterator

template <class Iterator>
typename iterator_traits<Iterator>::iterator_category iterator_category( const Iterator& ) {
    typedef typename iterator_traits<Iterator>::iterator_category _Category;
    return _Category();
}

template <class Iterator>
typename iterator_traits<Iterator>::difference_type* difference_type( const Iterator& ) {
    return static_cast<typename iterator_traits<Iterator>::difference_type*>( 0 );
}

template <class Iterator>
typename iterator_traits<Iterator>::value_type* value_type( const Iterator& ) {
    return static_cast<typename iterator_traits<Iterator>::value_type*>( 0 );
}

}  // namespace vince

#endif