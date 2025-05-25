#ifndef VINCE_ITERATOR_H__
#define VINCE_ITERATOR_H__

#include "type_traits.h"
#include <cstddef>
#include <iterator>
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

// Calculate the distance between iterators

// For input iterator
template <class InputIterator>
typename InputIterator::difference_type __distance( InputIterator first, InputIterator last, input_iterator_tag ) {
    typename InputIterator::difference_type n = 0;
    while ( first != last ) {
        ++first;
        ++n;
    }
    return n;
}

// For random iterator
template <class RandomIterator>
typename RandomIterator::difference_type __distance( RandomIterator first, RandomIterator last,
                                                     random_access_iterator_tag ) {
    return last - first;
}

template <class Iterator>
typename Iterator::difference_type distance( Iterator first, Iterator last ) {
    return __distance( first, last, iterator_category( first ) );
}

// Move the iterator forward by n steps

// For input iterator
template <class InputIterator, class Distance>
inline void __advance( InputIterator i, Distance n, input_iterator_tag ) {
    while ( n-- )
        ++i;
}

template <class BidirectionalIterator, class Distance>
inline void __advance( BidirectionalIterator i, Distance n, bidirectional_iterator_tag ) {
    if ( n >= 0 )
        while ( n-- )
            ++i;
    else
        while ( n++ )
            --i;
}

template <class RandomIterator, class Distance>
inline void __advance( RandomIterator i, Distance n, random_access_iterator_tag ) {
    i += n;
}

template <class Iterator, class Distance>
inline void advance( Iterator i, Distance n ) {
    __advance( i, n, iterator_category( i ) );
}

// Define reverse iterator

template <class Iterator>
class reverse_iterator {
protected:
    Iterator current;

public:
    typedef typename Iterator::iterator_category iterator_category;
    typedef typename Iterator::value_type        value_type;
    typedef typename Iterator::pointer           pointer;
    typedef typename Iterator::reference         reference;
    typedef typename Iterator::difference_type   difference_type;

    typedef Iterator                   iterator_type;
    typedef reverse_iterator<Iterator> Self;

public:
    reverse_iterator() = default;
    explicit reverse_iterator( iterator_type i ) : current( i ) {}
    reverse_iterator( const Self& rhs ) : current( rhs.current ) {}

    iterator_type base() const {
        return current;
    }

    value_type& operator*() {
        iterator_type temp = current;
        return *--temp;
    }

    pointer operator->() const {
        return &( operator*() );
    }

    Self& operator++() {
        --current;
        return *this;
    }

    Self operator++( int ) {
        Self temp = *this;
        --current;
        return temp;
    }

    Self& operator--() {
        ++current;
        return *this;
    }

    Self operator--( int ) {
        Self temp = *this;
        ++current;
        return temp;
    }

    Self operator+( difference_type n ) const {
        return Self( current - n );
    }

    Self& operator+=( difference_type n ) {
        current -= n;
        return *this;
    }

    Self operator-( difference_type n ) const {
        return Self( current + n );
    }

    Self& operator-=( difference_type n ) {
        current += n;
        return *this;
    }

    reference operator[]( difference_type n ) const {
        return *( *this + n );
    }
};

template <class Iterator>
inline bool operator==( const reverse_iterator<Iterator>& x, const reverse_iterator<Iterator>& y ) {
    return x.base() == y.base();
}

template <class Iterator>
inline bool operator!=( const reverse_iterator<Iterator>& x, const reverse_iterator<Iterator>& y ) {
    return !( x == y );
}

template <class Iterator>
inline bool operator<( const reverse_iterator<Iterator>& x, const reverse_iterator<Iterator>& y ) {
    return x.base() < y.base();
}

template <class Iterator>
inline bool operator>( const reverse_iterator<Iterator>& x, const reverse_iterator<Iterator>& y ) {
    return x.base() > y.base();
}

template <class Iterator>
inline bool operator<=( const reverse_iterator<Iterator>& x, const reverse_iterator<Iterator>& y ) {
    return !( x.base() > y.base() );
}

template <class Iterator>
inline bool operator>=( const reverse_iterator<Iterator>& x, const reverse_iterator<Iterator>& y ) {
    return !( x.base() < y.base() );
}

template <class Iterator>
inline typename reverse_iterator<Iterator>::difference_type operator-( const reverse_iterator<Iterator>& x,
                                                                       const reverse_iterator<Iterator>& y ) {
    return y.base() - x.base();
}

template <class Iterator>
inline reverse_iterator<Iterator> operator+( typename reverse_iterator<Iterator>::difference_type n,
                                             const reverse_iterator<Iterator>&                    x ) {
    return reverse_iterator<Iterator>( x.base() - n );
}

}  // namespace vince

#endif