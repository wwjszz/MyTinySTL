#ifndef WYNE_IN_PLACE_H__
#define WYNE_IN_PLACE_H__

#include <cstddef>

namespace wyne {

struct in_place_t {
    explicit in_place_t() = default;
};

template <std::size_t I>
struct in_place_index_t {
    explicit in_place_index_t() = default;
};

template <class T>
struct in_place_type_t {
    explicit in_place_type_t() = default;
};

constexpr in_place_t in_place{};

template <std::size_t I>
constexpr in_place_index_t<I> in_place_index{};

template <class T>
constexpr in_place_type_t<T> in_place_type{};

}  // namespace wyne

#endif