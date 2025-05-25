#ifndef VINCE_UTIL_H__
#define VINCE_UTIL_H__

#include <type_traits>

namespace vince {

// move

template <class T>
typename std::remove_reference<T>::type&& move( T&& t ) noexcept {
    return static_cast<std::remove_reference<T>::type>( t );
}

// forward

template <class T>
T&& forward( std::remove_reference_t<T>& t ) noexcept {
    return static_cast<T&&>( t );
}

template <class T>
T&& forward( std::remove_reference_t<T>&& t ) noexcept {
    static_assert( !std::is_lvalue_reference_v<T>, "cannot forward an rvalue as an lvalue" );
    return static_cast<T&&>( t );
}

// swap

template <class T>
using swap_result_t = std::enable_if_t<std::is_move_constructible_v<T> && std::is_move_assignable_v<T>>;

template <class T>
inline swap_result_t<T> swap( T& x, T& y ) noexcept( std::is_nothrow_move_constructible_v<T>
                                                     && std::is_nothrow_move_assignable_v<T> ) {
    T t( move( x ) );
    x = move( y );
    y = move( t );
}

template <class T, size_t N, std::enable_if_t<std::is_swappable_v<T>, int>>
inline void swap( T ( &a )[ N ], T ( &b )[ N ] ) noexcept( std::is_nothrow_swappable_v<T> ) {
    for ( size_t i = 0; i != N; ++i ) {
        swap( a[ i ], b[ i ] );
    }
}

};  // namespace vince

#endif