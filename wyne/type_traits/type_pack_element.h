#ifndef WYNE_TYPE_PACK_ELEMENT_H__
#define WYNE_TYPE_PACK_ELEMENT_H__

#include <cstddef>

#include "config.h"
#include "type_traits/integer_sequence.h"

namespace wyne {

template <std::size_t I, class T>
struct indexed_type {
    static constexpr std::size_t value = I;
    using type                         = T;
};

#ifdef WYNE_TYPE_PACK_ELEMENT
template <std::size_t I, class... Ts>
struct type_pack_element {
private:
    template <class... T>
    struct set;

    template <std::size_t... Is>
    struct set<index_sequence<Is...>> : indexed_type<Is, Ts>... {};

    template <class T>
    static constexpr T impl( indexed_type<I, T> );

public:
    using type = decltype( impl( set<index_sequence_for<Ts...>>{} ) );
};

template <std::size_t I, class... Ts>
using type_pack_element_t = typename type_pack_element<I, Ts...>::type;

#else

template <std::size_t I, class... Ts>
using type_pack_element_t = __type_pack_element<I, Ts...>;

#endif

}  // namespace wyne

#endif