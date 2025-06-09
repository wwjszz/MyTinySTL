#ifndef WYNE_SWAP_ALLOCATOR_H__
#define WYNE_SWAP_ALLOCATOR_H__

#include "type_traits.h"
#include "util.h"
#include <memory>

namespace wyne {
template <class Alloc>
inline constexpr void __swap_allocator( Alloc& a1, Alloc& a2, true_type ) noexcept {
    using wyne::swap;
    swap( a1, a2 );
}

template <class Alloc>
inline constexpr void __swap_allocator( Alloc& a1, Alloc& a2, false_type ) noexcept {}

template <class Alloc>
inline constexpr void swap_allocator( Alloc& a1, Alloc& a2 ) noexcept {
    __swap_allocator( a1, a2, bool_constant<std::allocator_traits<Alloc>::propagate_on_container_swap::value>() );
}

}  // namespace wyne

#endif