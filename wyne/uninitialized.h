#ifndef WYNE_UNINITIALIZED_H__
#define WYNE_UNINITIALIZED_H__

#include <cstring>
#include <memory>
#include <type_traits>

#include "algobase.h"
#include "allocator.h"
#include "allocator_traits.h"
#include "construct.h"
#include "iterator.h"
#include "type_traits.h"
#include "util.h"

namespace wyne {

// uninitialized_copy

template <class InputIterator, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_copy( InputIterator first, InputIterator last,
                                                       ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; first != last; ++first, ++cur )
            wyne::construct_at( std::to_address( cur ), *first );
    }
    catch ( ... ) {
        wyne::destroy( result, cur );
        throw;
    }
    return cur;
}

template <class Tp, class Up, std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_copy( Tp* first, Tp* last, Up* result ) {
    return wyne::__copy_trivial( first, last, result );
}

template <class Tp, class Up, std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_copy( Tp const* first, Tp const* last, Up* result ) {
    return wyne::__copy_trivial( first, last, result );
}

template <class InputIterator, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_copy( InputIterator first, InputIterator last, ForwardIterator result ) {
    return wyne::__uninitialized_copy( wyne::move( first ), wyne::move( last ), wyne::move( result ) );
}

// uninitialized_copy_n

template <class InputIterator, class Size, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_copy_n( InputIterator first, Size n, ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; n > 0; --n, ++cur, ++first )
            wyne::construct_at( std::to_address( cur ), *first );
    }
    catch ( ... ) {
        wyne::destroy( result, cur );
        throw;
    }
    return cur;
}

template <class InputIterator, class Size, class ForwardIterator,
          std::enable_if_t<is_random_access_iterator_t<InputIterator>>>
inline constexpr ForwardIterator __uninitialized_copy_n( InputIterator first, Size n, ForwardIterator result ) {
    return wyne::uninitialized_copy( first, first + n, wyne::move( result ) );
}

template <class InputIterator, class Size, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_copy_n( InputIterator first, Size n, ForwardIterator result ) {
    return wyne::__uninitialized_copy_n( wyne::move( first ), wyne::move( n ), wyne::move( result ) );
}

// uninitialized_fill_n

template <class ForwardIterator, class Size, class Tp>
inline constexpr ForwardIterator __uninitialized_fill_n( ForwardIterator first, Size n, const Tp& value ) {
    ForwardIterator cur = first;
    try {
        for ( ; n > 0; --n, ++cur )
            wyne::construct_at( std::to_address( cur ), value );
    }
    catch ( ... ) {
        wyne::destroy( first, cur );
        throw;
    }
    return cur;
}

template <class Tp, class Size, class Up>
inline constexpr std::enable_if_t<sizeof( Up ) == 1 && sizeof( Tp ) == 1
                                      && std::is_integral_v<Tp> && std::is_integral_v<Up> && !std::is_same_v<Tp, bool>,
                                  Tp*>
__uninitialized_fill_n( Tp* first, Size n, const Up& value ) {
    return wyne::__fill_n( first, wyne::move( n ), value );
}

template <class ForwardIterator, class Size, class Tp>
inline constexpr ForwardIterator uninitialized_fill_n( ForwardIterator first, Size n, const Tp& value ) {
    return wyne::__uninitialized_fill_n( first, wyne::move( n ), value );
}

// uninitialized_fill

template <class ForwardIterator, class Tp, class = void>
inline constexpr ForwardIterator __uninitialized_fill( ForwardIterator first, ForwardIterator last, const Tp& value ) {
    ForwardIterator cur = first;
    try {
        for ( ; cur != last; ++cur )
            wyne::construct_at( std::to_address( cur ), value );
    }
    catch ( ... ) {
        wyne::destroy( first, cur );
        throw;
    }
    return cur;
}

template <class ForwardIterator, class Tp, std::enable_if_t<is_random_access_iterator_t<ForwardIterator>>>
inline constexpr ForwardIterator __uninitialized_fill( ForwardIterator first, ForwardIterator last, const Tp& value ) {
    return wyne::uninitialized_fill_n( first, last - first, value );
}

template <class ForwardIterator, class Tp>
inline constexpr ForwardIterator uninitialized_fill( ForwardIterator first, ForwardIterator last, const Tp& value ) {
    return wyne::__uninitialized_fill( wyne::move( first ), wyne::move( last ), value );
}

// uninitialized_move

template <class InputIterator, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_move( InputIterator first, InputIterator last,
                                                       ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; first != last; ++first, ++cur )
            wyne::construct_at( std::to_address( cur ), wyne::move( *first ) );
    }
    catch ( ... ) {
        wyne::destroy( result, cur );
        throw;
    }
    return cur;
}

template <class Tp, class Up, std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_move( Tp* first, Tp* last, Up* result ) {
    return wyne::__move_trivial( first, last, result );
}

template <class Tp, class Up, std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_move( Tp const* first, Tp const* last, Up* result ) {
    return wyne::__move_trivial( first, last, result );
}

template <class InputIterator, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_move( InputIterator first, InputIterator last, ForwardIterator result ) {
    return wyne::__uninitialized_move( wyne::move( first ), wyne::move( last ), wyne::move( result ) );
}

// uninitialized_move_n

