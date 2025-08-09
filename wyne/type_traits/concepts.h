#ifndef WYNE_CONCEPTS_H__
#define WYNE_CONCEPTS_H__

#include <functional>
#include <type_traits>

#include "util.h"

namespace wyne {

// return_as concept
template <class Ret, class F, class... Args>
concept return_as = requires( F&& f, Args&&... args ) {
    { std::invoke( wyne::forward<F>( f ), wyne::forward<Args>( args )... ) } -> std::same_as<Ret>;
};

template <class T>
concept default_constructible = std::is_default_constructible_v<T>;

template <class T, class... Args>
concept constructible = std::is_constructible_v<T, Args...>;

template <class T, class... Args>
concept assignable = std::is_assignable_v<T, Args...>;

}  // namespace wyne

#endif