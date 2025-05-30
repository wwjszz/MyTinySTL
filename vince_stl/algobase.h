#ifndef VINCE_ALGOBASE_H__
#define VINCE_ALGOBASE_H__

#include <cstring>
#include <type_traits>

#include "iterator.h"
#include "type_traits.h"
#include "util.h"

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
inline constexpr const T& max( const T& a, const T& b ) {
    return a > b ? a : b;
}

template <class T>
inline constexpr const T& min( const T& a, const T& b ) {
    return a < b ? a : b;
}

template <class T, class Comp>
inline constexpr const T& max( const T& a, const T& b, Comp comp ) {
    return comp( b, a ) ? a : b;
}

template <class T, class Comp>
inline constexpr const T& min( const T& a, const T& b, Comp comp ) {
    return comp( a, b ) ? a : b;
}

// iter_swap

template <class ForwardIterator1, class ForwardIterator2>
inline constexpr void iter_swap( ForwardIterator1 a, ForwardIterator2 b ) noexcept(
    noexcept( swap( *std::declval<ForwardIterator1>(), *std::declval<ForwardIterator2>() ) ) ) {
    swap( *a, *b );
}

// copy

template <class InputIterator, class OutputIterator, class Distance>
inline constexpr OutputIterator __copy( InputIterator first, InputIterator last, OutputIterator result,
                                        input_iterator_tag, Distance* ) {
    for ( ; first != last; ++first, ++result )
        *result = *first;
    return result;
}

template <class InputIterator, class OutputIterator, class Distance>
inline constexpr OutputIterator __copy( InputIterator first, InputIterator last, OutputIterator result,
                                        random_access_iterator_tag, Distance* ) {
    for ( Distance n = last - first; n > 0; --n ) {
        *result = *first;
        ++first;
        ++result;
    }
    return result;
}

template <class Tp>
inline Tp* __copy_trivial( Tp const* first, Tp const* last, Tp* result ) {
    std::memmove( result, first, sizeof( Tp ) * ( last - first ) );
    return result + ( last - first );
}

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator __copy_aux( InputIterator first, InputIterator last, OutputIterator result ) {
    return __copy( first, last, result, iterator_category( first ), distance_type( first ) );
}

template <class Tp>
inline constexpr std::enable_if_t<std::is_trivially_copy_assignable_v<Tp>, Tp*> __copy_aux( Tp* first, Tp* last,
                                                                                            Tp* result ) {
    return __copy_trivial( first, last, result );
}

template <class Tp>
inline constexpr std::enable_if_t<std::is_trivially_copy_assignable_v<Tp>, Tp*>
__copy_aux( Tp const* first, Tp const* last, Tp* result ) {
    return __copy_trivial( first, last, result );
}

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator copy( InputIterator first, InputIterator last, OutputIterator result ) {
    return __copy_aux( first, last, result );
}

// copy_backward

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iterator_t<InputIterator>, int> = 0>
inline constexpr OutputIterator __copy_backward( InputIterator first, InputIterator last, OutputIterator result,
                                                 input_iterator_tag, Distance* ) {
    while ( last != first )
        *--result = *--last;
    return result;
}

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iterator_t<InputIterator>, int> = 0>
inline constexpr OutputIterator __copy_backward( InputIterator first, InputIterator last, OutputIterator result,
                                                 random_access_iterator_tag, Distance* ) {
    for ( Distance n = last - first; n > 0; --n )
        *--result = *--last;
    return result;
}

template <class Tp>
inline Tp* __copy_backward_trivial( Tp const* first, Tp const* last, Tp* result ) {
    const size_t n = last - first;
    result -= n;

    std::memmove( result, first, sizeof( Tp ) * n );
    return result;
}

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator __copy_backward_aux( InputIterator first, InputIterator last, OutputIterator result ) {
    return __copy_backward( first, last, result, iterator_category( first ), distance_type( first ) );
}

template <class Tp>
inline constexpr std::enable_if_t<std::is_trivially_copy_assignable_v<Tp>, Tp*>
__copy_backward_aux( Tp* first, Tp* last, Tp* result ) {
    return __copy_backward_trivial( first, last, result );
}

template <class Tp>
inline constexpr std::enable_if_t<std::is_trivially_copy_assignable_v<Tp>, Tp*>
__copy_backward_aux( Tp const* first, Tp const* last, Tp* result ) {
    return __copy_backward_trivial( first, last, result );
}

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator copy_backward( InputIterator first, InputIterator last, OutputIterator result ) {
    return __copy_backward_aux( first, last, result );
}

// copy_if

template <class InputIterator, class OutputIterator, class Predicate,
          std::enable_if_t<is_input_iterator_t<InputIterator>, int> = 0>
