#ifndef WYNE_ITERATOR_H__
#define WYNE_ITERATOR_H__

#include "type_traits.h"
#include <cstddef>
#include <type_traits>

namespace wyne {

// Iterator types
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iteartor_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iteartor_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

// Iterator template
template <class Category, class T, class Distance = ptrdiff_t, class Pointer = T*, class Reference = T&>
struct iterator {
    using iterator_category = Category;
    using value_type        = T;
    using pointer           = Pointer;
    using reference         = Reference;
    using difference_type   = Distance;
};

// Iterator traits: performed only if the iterator defines iterator_category

template <class Iterator, bool>
struct iterator_traits_impl {};

template <class Iterator>
struct iterator_traits_impl<Iterator, true> {
    using iterator_category = typename Iterator::iterator_category;
    using value_type        = typename Iterator::value_type;
    using pointer           = typename Iterator::pointer;
    using reference         = typename Iterator::reference;
    using difference_type   = typename Iterator::difference_type;
};

template <class Iterator, class = void>
struct iterator_traits {};

template <class Iterator>
struct iterator_traits<
    Iterator, std::enable_if_t<std::is_convertible_v<typename Iterator::iterator_category, input_iterator_tag>
                               || std::is_convertible_v<typename Iterator::iterator_category, output_iterator_tag>>>
    : public iterator_traits_impl<Iterator, true> {};

template <class T>
struct iterator_traits<T*> {
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;
    using difference_type   = ptrdiff_t;
};

template <class T>
struct iterator_traits<const T*> {
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using pointer           = const T*;
    using reference         = const T&;
    using difference_type   = ptrdiff_t;
};

// Helper tratis: check if the type is the specific iterator category

template <class Iteartor, class Tag, class = void>
struct iterator_check_helper : _false_type {};

template <class Iterator, class Tag>
struct iterator_check_helper<
    Iterator, Tag, std::enable_if_t<is_derived_from_v<typename iterator_traits<Iterator>::iterator_category, Tag>>>
    : _true_type {};

template <class Iterator>
struct is_input_iterator : public iterator_check_helper<Iterator, input_iterator_tag> {};

template <class Iterator>
struct is_output_iterator
    : public std::bool_constant<
          is_derived_from_v<typename iterator_traits<Iterator>::iterator_category, output_iterator_tag>
          && is_derived_from_v<typename iterator_traits<Iterator>::iterator_category, forward_iteartor_tag>> {};

template <class Iterator>
struct is_forward_iterator : public iterator_check_helper<Iterator, forward_iteartor_tag> {};

template <class Iterator>
struct is_bidirectional_iterator : public iterator_check_helper<Iterator, bidirectional_iterator_tag> {};

template <class Iterator>
struct is_random_access_iterator : public iterator_check_helper<Iterator, random_access_iterator_tag> {};

// TODO: fix: is_input_iterator_t -> _v

template <class Iterator>
inline constexpr bool is_input_iterator_t = is_input_iterator<Iterator>::value;

template <class Iterator>
inline constexpr bool is_output_iterator_t = is_output_iterator<Iterator>::value;

template <class Iterator>
inline constexpr bool is_forward_iterator_t = is_forward_iterator<Iterator>::value;

template <class Iterator>
inline constexpr bool is_bidirectional_iterator_t = is_bidirectional_iterator<Iterator>::value;

template <class Iterator>
inline constexpr bool is_random_access_iterator_t = is_random_access_iterator<Iterator>::value;

template <class Iterator>
inline constexpr bool is_value_trivially_copy_assignable_v =
    std::is_trivially_copy_assignable_v<std::remove_reference_t<decltype( *std::declval<Iterator>() )>>;

template <class Iterator>
inline constexpr bool is_exactly_input_iterator_t = is_input_iterator_t<Iterator> && !is_forward_iterator_t<Iterator>;

template <class Iterator>
inline constexpr bool is_exactly_forward_iterator_t =
    is_forward_iterator_t<Iterator> && !is_bidirectional_iterator_t<Iterator>;

template <class Iterator>
inline constexpr bool is_exactly_bidirectional_iterator_t =
    is_bidirectional_iterator_t<Iterator> && !is_random_access_iterator_t<Iterator>;

template <class Iterator>
inline constexpr bool is_exactly_random_access_iterator_t = is_random_access_iterator<Iterator>::value;

// Extracts the various properties of an iterator

template <class Iterator>
typename iterator_traits<Iterator>::iterator_category iterator_category( const Iterator& ) {
    using _Category = typename iterator_traits<Iterator>::iterator_category;
    return _Category();
}

template <class Iterator>
typename iterator_traits<Iterator>::difference_type* distance_type( const Iterator& ) {
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
    return wyne::__distance( first, last, iterator_category( first ) );
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
    wyne::__advance( i, n, iterator_category( i ) );
}

// Define reverse iterator

template <class Iterator>
class reverse_iterator {
protected:
    Iterator current;

public:
    using iterator_category = typename iterator_traits<Iterator>::iterator_category;
    using value_type        = typename iterator_traits<Iterator>::value_type;
    using pointer           = typename iterator_traits<Iterator>::pointer;
    using reference         = typename iterator_traits<Iterator>::reference;
    using difference_type   = typename iterator_traits<Iterator>::difference_type;

    using iterator_type = Iterator;
    using Self          = reverse_iterator<Iterator>;

public:
    reverse_iterator() = default;
    explicit reverse_iterator( iterator_type i ) : current( i ) {}
    reverse_iterator( const Self& rhs ) : current( rhs.current ) {}

    iterator_type base() const { return current; }

    value_type& operator*() {
        iterator_type temp = current;
        return *--temp;
    }

    pointer operator->() const { return &( operator*() ); }

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

    Self operator+( difference_type n ) const { return Self( current - n ); }

    Self& operator+=( difference_type n ) {
        current -= n;
        return *this;
    }

    Self operator-( difference_type n ) const { return Self( current + n ); }

    Self& operator-=( difference_type n ) {
        current += n;
        return *this;
    }

    reference operator[]( difference_type n ) const { return *( *this + n ); }
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

}  // namespace wyne

#endif