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

};  // namespace wyne
#endif