inline constexpr OutputIterator copy_if( InputIterator first, InputIterator last, OutputIterator result,
                                         Predicate pred ) {
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
          std::enable_if_t<is_input_iterator_t<InputIterator> && !is_random_access_iterator_t<InputIterator>, int> = 0>
inline constexpr OutputIterator copy_n( InputIterator first, Size n, OutputIterator result ) {
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
inline constexpr OutputIterator copy_n( InputIterator first, Size n, OutputIterator result ) {
    typedef typename iterator_traits<InputIterator>::difference_type difference_type;
    return copy( first, first + difference_type( n ), result );
}

// move

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iterator_t<InputIterator>, int> = 0>
inline constexpr OutputIterator __move( InputIterator first, InputIterator last, OutputIterator result,
                                        input_iterator_tag, Distance* ) {
    for ( ; first != last; ++first, ++result )
        *result = vince::move( *first );
    return result;
}

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iterator_t<InputIterator>, int> = 0>
inline constexpr OutputIterator __move( InputIterator first, InputIterator last, OutputIterator result,
                                        random_access_iterator_tag, Distance* ) {
    for ( Distance n = last - first; n > 0; --n ) {
        *result = vince::move( *first );
        ++first;
        ++result;
    }
    return result;
}

template <class Tp>
inline Tp* __move_trivial( Tp const* first, Tp const* last, Tp* result ) {
    std::memmove( result, first, sizeof( Tp ) * ( last - first ) );
    return result + ( last - first );
}

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator __move_aux( InputIterator first, InputIterator last, OutputIterator result ) {
    return __move( first, last, result, iterator_category( first ), distance_type( first ) );
}

template <class Tp>
inline constexpr std::enable_if_t<std::is_trivially_copy_assignable_v<Tp>, Tp*> __move_aux( Tp* first, Tp* last,
                                                                                            Tp* result ) {
    return __move_trivial( first, last, result );
}

template <class Tp>
inline constexpr std::enable_if_t<std::is_trivially_copy_assignable_v<Tp>, Tp*>
__move_aux( Tp const* first, Tp const* last, Tp* result ) {
    return __move_trivial( first, last, result );
}

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator move( InputIterator first, InputIterator last, OutputIterator result ) {
    return __move_aux( first, last, result );
}

// equal

struct equal_to {
    template <class T1, class T2>
    constexpr bool operator()( const T1& x, const T2& y ) const {
        return x == y;
    }
};

template <class InputIterator1, class InputIterator2, class Pred,
          std::enable_if_t<vince::is_satisfied_v<is_input_iterator, InputIterator1, InputIterator2>, int> = 0>
inline constexpr bool equal( InputIterator1 first1, InputIterator1 last1, InputIterator1 first2, Pred pred ) {
    for ( ; first1 != last1; ++first1, ++first2 )
        if ( !pred( first1, first2 ) )
            return false;
    return true;
}

template <class InputIterator1, class InputIterator2,
          std::enable_if_t<vince::is_satisfied_v<is_input_iterator, InputIterator1, InputIterator2>, int> = 0>
inline constexpr bool equal( InputIterator1 first1, InputIterator1 last1, InputIterator1 first2 ) {
    return vince::equal( first1, last1, first2, equal_to() );
}

// move_backward

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iterator_t<InputIterator>, int> = 0>
inline constexpr OutputIterator __move_backward( InputIterator first, InputIterator last, OutputIterator result,
                                                 input_iterator_tag, Distance* ) {
    while ( last != first )
        *--result = vince::move( *--last );
    return result;
}

template <class InputIterator, class OutputIterator, class Distance,
          std::enable_if_t<is_input_iterator_t<InputIterator>, int> = 0>
inline constexpr OutputIterator __move_backward( InputIterator first, InputIterator last, OutputIterator result,
                                                 random_access_iterator_tag, Distance* ) {
    for ( Distance n = last - first; n > 0; --n )
        *--result = vince::move( *--last );
    return result;
}

template <class Tp>
inline Tp* __move_backward_trivial( Tp const* first, Tp const* last, Tp* result ) {
    const size_t n = last - first;
    result -= n;

    std::memmove( result, first, sizeof( Tp ) * n );
    return result;
}

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator __move_backward_aux( InputIterator first, InputIterator last, OutputIterator result ) {
    return __move_backward( first, last, result, iterator_category( first ), distance_type( first ) );
}

template <class Tp>
inline constexpr std::enable_if_t<std::is_trivially_copy_assignable_v<Tp>, Tp*>
__move_backward_aux( Tp* first, Tp* last, Tp* result ) {
    return __move_backward_trivial( first, last, result );
}

template <class Tp>
inline constexpr std::enable_if_t<std::is_trivially_copy_assignable_v<Tp>, Tp*>
__move_backward_aux( Tp const* first, Tp const* last, Tp* result ) {
    return __move_backward_trivial( first, last, result );
}

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator move_backward( InputIterator first, InputIterator last, OutputIterator result ) {
    return __move_backward_aux( first, last, result );
}

}  // namespace vince

#endif