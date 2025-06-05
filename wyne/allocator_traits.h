#ifndef WYNE_ALLOCATOR_TRAITS_H__
#define WYNE_ALLOCATOR_TRAITS_H__

#include "type_traits.h"

namespace wyne {

// is_trivially_relocate

template <class Tp>
struct is_trivially_relocate : public first_true<
#if __has_builtin( __is_trivially_relocatable )
                                   _bool_constant<__is_trivially_relocatable( Tp )>,
#else
                                   std::is_trivially_copyable<Tp>,
#endif
                                   _false_type> {
};

}  // namespace wyne

#endif