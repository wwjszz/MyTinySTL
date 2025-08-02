#ifndef WYNE_INTEGER_SEQUENCE_H__
#define WYNE_INTEGER_SEQUENCE_H__

#include <cstddef>

#include "config.h"

namespace wyne {

// identity
template <class T>
struct identity {
    using type = T;
};

#ifdef WYNE_INTEGER_SEQUENCE
// integer_sequence
template <class T, T... Is>
struct integer_sequence {
    using value_type = T;
    constexpr std::size_t size() const noexcept { return sizeof...( Is ); }
};

// index_sequence
template <std::size_t... Is>
using index_sequence = integer_sequence<std::size_t, Is...>;

// make_index_sequence
template <class, class>
struct make_index_sequence_concat;

template <std::size_t... Lhs, std::size_t... Rhs>
struct make_index_sequence_concat<index_sequence<Lhs...>, index_sequence<Rhs...>>
    : identity<index_sequence<Lhs..., ( sizeof...( Lhs ) + Rhs )...>> {};

template <std::size_t N>
struct make_index_sequence_impl;

template <std::size_t N>
using make_index_sequence = typename make_index_sequence_impl<N>::type;

template <std::size_t N>
struct make_index_sequence_impl
    : make_index_sequence_concat<make_index_sequence<N / 2>, make_index_sequence<N - N / 2>> {};

template <>
struct make_index_sequence_impl<0> : identity<index_sequence<>> {};

template <>
struct make_index_sequence_impl<1> : identity<index_sequence<0>> {};

// index_sequence_for
template <class... Ts>
using index_sequence_for = make_index_sequence<sizeof...( Ts )>;

#else

using std::index_sequence;
using std::index_sequence_for;
using std::integer_sequence;
using std::make_index_sequence;

#endif

}  // namespace wyne

#endif