#ifndef WYNE_COMPRESSED_PAIR_H__
#define WYNE_COMPRESSED_PAIR_H__

#include "config.h"
#include "util.h"
#include <tuple>
#include <type_traits>
#include <utility>
namespace wyne {

struct default_init_tag {};
struct value_init_tag {};

template <class Tp, int Idx, bool empty = std::is_empty_v<Tp> && !std::is_final_v<Tp>>
struct compressed_pair_elem {
    using reference       = Tp&;
    using const_reference = const Tp&;

    constexpr explicit compressed_pair_elem( default_init_tag ) {}
    constexpr explicit compressed_pair_elem( value_init_tag ) : value_() {}

    template <class Up, std::enable_if_t<!std::is_same_v<compressed_pair_elem, std::decay_t<Tp>>, int> = 0>
    constexpr explicit compressed_pair_elem( Up&& u ) : value_( wyne::forward<Up>( u ) ) {}

    template <class... Args, size_t... Indices>
    constexpr explicit compressed_pair_elem( std::piecewise_construct_t, std::tuple<Args...> args,
                                             std::index_sequence<Indices...> )
        : value_( wyne::forward<Args>( std::get<Indices>( args ) )... ) {}

    reference get() noexcept {
        return value_;
    }

    const_reference get() const noexcept {
        return value_;
    }

private:
    Tp value_;
};

template <class Tp, int Idx>
struct compressed_pair_elem<Tp, Idx, true> : private Tp {
    using reference       = Tp&;
    using const_reference = const Tp&;
    using value_type      = Tp;

    constexpr explicit compressed_pair_elem() = default;
    constexpr explicit compressed_pair_elem( default_init_tag ) {}
    constexpr explicit compressed_pair_elem( value_init_tag ) : value_type() {}

    template <class Up, std::enable_if_t<!std::is_same_v<compressed_pair_elem, std::decay_t<Tp>>, int> = 0>
    constexpr explicit compressed_pair_elem( Up&& u ) : value_type( wyne::forward<Up>( u ) ) {}

    template <class... Args, size_t... Indices>
    constexpr explicit compressed_pair_elem( std::piecewise_construct_t, std::tuple<Args...> args,
                                             std::index_sequence<Indices...> )
        : value_type( wyne::forward<Args>( std::get<Indices>( args ) )... ) {}

    reference get() noexcept {
        return *this;
    }

    const_reference get() const noexcept {
        return *this;
    }
};

template <class T1, class T2>
class compressed_pair : private compressed_pair_elem<T1, 0>, private compressed_pair_elem<T2, 1> {
public:
    static_assert( !std::is_same_v<T1, T2>, "" );

    using Base1 = compressed_pair_elem<T1, 0>;
    using Base2 = compressed_pair_elem<T2, 1>;

    template <bool Dummy            = true,
              std::enable_if_t<dependent_type<std::is_default_constructible<Base1>, Dummy>::value
                                   && dependent_type<std::is_default_constructible<Base1>, Dummy>::value,
                               int> = 0>
    constexpr compressed_pair() : Base1( value_init_tag() ), Base2( value_init_tag() ) {}

    template <class U1, class U2>
    constexpr compressed_pair( U1&& u1, U2&& u2 )
        : Base1( wyne::forward<U1>( u1 ) ), Base2( wyne::forward<U2>( u2 ) ) {}

    template <class... Args1, class... Args2>
    constexpr compressed_pair( std::piecewise_construct_t pc, std::tuple<Args1...> first_args,
                               std::tuple<Args2...> second_args )
        : Base1( pc, wyne::move( first_args ), std::make_index_sequence<sizeof...( Args1 )>() ),
          Base2( pc, wyne::move( second_args ), std::make_index_sequence<sizeof...( Args2 )>() ) {}

    constexpr typename Base1::reference first() noexcept {
        return static_cast<Base1&>( *this ).get();
    }

    constexpr typename Base1::const_reference first() const noexcept {
        return static_cast<Base1 const&>( *this ).get();
    }

    constexpr typename Base2::reference second() noexcept {
        return static_cast<Base2&>( *this ).get();
    }

    constexpr typename Base2::const_reference second() const noexcept {
        return static_cast<Base2 const&>( *this ).get();
    }

    constexpr static Base1* get_first_base( compressed_pair* pair ) noexcept {
        return static_cast<Base1*>( pair );
    }

    constexpr static Base2* get_second_base( compressed_pair* pair ) noexcept {
        return static_cast<Base2*>( pair );
    }

    constexpr void swap( compressed_pair& x ) noexcept( std::is_nothrow_swappable_v<T1>
                                                        && std::is_nothrow_swappable_v<T2> ) {
        using wyne::swap;
        swap( first(), x.first() );
        swap( second(), x.second() );
    }
};

template <class T1, class T2>
inline constexpr void swap( compressed_pair<T1, T2>& x,
                            compressed_pair<T1, T2>& y ) noexcept( std::is_nothrow_swappable_v<T1>
                                                                   && std::is_nothrow_swappable_v<T2> ) {
    x.swap( y );
}

};  // namespace wyne

#endif