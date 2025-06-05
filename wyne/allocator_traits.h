#ifndef WYNE_ALLOCATOR_TRAITS_H__
#define WYNE_ALLOCATOR_TRAITS_H__

#include "type_traits.h"
#include <type_traits>

namespace wyne {

// is_trivially_relocate

#if __has_builtin( __is_trivially_relocatable )
template <class _Tp, class = void>
struct is_trivially_relocate : _bool_constant<__is_trivially_relocatable( _Tp )> {};
#else
template <class _Tp, class = void>
struct is_trivially_relocate : is_trivially_copyable<_Tp> {};
#endif

template <class _Tp>
struct is_trivially_relocate<_Tp, std::enable_if_t<std::is_same_v<_Tp, typename _Tp::__trivially_relocatable>>>
    : _true_type {};

}  // namespace wyne

#endif