#ifndef WYNE_UTIL_H__
#define WYNE_UTIL_H__

#include "type_traits.h"
#include <cstdarg>
#include <type_traits>

namespace wyne {

// move

template <class T>
inline constexpr typename std::remove_reference<T>::type&& move( T&& t ) noexcept {
    return static_cast<std::remove_reference<T>::type&&>( t );
}

// move_if_noexcept

template <class Tp>
using move_if_noexcept_result_t = conditional_t<!std::is_nothrow_move_constructible_v<Tp> && std::is_copy_constructible_v<Tp>, const Tp&, Tp&&>;

template <class Tp>
inline constexpr move_if_noexcept_result_t<Tp> move_if_noexcept( Tp& x ) noexcept {
    return wyne::move( x );
}

// forward

template <class T>
inline constexpr T&& forward( std::remove_reference_t<T>& t ) noexcept {
    return static_cast<T&&>( t );
}

template <class T>
inline constexpr T&& forward( std::remove_reference_t<T>&& t ) noexcept {
    static_assert( !std::is_lvalue_reference_v<T>, "cannot forward an rvalue as an lvalue" );
    return static_cast<T&&>( t );
}

// swap

template <class T>
using swap_result_t = std::enable_if_t<std::is_move_constructible_v<T> && std::is_move_assignable_v<T>>;

template <class T>
inline constexpr swap_result_t<T> swap( T& x, T& y ) noexcept( std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T> ) {
    T t( wyne::move( x ) );
    x = wyne::move( y );
    y = wyne::move( t );
}

template <class T, size_t N, std::enable_if_t<std::is_swappable_v<T>, int>>
inline constexpr void swap( T ( &a )[ N ], T ( &b )[ N ] ) noexcept( std::is_nothrow_swappable_v<T> ) {
    for ( size_t i = 0; i != N; ++i ) {
        swap( a[ i ], b[ i ] );
    }
}

// pair

template <class T1, class T2>
struct pair {

    using first_type  = T1;
    using second_type = T2;

    T1 first;
    T2 second;

    pair( const pair& ) = default;
    pair( pair&& )      = default;

    struct CheckArgs {
        template <int&...>
        static constexpr bool enable_default() {
            return std::is_default_constructible_v<T1> && std::is_default_constructible_v<T2>;
        }

        template <class U1, class U2>
        static constexpr bool is_pair_constructible() {
            return std::is_constructible_v<first_type, U1> && std::is_constructible_v<second_type, U2>;
        }

        template <class U1, class U2>
        static constexpr bool is_implicit() {
            return std::is_convertible_v<U1, first_type> && std::is_convertible_v<U2, second_type>;
        }
    };

    template <std::enable_if_t<CheckArgs::enable_default(), int> = 0>
    constexpr pair() noexcept( std::is_nothrow_default_constructible_v<T1> && std::is_nothrow_default_constructible_v<T2> ) : first(), second() {}

    template <std::enable_if_t<CheckArgs::template is_pair_constructible<T1 const&, T2 const&>(), int> = 0>
    explicit( !CheckArgs::template is_implicit<T1 const&, T2 const&>() ) constexpr pair( T1 const& t1, T2 const& t2 ) noexcept(
        std::is_nothrow_copy_constructible_v<T1> && std::is_nothrow_copy_constructible_v<T2> )
        : first( t1 ), second( t2 ) {}

    template <class U1, class U2, std::enable_if_t<CheckArgs::template is_pair_constructible<U1, U2>(), int> = 0>
    explicit( !CheckArgs::template is_implicit<U1, U2>() ) constexpr pair( U1&& u1,
                                                                           U2&& u2 ) noexcept( std::is_nothrow_constructible_v<first_type, U1>
                                                                                               && std::is_nothrow_constructible_v<second_type, U2> )
        : first( wyne::forward<U1>( u1 ) ), second( wyne::forward<U2>( u2 ) ) {}

    template <class U1, class U2, std::enable_if_t<CheckArgs::template is_pair_constructible<U1, U2>(), int> = 0>
    explicit( !CheckArgs::template is_implicit<U1, U2>() ) constexpr pair( pair<U1, U2> const& p ) noexcept(
        std::is_nothrow_constructible_v<first_type, U1> && std::is_nothrow_constructible_v<second_type, U2> )
        : first( p.first ), second( p.second ) {}

