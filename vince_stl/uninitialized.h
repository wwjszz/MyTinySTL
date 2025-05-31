#ifndef VINCE_UNINITIALIZED_H__
#define VINCE_UNINITIALIZED_H__

#include "algobase.h"
#include "construct.h"
#include "iterator.h"
#include <type_traits>

namespace vince {

// uninitialized_copy

template <class InputIterator, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_copy( InputIterator first, InputIterator last,
                                                       ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; first != last; ++first, ++cur )
            vince::construct_at( cur, *first );
    }
    catch ( ... ) {
        vince::destroy( result, cur );
        throw;
    }
    return cur;
}

template <class Tp, class Up, std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_copy( Tp* first, Tp* last, Up* result ) {
    return vince::__copy_trivial( first, last, result );
}

template <class Tp, class Up, std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_copy( Tp const* first, Tp const* last, Up* result ) {
    return vince::__copy_trivial( first, last, result );
}

template <class InputIterator, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_copy( InputIterator first, InputIterator last, ForwardIterator result ) {
    return vince::__uninitialized_copy( vince::move( first ), vince::move( last ), vince::move( result ) );
}

// uninitialized_copy_n

template <class InputIterator, class Size, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_copy_n( InputIterator first, Size n, ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; n > 0; --n, ++cur, ++first )
            vince::construct_at( &*cur, *first );
    }
    catch ( ... ) {
        vince::destroy( result, cur );
        throw;
    }
    return cur;
}

template <class InputIterator, class Size, class ForwardIterator,
          std::enable_if_t<is_random_access_iterator_t<InputIterator>>>
inline constexpr ForwardIterator __uninitialized_copy_n( InputIterator first, Size n, ForwardIterator result ) {
    return vince::uninitialized_copy( first, first + n, vince::move( result ) );
}

template <class InputIterator, class Size, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_copy_n( InputIterator first, Size n, ForwardIterator result ) {
    return vince::__uninitialized_copy_n( vince::move( first ), vince::move( n ), vince::move( result ) );
}

// uninitialized_fill_n

template <class ForwardIterator, class Size, class Tp>
inline constexpr ForwardIterator __uninitialized_fill_n( ForwardIterator first, Size n, const Tp& value ) {
    ForwardIterator cur = first;
    try {
        for ( ; n > 0; --n, ++cur )
            vince::construct_at( &*cur, value );
    }
    catch ( ... ) {
        vince::destroy( first, cur );
        throw;
    }
    return cur;
}

template <class Tp, class Size, class Up>
inline constexpr std::enable_if_t<sizeof( Up ) == 1 && sizeof( Tp ) == 1
                                      && std::is_integral_v<Tp> && std::is_integral_v<Up> && !std::is_same_v<Tp, bool>,
                                  Tp*>
__uninitialized_fill_n( Tp* first, Size n, const Up& value ) {
    return vince::__fill_n( first, vince::move( n ), value );
}

template <class ForwardIterator, class Size, class Tp>
inline constexpr ForwardIterator uninitialized_fill_n( ForwardIterator first, Size n, const Tp& value ) {
    return vince::__uninitialized_fill_n( first, vince::move( n ), value );
}

// uninitialized_fill

template <class ForwardIterator, class Tp, class = void>
inline constexpr ForwardIterator __uninitialized_fill( ForwardIterator first, ForwardIterator last, const Tp& value ) {
    ForwardIterator cur = first;
    try {
        for ( ; cur != last; ++cur )
            vince::construct_at( &*cur, value );
    }
    catch ( ... ) {
        vince::destroy( first, cur );
        throw;
    }
    return cur;
}

template <class ForwardIterator, class Tp, std::enable_if_t<is_random_access_iterator_t<ForwardIterator>>>
inline constexpr ForwardIterator __uninitialized_fill( ForwardIterator first, ForwardIterator last, const Tp& value ) {
    return vince::uninitialized_fill_n( first, last - first, value );
}

template <class ForwardIterator, class Tp>
inline constexpr ForwardIterator uninitialized_fill( ForwardIterator first, ForwardIterator last, const Tp& value ) {
    return vince::__uninitialized_fill( vince::move( first ), vince::move( last ), value );
}

// uninitialized_move

template <class InputIterator, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_move( InputIterator first, InputIterator last,
                                                       ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; first != last; ++first, ++cur )
            vince::construct_at( cur, vince::move( *first ) );
    }
    catch ( ... ) {
        vince::destroy( result, cur );
        throw;
    }
    return cur;
}

template <class Tp, class Up, std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_move( Tp* first, Tp* last, Up* result ) {
    return vince::__move_trivial( first, last, result );
}

template <class Tp, class Up, std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_move( Tp const* first, Tp const* last, Up* result ) {
    return vince::__move_trivial( first, last, result );
}

template <class InputIterator, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_move( InputIterator first, InputIterator last, ForwardIterator result ) {
    return vince::__uninitialized_move( vince::move( first ), vince::move( last ), vince::move( result ) );
}

// uninitialized_move_n

template <class InputIterator, class Size, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_move_n( InputIterator first, Size n, ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; n > 0; --n, ++cur, ++first )
            vince::construct_at( &*cur, vince::move( *first ) );
    }
    catch ( ... ) {
        vince::destroy( result, cur );
        throw;
    }
    return cur;
}

template <class InputIterator, class Size, class ForwardIterator,
          std::enable_if_t<is_random_access_iterator_t<InputIterator>>>
inline constexpr ForwardIterator __uninitialized_move_n( InputIterator first, Size n, ForwardIterator result ) {
    return vince::uninitialized_move( first, first + n, vince::move( result ) );
}

template <class InputIterator, class Size, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_move_n( InputIterator first, Size n, ForwardIterator result ) {
    return vince::__uninitialized_move_n( vince::move( first ), vince::move( n ), vince::move( result ) );
}

};  // namespace vince

#endif