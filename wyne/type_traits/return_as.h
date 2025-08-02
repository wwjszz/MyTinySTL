#ifndef WYNE_RETURN_AS_H__
#define WYNE_RETURN_AS_H__

#include <functional>

#include "util.h"

namespace wyne {

// return_as concept
template <class Ret, class F, class... Args>
concept return_as = requires( F&& f, Args&&... args ) {
    { std::invoke( wyne::forward<F>( f ), wyne::forward<Args>( args )... ) } -> std::same_as<Ret>;
};

}  // namespace wyne

#endif