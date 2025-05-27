#ifndef VINCE_ALGOBASE_H__
#define VINCE_ALGOBASE_H__

#include "iterator.h"
#include "util.h"
#include <algorithm>
#include <cstring>
#include <type_traits>

#define STRINGIFY( x ) #x
#define COMPILER_MESSAGE( msg ) _Pragma( STRINGIFY( GCC warning msg ) )

#ifdef max
COMPILER_MESSAGE( "#undefing macro max" )
#undef max
#endif

#ifdef min
COMPILER_MESSAGE( "#undefing macro min" )
#undef min
#endif

namespace vince {

// max min

template <class T>
inline const T& max( const T& a, const T& b ) {
    return a > b ? a : b;
}

template <class T>
inline const T& min( const T& a, const T& b ) {
    return a < b ? a : b;
}

template <class T, class Comp>
inline const T& max( const T& a, const T& b, Comp comp ) {
    return comp( b, a ) ? a : b;
}

template <class T, class Comp>
inline const T& min( const T& a, const T& b, Comp comp ) {
    return comp( a, b ) ? a : b;
}

// iter_swap

template <class ForwardIterator1, class ForwardIterator2>
inline void iter_swap( ForwardIterator1 a,
                       ForwardIterator2 b ) noexcept( noexcept( swap( *std::declval<ForwardIterator1>(),
                                                                      *std::declval<ForwardIterator2>() ) ) ) {
    swap( *a, *b );
}

// copy

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iteraotr_t<InputIterator>, int> = 0>
inline OutputIterator __copy( InputIterator first, InputIterator last, OutputIterator result, input_iterator_tag,
                              Distance* ) {
    for ( ; first != last; ++first, ++result )
        *result = *first;
    return result;
}

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iteraotr_t<InputIterator>, int> = 0>
inline OutputIterator __copy( InputIterator first, InputIterator last, OutputIterator result,
                              random_access_iterator_tag, Distance* ) {
    for ( Distance n = last - first; n > 0; --n ) {
        *result = *first;
        ++first;
        ++result;
    }
    return result;
}

template <class T>
inline T* __copy( T* first, T* last, T* result ) {
    memmove( result, first, sizeof( T ) * ( last - first ) );
    return result + ( last - first );
}

template <class InputIterator, class OutputIterator, std::enable_if_t<is_input_iteraotr_t<InputIterator>, int> = 0>
inline OutputIterator copy( InputIterator first, InputIterator last, OutputIterator result ) {
    return __copy( first, last, result, iterator_category( first ), distance_type( first ) );
}

template <class T>
inline T* copy( T* first, T* last, T* result ) {
    return __copy( first, last, result );
}

// copy_backward

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iteraotr_t<InputIterator>, int> = 0>
inline OutputIterator __copy_backward( InputIterator first, InputIterator last, OutputIterator result,
                                       input_iterator_tag, Distance* ) {
    while ( last != first )
        *--result = *--last;
    return result;
}

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iteraotr_t<InputIterator>, int> = 0>
inline OutputIterator __copy_backward( InputIterator first, InputIterator last, OutputIterator result,
                                       random_access_iterator_tag, Distance* ) {
    for ( Distance n = last - first; n > 0; --n )
        *--result = *--last;
    return result;
}

template <class T>
inline T* __copy_backward( T* first, T* last, T* result ) {
    const size_t n = last - first;
    result -= n;

    memmove( result, first, sizeof( T ) * n );
    return result;
}

template <class InputIterator, class OutputIterator, std::enable_if_t<is_input_iteraotr_t<InputIterator>, int> = 0>
inline OutputIterator copy_backward( InputIterator first, InputIterator last, OutputIterator result ) {
    return __copy_backward( first, last, result, iterator_category( first ), distance_type( first ) );
}

template <class T>
inline T* copy_backward( T* first, T* last, T* result ) {
    return __copy_backward( first, last, result );
}

// copy_if

template <class InputIterator, class OutputIterator, class Predicate>
inline OutputIterator copy_if( InputIterator first, InputIterator last, OutputIterator result, Predicate pred ) {
    for ( ; first != last; ++first ) {
        if ( pred( *first ) ) {
            *result = *last;
            ++result;
        }
    }
    return result;
}

// copy_n

template <class InputIterator, class Size, class OutputIterator,
          std::enable_if_t<is_input_iteraotr_t<InputIterator> && !is_random_access_iterator_t<InputIterator>, int> = 0>
inline OutputIterator copy_n( InputIterator first, Size n, OutputIterator result ) {
    if ( n > 0 ) {
        *result = *first;
        ++result;
        for ( --n; n > 0; --n ) {
            ++first;
            *result = *first;
            ++result;
        }
    }
    return result;
}

template <class InputIterator, class Size, class OutputIterator,
          std::enable_if_t<is_random_access_iterator_t<InputIterator>, int> = 0>
inline OutputIterator copy_n( InputIterator first, Size n, OutputIterator result ) {
    typedef typename iterator_traits<InputIterator>::difference_type difference_type;
    return copy( first, first + difference_type( n ), result );
}

}  // namespace vince

#endif