#ifndef WYNE_CONFIG_H__
#define WYNE_CONFIG_H__

#define WYNE_LIKELY( x ) ( __builtin_expect( !!( x ), 1 ) )
#define WYNE_UNLIKELY( x ) ( __builtin_expect( !!( x ), 0 ) )

namespace wyne {

// new file?

template <class T>
inline constexpr bool requires_special_alignment() {
    return alignof( T ) > __STDCPP_DEFAULT_NEW_ALIGNMENT__;
}

template <class Tp, bool>
struct dependent_type : public Tp {};

#ifndef WYNE_USE_STD
#define WYNE_INTEGER_SEQUENCE
#define WYNE_TYPE_PACK_ELEMENT
#endif

};  // namespace wyne

#endif