    template <class U1, class U2, std::enable_if_t<CheckArgs::template is_pair_constructible<U1, U2>(), int> = 0>
    explicit( !CheckArgs::template is_implicit<U1, U2>() ) constexpr pair( pair<U1, U2>&& p ) noexcept(
        std::is_nothrow_constructible_v<first_type, U1&&> && std::is_nothrow_constructible_v<second_type, U2&&> )
        : first( wyne::forward<U1>( p.first ) ), second( forward<U2>( p.second ) ) {}

    template <std::enable_if_t<std::is_copy_assignable_v<T1> && std::is_copy_assignable_v<T2>, int> = 0>
    pair& operator=( pair const& p ) noexcept( std::is_nothrow_copy_assignable_v<T1> && std::is_nothrow_copy_assignable_v<T2> ) {
        first  = p.first;
        second = p.second;
        return *this;
    }

    template <std::enable_if_t<std::is_move_assignable_v<T1> && std::is_move_assignable_v<T2>, int> = 0>
    pair& operator=( pair&& p ) noexcept( std::is_nothrow_move_assignable_v<T1> && std::is_nothrow_move_assignable_v<T2> ) {
        first  = wyne::forward<T1>( p.first );
        second = wyne::forward<T2>( p.second );
        return *this;
    }

    template <class U1, class U2,
              std::enable_if_t<std::is_assignable_v<first_type&, U1 const&> && std::is_assignable_v<second_type&, U2 const&>, int> = 0>
    pair& operator=( pair<U1, U2> const& p ) noexcept( std::is_nothrow_assignable_v<first_type&, U1 const&>
                                                       && std::is_nothrow_assignable_v<second_type&, U2 const&> ) {
        first  = p.first;
        second = p.second;
        return *this;
    }

    template <class U1, class U2, std::enable_if_t<std::is_assignable_v<first_type&, U1> && std::is_assignable_v<second_type&, U2>, int> = 0>
    pair& operator=( pair<U1, U2>&& p ) noexcept( std::is_nothrow_assignable_v<first_type&, U1&&>
                                                  && std::is_nothrow_assignable_v<second_type&, U2&&> ) {
        first  = wyne::forward<U1>( p.first );
        second = wyne::forward<U2>( p.second );
        return *this;
    }

    void swap( pair& p ) noexcept( std::is_nothrow_swappable_v<first_type> && std::is_nothrow_swappable_v<second_type> ) {
        using wyne::swap;
        swap( first, p.first );
        swap( second, p.second );
    }
};

template <class T1, class T2, class U1, class U2>
inline bool operator==( const pair<T1, T2>& x, const pair<U1, U2>& y ) {
    return x.first == y.first && x.second == y.second;
}

template <class T1, class T2, class U1, class U2>
inline bool operator<( const pair<T1, T2>& x, const pair<U1, U2>& y ) {
    return x.first < y.first || ( x.first == y.first && x.second < y.second );
}

template <class T1, class T2, class U1, class U2>
inline bool operator!=( const pair<T1, T2>& x, const pair<U1, U2>& y ) {
    return !( x == y );
}

template <class T1, class T2, class U1, class U2>
inline bool operator>( const pair<T1, T2>& x, const pair<U1, U2>& y ) {
    return y < x;
}

template <class T1, class T2, class U1, class U2>
inline bool operator<=( const pair<T1, T2>& x, const pair<U1, U2>& y ) {
    return !( y < x );
}

template <class T1, class T2, class U1, class U2>
inline bool operator>=( const pair<T1, T2>& x, const pair<U1, U2>& y ) {
    return !( x < y );
}

template <class T1, class T2, std::enable_if_t<std::is_swappable_v<T1> && std::is_swappable_v<T2>, int> = 0>
void swap( pair<T1, T2>& x, pair<T1, T2>& y ) noexcept( std::is_nothrow_swappable_v<T1> && std::is_nothrow_swappable_v<T2> ) {
    x.swap( y );
}

template <class T1, class T2>
inline pair<std::unwrap_ref_decay_t<T1>, std::unwrap_ref_decay_t<T2>> make_pair( T1&& t1, T2&& t2 ) {
    return pair<std::unwrap_ref_decay_t<T1>, std::unwrap_ref_decay_t<T2>>( wyne::forward<T1>( t1 ), wyne::forward<T2>( t2 ) );
}

};  // namespace wyne

#endif