template <class InputIterator, class Size, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_move_n( InputIterator first, Size n, ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; n > 0; --n, ++cur, ++first )
            wyne::construct_at( std::to_address( cur ), wyne::move( *first ) );
    }
    catch ( ... ) {
        wyne::destroy( result, cur );
        throw;
    }
    return cur;
}

template <class InputIterator, class Size, class ForwardIterator,
          std::enable_if_t<is_random_access_iterator_t<InputIterator>>>
inline constexpr ForwardIterator __uninitialized_move_n( InputIterator first, Size n, ForwardIterator result ) {
    return wyne::uninitialized_move( first, first + n, wyne::move( result ) );
}

template <class InputIterator, class Size, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_move_n( InputIterator first, Size n, ForwardIterator result ) {
    return wyne::__uninitialized_move_n( wyne::move( first ), wyne::move( n ), wyne::move( result ) );
}

// uninitialized_allocator_destroy

template <class Alloc, class Iter, class Sent>
constexpr void allocator_destroy( Alloc& alloc, Iter first, Sent last ) {
    for ( ; first != last; ++first )
        std::allocator_traits<Alloc>::destroy( alloc, std::to_address( first ) );
}

// uninitialized_allocator_copy

template <class Alloc, class InputIterator, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_allocator_copy( Alloc& alloc, InputIterator first, InputIterator last,
                                                                 ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; first != last; ++first, ++cur )
            std::allocator_traits<Alloc>::construct( alloc, std::to_address( cur ), *first );
    }
    catch ( ... ) {
        wyne::allocator_destroy( alloc, result, cur );
        throw;
    }
    return cur;
}

template <class Alloc, class Tp, class Up,
          std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_allocator_copy( Alloc&, Tp* first, Tp* last, Up* result ) {
    return wyne::__copy_trivial( first, last, result );
}

template <class Alloc, class Tp, class Up,
          std::enable_if_t<std::is_same_v<Tp, Up> && std::is_trivially_copy_constructible_v<Up>>>
inline constexpr Up* __uninitialized_allocator_copy( Alloc&, Tp const* first, Tp const* last, Up* result ) {
    return wyne::__copy_trivial( first, last, result );
}

template <class Alloc, class InputIterator, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_allocator_copy( Alloc& alloc, InputIterator first, InputIterator last,
                                                               ForwardIterator result ) {
    return wyne::__uninitialized_allocator_copy( alloc, wyne::move( first ), wyne::move( last ), wyne::move( result ) );
}

// uninitialized_allocator_copy_n

template <class Alloc, class InputIterator, class Size, class ForwardIterator, class = void>
inline constexpr ForwardIterator __uninitialized_allocator_copy_n( Alloc& alloc, InputIterator first, Size n,
                                                                   ForwardIterator result ) {
    ForwardIterator cur = result;
    try {
        for ( ; n > 0; --n, ++cur, ++first )
            std::allocator_traits<Alloc>::construct( alloc, std::to_address( cur ), *first );
    }
    catch ( ... ) {
        wyne::allocator_destroy( alloc, result, cur );
        throw;
    }
    return cur;
}

template <class Alloc, class InputIterator, class Size, class ForwardIterator,
          std::enable_if_t<is_random_access_iterator_t<InputIterator>>>
inline constexpr ForwardIterator __uninitialized_allocator_copy_n( Alloc& alloc, InputIterator first, Size n,
                                                                   ForwardIterator result ) {
    return wyne::uninitialized_allocator_copy( alloc, first, first + n, wyne::move( result ) );
}

template <class Alloc, class InputIterator, class Size, class ForwardIterator>
inline constexpr ForwardIterator uninitialized_allocator_copy_n( Alloc& alloc, InputIterator first, Size n,
                                                                 ForwardIterator result ) {
    return wyne::__uninitialized_allocator_copy_n( alloc, wyne::move( first ), wyne::move( n ), wyne::move( result ) );
}

// uninitialized_allocator_relocate

template <class Alloc, class Tp>
struct allocator_has_trivial_move_construct : public Not<std::__has_construct<Alloc, Tp*, Tp&&>> {};

template <class Tp>
struct allocator_has_trivial_move_construct<allocator<Tp>, Tp> : _true_type {};

template <class Alloc, class Tp>
struct allocator_has_destroy : public Not<std::__has_destroy<Alloc, Tp*>> {};

template <class Tp, class Up>
struct allocator_has_destroy<allocator<Tp>, Up> : _true_type {};

template <class Alloc, class Tp>
constexpr void uninitialized_allocator_relocate( Alloc& alloc, Tp* first, Tp* last, Tp* result ) {
    if constexpr ( !is_trivially_relocate<Tp>::value && !allocator_has_trivial_move_construct<Alloc, Tp>::value
                   && !allocator_has_destroy<Alloc, Tp>::value ) {
        auto iter = first;
        while ( iter != last ) {
            std::allocator_traits<Alloc>::construct( alloc, result, wyne::move_if_noexcept( *iter ) );
            ++iter;
            ++result;
        }
        wyne::allocator_destroy( alloc, first, last );
    }
    else {
        std::memcpy( result, first, sizeof( Tp ) * ( last - first ) );
    }
}

};  // namespace wyne

#endif